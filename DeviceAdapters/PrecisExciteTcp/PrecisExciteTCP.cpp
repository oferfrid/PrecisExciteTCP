///////////////////////////////////////////////////////////////////////////////
// FILE:          PreciseExciteTCP.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   PrecisExcite controller adapter
// COPYRIGHT:     Hebrew university of Jerusalem
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
// AUTHOR:        Ofer Fridman & Sivan Pearl May 2012
//
// NOTE:          Based on PrecieExcite adpater by Nico S
//





#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN

	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <stdlib.h>
	#include <stdio.h>


	// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
	#define snprintf _snprintf 

#endif

#define DEFAULT_BUFLEN 512
#include "PrecisExciteTCP.h"
#include <string>
#include <math.h>
#include "../../MMDevice/ModuleInterface.h"
#include <sstream>


const char* g_ControllerName = "PrecisExciteTCP";
const char* g_Keyword_Intensity = "Intensity";
const char* g_Keyword_Trigger = "Trigger";
const char* g_Keyword_Trigger_Sequence = "TriggerSequence";
const char * carriage_return = "\r";
const char * line_feed = "\n";
const int TIME_TO_WAIT_FOR_TCP = 1000;


using namespace std;


///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////
MODULE_API void InitializeModuleData()
{
   AddAvailableDeviceName(g_ControllerName, "Colled PrecisExcite TCP" );   
 
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   if (strcmp(deviceName, g_ControllerName) == 0)
   {
      PrecisExciteTCP* pVMM1 = new PrecisExciteTCP();
      return pVMM1;
   }   
   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}


///////////////////////////////////////////////////////////////////////////////
// PrecisExcit TCP device
///////////////////////////////////////////////////////////////////////////////

PrecisExciteTCP::PrecisExciteTCP() :
   initialized_(false),
   tcpport_("18259"),   //default value of CoolLed
   IP_("192.168.0.252"), //default value of CoolLed
   intensity_(0),
   state_(0),
   busy_(false),
   error_(0),
   changedTime_(0.0)
{
   InitializeDefaultErrorMessages();
   InitializeTCPVars();

   // create pre-initialization properties
   // ------------------------------------

   // Name
   CreateProperty(MM::g_Keyword_Name, g_ControllerName, MM::String, true);

   // Description
   CreateProperty(MM::g_Keyword_Description, "CoolLed PrecisExcite TCP controller adapter", MM::String, true);

   // Port
   CPropertyAction* pAct = new CPropertyAction (this, &PrecisExciteTCP::OnPort);
   CreateProperty("TCP Port","18259", MM::String, false, pAct, true);

   // Address
   pAct = new CPropertyAction (this, &PrecisExciteTCP::OnIP);
   CreateProperty("IP Address", "192.168.0.252", MM::String, false, pAct, true);

   EnableDelay(); // signals that the delay setting will be used
   UpdateStatus();
   }

    struct addrinfo *result = NULL,*ptr = NULL,hints;

PrecisExciteTCP::~PrecisExciteTCP()
{
   Shutdown();
}

/*
 * This device does not give any feedback
 * So, start timer after the last command and check whether a predetermined time has elapsed since - NS
 */
bool PrecisExciteTCP::Busy()
{
   MM::MMTime interval = GetCurrentMMTime() - changedTime_;
   MM::MMTime delay(GetDelayMs()*1000.0);
   if (interval < delay)
      return true;
   else
      return false;
}


void PrecisExciteTCP::GetName(char* Name) const
{
   CDeviceUtils::CopyLimitedString(Name, g_ControllerName);
}

int PrecisExciteTCP::Initialize()
{
	this->LogMessage("PrecisExciteTCP::Initialize()",1);
	currentChannel_ = 0;	 
	
	ConnectTCP(IP_,tcpport_);
	
   ReadGreeting(); 
   int result = ReadChannelLabels();
   if (result != DEVICE_OK)
	   return result;

  
   GenerateChannelChooser();
   GeneratePropertyIntensity();
   GeneratePropertyState();
    
   initialized_ = true;
   return HandleErrors(); 
   
}



void PrecisExciteTCP::ReadGreeting()
{
	string buf_string_=SendCommand("\n");   
}


int PrecisExciteTCP::ReadChannelLabels()
{
 
   string buf_string_=SendCommand("LAMS\n");
   LogMessage(buf_string_.c_str(),1);
   istringstream stream(buf_string_);
   string sub;
   while(getline(stream, sub)) 
	{
	if (sub.substr(0,3).compare("LAM")==0 && sub.substr(10,2).compare("nm")==0 ) 
	{
		channelLetters_.push_back(sub[4]);
		 string label = sub.substr(6);

         string label1 = trim(label);
         channelLabels_.push_back(label1);
	}
   }


   if (channelLabels_.size() == 0)
	   return DEVICE_ERR;
   else{
	   stringstream msg;
	   msg<<"chanelLaeblse size is :"<<channelLabels_.size()<<line_feed	;
	   LogMessage(msg.str(),1);
	   return DEVICE_OK;
   }
}


/////////////////////////////////////////////
// Property Generators
/////////////////////////////////////////////

void PrecisExciteTCP::GeneratePropertyState()
{
   CPropertyAction* pAct = new CPropertyAction (this, &PrecisExciteTCP::OnState);
   CreateProperty(MM::g_Keyword_State, "0", MM::Integer, false, pAct);
   AddAllowedValue(MM::g_Keyword_State, "0");
   AddAllowedValue(MM::g_Keyword_State, "1");
}
void PrecisExciteTCP::GenerateChannelChooser()
{
   if (! channelLabels_.empty()) 
   {  
	   
      CPropertyAction* pAct;
      pAct = new CPropertyAction (this, &PrecisExciteTCP::OnChannelLabel);
      CreateProperty("ChannelLabel", channelLabels_[0].c_str(), MM::String, false, pAct);
      SetAllowedValues("ChannelLabel",channelLabels_);
      SetProperty("ChannelLabel",channelLabels_[0].c_str());
            
   }
}

void PrecisExciteTCP::GeneratePropertyIntensity()
{
   string intensityName;
   CPropertyActionEx* pAct; 
   for (unsigned i=0;i<channelLetters_.size();i++)
   {
      pAct = new CPropertyActionEx(this, &PrecisExciteTCP::OnIntensity, i);
      intensityName = g_Keyword_Intensity;
      intensityName.push_back(channelLetters_[i]);
      CreateProperty(intensityName.c_str(), "0", MM::Integer, false, pAct);
      SetPropertyLimits(intensityName.c_str(), 0, 100);
   }
}

void PrecisExciteTCP::GeneratePropertyTrigger()
{

}

void PrecisExciteTCP::GeneratePropertyTriggerSequence()
{
}


int PrecisExciteTCP::Shutdown()
{
   if (initialized_)
   {
		closesocket(ConnectSocket);
		WSACleanup();
	   initialized_ = false;

   }
   return HandleErrors();
}


///////////////////////////////////////////////////////////////////////////////
// Action handlers
///////////////////////////////////////////////////////////////////////////////
/*
 * Sets the Serial Port to be used.
 * Should be called before initialization
 */
int PrecisExciteTCP::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(tcpport_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      if (initialized_)
      {
         // revert
         pProp->Set(tcpport_.c_str());
         return ERR_PORT_CHANGE_FORBIDDEN;
      }

      pProp->Get(tcpport_);
   }
   return DEVICE_OK;
}


/**
 * Determines which IP controller uses
 */
int PrecisExciteTCP::OnIP(MM::PropertyBase* ipProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      ipProp->Set(IP_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      if (initialized_)
      {
         // revert
         ipProp->Set(IP_.c_str());
         return ERR_PORT_CHANGE_FORBIDDEN;
      }

      ipProp->Get(IP_);
   }
   return HandleErrors(); 
}


int PrecisExciteTCP::OnTrigger(MM::PropertyBase* pProp, MM::ActionType eAct)
{

   if (eAct == MM::BeforeGet)
   {
      pProp->Set(trigger_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      char cmd=0;
      pProp->Get(trigger_);
      for (int i=0;i<5;i++)
      {
         if (trigger_.compare(TriggerLabels[i])==0)
         {
            cmd = TriggerCmd[i];
            triggerMode_ = (TriggerType) i;
         }
      }
      
      SetTrigger();

   }
  return HandleErrors();
}


int PrecisExciteTCP::OnTriggerSequence(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(triggerSequence_.c_str());
   }
   else if (eAct == MM::AfterSet)
   { 
      pProp->Get(triggerSequence_);
      SetTrigger();
   }
   return HandleErrors();
}


int PrecisExciteTCP::OnChannelLabel(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(currentChannelLabel_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
	   LogMessage(currentChannelLabel_.c_str(),1); 
      GetState(state_);
      pProp->Get(currentChannelLabel_);
      for (unsigned i=0;i<channelLabels_.size();i++)
         if (channelLabels_[i].compare(currentChannelLabel_) == 0)
         {
            currentChannel_ = i;
            SetState(state_);
         }
   }
   return HandleErrors();
}

int PrecisExciteTCP::OnState(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      GetState(state_);
      pProp->Set(state_);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(state_);
      SetState(state_);
   }
   
   return HandleErrors();
}

int PrecisExciteTCP::OnIntensity(MM::PropertyBase* pProp, MM::ActionType eAct, long index)
{
   long intensity;
   if (eAct == MM::BeforeGet)
   {
      GetIntensity(intensity,index);
      pProp->Set(intensity);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(intensity);
      SetIntensity(intensity, index);
   }
    return HandleErrors();
}


///////////////////////////////////////////////////////////////////////////////
// Utility methods
///////////////////////////////////////////////////////////////////////////////



void PrecisExciteTCP::SetIntensity(long intensity, long index)
{
   stringstream msg;
   msg << "C" << channelLetters_[index] << "I" << intensity << line_feed;
   string buf_string_=SendCommand(msg.str());
}



void PrecisExciteTCP::GetIntensity(long& intensity, long index)
{
   stringstream msg; 
   msg << "C" << channelLetters_[index] << "?" << line_feed;
 //  Purge();
   string buf_string_=SendCommand(msg.str());
  // ReceiveOneLine();
   if (! buf_string_.empty())
      if (0 == buf_string_.compare(0,2,msg.str(),0,2))
      {
         intensity = atol(buf_string_.substr(2,3).c_str());
      }
}

void PrecisExciteTCP::SetState(long state)
{
   state_ = state;
   stringstream msg;
   Illuminate();
   // Set timer for the Busy signal
   changedTime_ = GetCurrentMMTime();
}

void PrecisExciteTCP::GetState(long &state)
{  
      long stateTmp = 0;
	  string buf_string_=SendCommand("C?\n");
	  LogMessage(buf_string_.c_str(),1);
	  istringstream iss(buf_string_);
	  do
    {
		string sub;
        iss >> sub;
		 if(sub.length()>5 && sub[5]=='N')
			stateTmp=1;
    } while (iss);
     
      state = stateTmp; 
}


void PrecisExciteTCP::SetTrigger()
{
	//Trigger not suported

   /*stringstream msg;
   msg << "SQX" << carriage_return;

   for (unsigned i=0;i<triggerSequence_.size();i++)
   {
      msg << "SQ" << triggerSequence_[i] << carriage_return;
   }

   triggerMessage_ = msg.str();

   Illuminate();*/

}


void PrecisExciteTCP::Illuminate()
{
	 stringstream msg;
   if (state_==0)
	   
   {
		msg  << "C" << channelLetters_[currentChannel_] << "F" << line_feed ;
   }
   else if (state_==1)
   {
	  msg <<  "C" << channelLetters_[currentChannel_] << "N"  << line_feed ;
   }
   SendCommand(msg.str());
}


int PrecisExciteTCP::HandleErrors()
{
   int lastError = error_;
   error_ = 0;
   return lastError;
}

/////////////////////////////////////
//  Communications
/////////////////////////////////////

void PrecisExciteTCP::InitializeTCPVars()
{	
   ConnectSocket = INVALID_SOCKET;
   recvbuflen = DEFAULT_BUFLEN;
}  

string PrecisExciteTCP::SendCommand(string  cmd )
{
	char *sendbuf = _strdup(cmd.c_str());
	    // Send an initial buffer
    int iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {		
        closesocket(ConnectSocket);
        WSACleanup();
    }    

	Sleep(TIME_TO_WAIT_FOR_TCP); // wait for tcp socket to complete.
	string  Result="";
	    char recvbuf[DEFAULT_BUFLEN];		
		int recvbuflen = DEFAULT_BUFLEN;
		memset(recvbuf,0,recvbuflen);	
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
		{			
			Result.assign(recvbuf, iResult);						
		}
        else if ( iResult == 0 )
			LogMessage("Fron reading: Connection closed");  
        else{
			LogMessage("recv failed"); 
		}
		return Result;
}

int PrecisExciteTCP::ConnectTCP(string IP,string port)
{
	LogMessage("Connect to TCP");
	 // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        LogMessage("WSAStartup failed with error: \n");
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
	const char *p1;
	p1=IP.c_str();	
	const char *p2;
	p2=port.c_str();	
	iResult = getaddrinfo(p1, p2, &hints, &result);
    if ( iResult != 0 ) {
        LogMessage("getaddrinfo failed with error:\n");
        WSACleanup();
        return 1;
    }
    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            LogMessage("socket failed with error: \n");
            WSACleanup();
            return 1;
        }
        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (ConnectSocket == INVALID_SOCKET) {
        LogMessage("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }	
	return DEVICE_OK; //all went well....
}
///////////////////////////////////////////////////////////////////////////////
// String utilities
///////////////////////////////////////////////////////////////////////////////

   
string PrecisExciteTCP::trim(const string& str) {
size_t start = str.find_first_not_of(" \t\n\r");
if(start == string::npos) return "";
return str.substr(start, str.find_last_not_of(" \t\n\r") - start + 1);
}



//********************
// Shutter API
//********************

int PrecisExciteTCP::SetOpen(bool open)
{

	SetState((long) open);
   return HandleErrors();
  
}

int PrecisExciteTCP::GetOpen(bool& open)
{
	long state;
   GetState(state);
   if (state==1)
      open = true;
   else if (state==0)
      open = false;
   else
      error_ = DEVICE_UNKNOWN_POSITION;

   return HandleErrors();

}
int PrecisExciteTCP::Fire(double /*deltaT*/)
{
   return DEVICE_UNSUPPORTED_COMMAND;
}