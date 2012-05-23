///////////////////////////////////////////////////////////////////////////////
// FILE:          PrecisExciteTCP.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   PrecisExcite controller adapter
// COPYRIGHT:     University of California, San Francisco, 2009
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
//
// AUTHOR:        Arthur Edelstein, arthuredelstein@gmail.com, 3/17/2009
//
//

#ifndef _PRECISEXCITETCP_H_
#define _PRECISEXCITETCP_H_

#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"
#include "../../MMDevice/DeviceUtils.h"
#include <string>
//#include <iostream>
#include <vector>
using namespace std;
//////////////////////////////////////////////////////////////////////////////
// Error codes
//
//#define ERR_UNKNOWN_POSITION         10002
//#define ERR_INVALID_SPEED            10003
#define ERR_PORT_CHANGE_FORBIDDEN    10004

enum TriggerType {OFF, RISING_EDGES, FALLING_EDGES, BOTH_EDGES, FOLLOW_PULSE};
string TriggerLabels[] = {"Off","RisingEdges","FallingEdges","BothEdges","FollowPulse"};
char TriggerCmd[] = {'Z', '+', '-', '*', 'X'};

class PrecisExciteTCP : public CShutterBase<PrecisExciteTCP>
{
public:
	PrecisExciteTCP();
	~PrecisExciteTCP();

	// Device API
	// ----------
	int Initialize();
	int Shutdown();

	void GetName(char* pszName) const;
	bool Busy();
	//MM::DeviceType GetType() const {return MM::GenericDevice;}

	// Shutter API
	int SetOpen(bool open = true);
	int GetOpen(bool& open);
	int Fire(double deltaT);

	// action interface
	// ----------------
	int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnCommand(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnIP(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnOpeningTime(MM::PropertyBase* pProp, MM::ActionType eAct);
	// int OnClosingTime(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnShutterName(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnChannelLabel(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnIntensity(MM::PropertyBase* pProp, MM::ActionType eAct, long index);
	int OnState(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTrigger(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerSequence(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
	int ExecuteCommand(const std::string& cmd);
	//bool busy_;
	//bool initialized_;
	//// MMCore name of serial port
	//std::string port_;  
	std::string IP_;
	//// address of controller on serial chain (x, 0-7)
	//std::string address_;   
	//// Time that last command was sent to shutter
	//MM::MMTime changedTime_;
	//long intensity_;
	//long state_;
	//int error_; 
	long intensity_;
	long state_;
	int error_;

	bool initialized_;
	std::string name_;
	std::string tcpport_;
	char currentChannelLetter_;
	string currentChannelLabel_;
	long currentChannel_;
	//unsigned char buf_[1000];
	//string buf_string_;
	//vector<string> buf_tokens_;
	//unsigned long buf_bytes_;
	long armState_;
	TriggerType triggerMode_;

	bool busy_;
	double answerTimeoutMs_;
	vector<char> channelLetters_;
	vector<string> channelLabels_;
	string triggerSequence_;
	string triggerMessage_;
	string trigger_;
	MM::MMTime changedTime_;

	//TCP connection
	WSADATA wsaData;
	SOCKET ConnectSocket;


	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen;

	void ReadGreeting();
	 int ReadChannelLabels();
	void SetIntensity(long intensity, long index);
	void GetIntensity(long& intensity, long index);
	void SetState(long state);
	void GetState(long &state);
	void GeneratePropertyIntensity();
	void GeneratePropertyState();
	void GenerateChannelChooser();
	void GeneratePropertyTrigger();
	void GeneratePropertyTriggerSequence();
	void Illuminate();
	void SetTrigger();
	 int HandleErrors();

	//void Send(string cmd);
	//int SendTCPCommand( string sendData );
  	void    InitializeTCPVars();
	int ConnectTCP(string IP,string port);
	//void ReceiveOneLine();
	string GetTCPAnswer();
	string SendCommand(string  cmd );


	void StripString(string& StringToModify);
};
#endif // _PrecisExciteTCP_H_