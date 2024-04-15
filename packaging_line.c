
#include "packaging_line.h"

 /*this function block is used to detect rising/falling edge of input signal
  *@param edgeDetectorDeviceptr - pointer to edge detector device structure
  *@detectFallingEdge - BOOL  - variable that tells the function wether to
  *detect risingedge / fallingEdge
  *OUTPUT: bOut(BOOL);
  */
 void FB_edgeDetector(edgeDetectorDevice *edgeDetectorDeviceptr, BOOL detectFallingEdge) {
    edgeDetectorDeviceptr->Q=FALSE;// reset egdedetctor device output
	 
	 if(edgeDetectorDeviceptr->CLK != edgeDetectorDeviceptr->previousIN){ /*current state of input singal differ from previous state?*/
		 if(detectFallingEdge){/*detect falling edge*/
			 if(edgeDetectorDeviceptr->CLK==FALSE)     edgeDetectorDeviceptr->Q=TRUE;
			
			 }else {/*detect rising edge*/
		       
			  if(edgeDetectorDeviceptr->CLK==TRUE) edgeDetectorDeviceptr->Q=TRUE;
				   
			      }
		  
	 }
       edgeDetectorDeviceptr->previousIN=edgeDetectorDeviceptr->CLK; //update previousState of signal to current state
 }
