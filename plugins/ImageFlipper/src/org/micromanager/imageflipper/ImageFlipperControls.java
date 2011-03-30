/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * ImageFlipperControls.java
 *
 * Created on Feb 18, 2011, 9:57:34 PM
 */

package org.micromanager.examples.imageflipper;

import mmcorej.TaggedImage;
import org.micromanager.api.DataProcessor;

/**
 *
 * @author arthur
 */
public class ImageFlipperControls extends javax.swing.JFrame {
   private final ImageFlippingProcessor processor_;

    /** Creates new form ImageFlipperControls */
    public ImageFlipperControls() {
        initComponents();
        processor_ = new ImageFlippingProcessor(this);
    }

    public DataProcessor<TaggedImage> getProcessor() {
       return processor_;
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
   // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
   private void initComponents() {

      flipCheckBox = new javax.swing.JCheckBox();
      mirrorCheckBox = new javax.swing.JCheckBox();
      rotateCheckBox = new javax.swing.JCheckBox();

      setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);

      flipCheckBox.setText("Flip");
      flipCheckBox.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            flipCheckBoxActionPerformed(evt);
         }
      });

      mirrorCheckBox.setText("Mirror");
      mirrorCheckBox.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            mirrorCheckBoxActionPerformed(evt);
         }
      });

      rotateCheckBox.setText("Rotate");
      rotateCheckBox.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            rotateCheckBoxActionPerformed(evt);
         }
      });

      org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
      getContentPane().setLayout(layout);
      layout.setHorizontalGroup(
         layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
         .add(layout.createSequentialGroup()
            .add(66, 66, 66)
            .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
               .add(flipCheckBox)
               .add(mirrorCheckBox)
               .add(rotateCheckBox))
            .addContainerGap(101, Short.MAX_VALUE))
      );
      layout.setVerticalGroup(
         layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
         .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
            .addContainerGap(82, Short.MAX_VALUE)
            .add(flipCheckBox)
            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
            .add(mirrorCheckBox)
            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
            .add(rotateCheckBox)
            .add(149, 149, 149))
      );

      pack();
   }// </editor-fold>//GEN-END:initComponents

    private void flipCheckBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_flipCheckBoxActionPerformed

    }//GEN-LAST:event_flipCheckBoxActionPerformed

    private void mirrorCheckBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mirrorCheckBoxActionPerformed
       // TODO add your handling code here:
    }//GEN-LAST:event_mirrorCheckBoxActionPerformed

    private void rotateCheckBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_rotateCheckBoxActionPerformed
       // TODO add your handling code here:
    }//GEN-LAST:event_rotateCheckBoxActionPerformed


    
    /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new ImageFlipperControls().setVisible(true);
            }
        });
    }

   // Variables declaration - do not modify//GEN-BEGIN:variables
   private javax.swing.JCheckBox flipCheckBox;
   private javax.swing.JCheckBox mirrorCheckBox;
   private javax.swing.JCheckBox rotateCheckBox;
   // End of variables declaration//GEN-END:variables

   public boolean getFlip() {
      return flipCheckBox.isSelected();
   }

   public boolean getMirror() {
      return mirrorCheckBox.isSelected();
   }

   public boolean getRotate() {
      return rotateCheckBox.isSelected();
   }

}