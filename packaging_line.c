
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

/* This function block is used to filter out unwanted signal */
/* @param filterDevice -  a pointer to edge filter device structcure */
/* @param detectFallingEdge - a variable that indicate wether to begin filtering */
/* if falling edge or rising edge is detected */

void FB_edgeFilter(edgeFilterDevice *filterDevice, BOOL detectFallingEdge){
		if(filterDevice->bReset){/*Todo:  Reset filter Device here*/}
			  
			if(filterDevice->bIsFiltering==FALSE)/*Filter  Device is Not Busy*/{
				filterDevice->edgeDetector->CLK=filterDevice->bIn;
				FB_edgeDetector(filterDevice->edgeDetector,detectFallingEdge);
				filterDevice->bOut=filterDevice->edgeDetector->Q;
				
				if(filterDevice->bOut){
					filterDevice->bIsFiltering=TRUE;
					filterDevice->nNextValid=filterDevice->nActual+filterDevice->nLength;
				}
			}else/*Filter Device is Busy*/{
				if(filterDevice->nActual >= filterDevice->nNextValid ){
					filterDevice->bIsFiltering=FALSE;
				}
				FB_edgeDetector(filterDevice->edgeDetector,FALSE);
				FB_edgeDetector(filterDevice->edgeDetector,TRUE); 
				filterDevice->bOut=filterDevice->edgeDetector->Q; 
				 //filterDevice->bOut=FALSE;
			}
		
	
	}
/* This function block is used to generate a signal of paramaterisable length to trigger devices */
/* @param triggerGenerator - pointer to trig generator device structure */

void FB_trigGenerator(trigGeneratorDevice *triggerGenerator){
		triggerGenerator->bOut=FALSE;
		triggerGenerator->bRising=FALSE;
		
		if(triggerGenerator->bReset){
			triggerGenerator->nIndexIn=0;
			triggerGenerator->nIndexOut=0;
			*triggerGenerator->deviceState=DEVICE_GET;
			triggerGenerator->nCount=0;
			triggerGenerator->bReset=FALSE;
	     }else if(triggerGenerator->bTrig){
			 triggerGenerator->nBuffer[triggerGenerator->nIndexIn]=triggerGenerator->nValue;
			 triggerGenerator->nIndexIn++;
			 triggerGenerator->nCount++;
			 triggerGenerator->bTrig=FALSE;
			 
			 if(triggerGenerator->nIndexIn>=MAX_FIFO_SIZE) triggerGenerator->nIndexIn=0;
			
		 }else if(*triggerGenerator->deviceState==DEVICE_GET && triggerGenerator->nCount==0)
			/*don't POP an empty FIFO, we can set error flag just before we return*/ return;
			 
			 switch(*triggerGenerator->deviceState){
				        case DEVICE_GET:
						             if(triggerGenerator->nCount>0){
										 
										 triggerGenerator->nNextSet=triggerGenerator->nBuffer[triggerGenerator->nIndexOut];
										 triggerGenerator->nBuffer[triggerGenerator->nIndexOut]=0;
										 triggerGenerator->nIndexOut++;
										 triggerGenerator->nCount--;
										 if(triggerGenerator->nIndexOut >= MAX_FIFO_SIZE ) triggerGenerator->nIndexOut=0;
									 }
				                    *triggerGenerator->deviceState=DEVICE_WAIT_SET;
									break;
				 
				         case  DEVICE_WAIT_SET:
						              if(triggerGenerator->nActualSet >= triggerGenerator->nNextSet){
										  triggerGenerator->bOut=TRUE;
										  triggerGenerator->bRising=TRUE;
										  triggerGenerator->nNextReset=triggerGenerator->nActualSet+triggerGenerator->nLength;
										  triggerGenerator->nTrigCount++;
										  *triggerGenerator->deviceState=DEVICE_WAIT_RESET;
										  
									  }
									  break;
									  
					      case  DEVICE_WAIT_RESET:
						               if(triggerGenerator->nActualReset >= triggerGenerator->nNextReset){
										   triggerGenerator->bOut=FALSE;
										   *triggerGenerator->deviceState=DEVICE_GET;
									   } else triggerGenerator->bOut=TRUE;
									   break;
							
					      default:
						        *triggerGenerator->deviceState=DEVICE_GET;
				 
			 }
}













