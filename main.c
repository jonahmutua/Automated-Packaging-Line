/*
 * InputFilter
 *
 * Created: 3/3/2023 1:19:16 PM
 * Author : jonah
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_FIFO_SIZE 256


/* 8----->PH5------->productSensor INPUT     6- PH3----output(trig printer)
 *13----->PB7------->edgeDetectedout 
 *7------>PH4------->Encoder INPUT 
 */
/*
 *               ___________
 *            - |           |-->13 (OUTPUT -edge detected)
 *              |           |-->8  (productDetector sensor INPUT)
 *              |           |-->7  (INPUT-Encoder simulator)
 *              |  ATMEGA   |-->6  OUT (trigPrinter )
 *              |   2560    |
 *              |           |
 *              |           |
 *              |           |
 *              |___________|
 */
 #define ENCODER PH4
 #define TRIGSENSOR PH5
 #define PULSEOUT PH7
 #define TRIGPRINTER PH3
 
typedef uint8_t BOOL;
BOOL TRUE=1;
BOOL FALSE=0;

/*product detector variables*/
uint16_t productDetectorFilterValue=0; //current position/time 
uint16_t productDtectorFilterLength=5; //product Detector Filter length/duration to prevent glitches on signal
uint16_t encoderValue=0;   //holds encoder pulses

/*PRINNTER Var*/
uint16_t printerDistanceValue=10;
uint16_t printerLength=3;

/*Description: This Device is used detects rising/falling edge of input signal
 *INPUT: CLK
 *OUTPUT: Q(of type bool)
 */
typedef struct edgeDetectorDevice{
	BOOL CLK; //input CLOCK signal
	BOOL previousIN; //previous CLOCK signal state
	
	BOOL Q; //output
	
	
	}edgeDetectorDevice;
	
 /*this function block is used to detect rising/falling edge of input signal
  * INPUT pointer to edgeDetectorDevice
  *OUTPUT: bOut(BOOL);
  */
 void FB_edgeDetector(edgeDetectorDevice *edgeDetectorDeviceptr, BOOL detectFallingEdge){
	//productDetectctorFilter->bIn=((PINH & (1<<PH5))>>PH5);  /*read the product detector sensor*/ if(edgeDetectorDeviceptr->Q) 
	 edgeDetectorDeviceptr->Q=FALSE;//ensure one pulse during edge detection
	 
	 if(edgeDetectorDeviceptr->CLK != edgeDetectorDeviceptr->previousIN){
		 if(detectFallingEdge){/*detect falling edge*/
			 if(edgeDetectorDeviceptr->CLK==FALSE)     edgeDetectorDeviceptr->Q=TRUE;
			
			 }else {/*detect rising edge*/
		       
			  if(edgeDetectorDeviceptr->CLK==TRUE) edgeDetectorDeviceptr->Q=TRUE;
				   
			      }
		  
	 }
       edgeDetectorDeviceptr->previousIN=edgeDetectorDeviceptr->CLK; //update previousState of signal to current
	   
 }
/* Description:Device to filter input signal
 *INPUT :bIN=signal to filter
 *OUTPUT :bOut=filtered signal
 */
typedef struct edgeFilterDevice{
	BOOL bIn; //(*signal to filter*)
	uint16_t nActual; //actual value to set computing
	uint16_t nLength; //filter length
	BOOL bReset;
	BOOL bOut; /*output*/
	/*internal variables*/
	uint16_t nNextValid;
	BOOL bIsFiltering;
	
	edgeDetectorDevice *edgeDetector;
	
	
	}edgeFilterDevice;
	
	void FB_edgeFilter(edgeFilterDevice *filterDevice, BOOL detectFallingEdge){
		if(filterDevice->bReset){/*Todo: To reset all params*/}
			  
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
	
typedef enum DEVICE_STATE{
	 DEVICE_GET=0,
	 DEVICE_WAIT_SET,
	 DEVICE_WAIT_RESET
	
	}DEVICE_STATE;
	
typedef struct trigGeneratorDevice{
	BOOL bTrig; //push value flag
	uint16_t nValue; // pushed value
	uint16_t nActualSet;// actual position/time used for SET computing
	uint16_t nActualReset;//actual position/time used for RESET computing
	uint16_t nLength;//Pulse length
	BOOL bReset;
	
	BOOL bOut; //output
	BOOL bRising;
	uint16_t nTrigCount;
	
	DEVICE_STATE *deviceState;
	uint16_t nNextSet;
	uint16_t nNextReset;
	uint16_t nIndexIn;
	uint16_t nIndexOut;
	uint16_t nCount;
	uint16_t nBuffer[MAX_FIFO_SIZE];
	
	
	}trigGeneratorDevice;
	
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
	
int main(void)
{
	
/* 8----->PH5------->encoder INPUT
 *13----->PB7------->edgeDetectedout  
 */
 DDRH  |=~(1<<DDH5); //pin8 input------>connect sensor
 PORTH &= ~(1<<PH5) ; //enable internal pullup
 
 DDRH  |=~(1<<DDH4); //pin7 input------>encoder
 PORTH &= ~(1<<PH4) ; //enable internal pullup
 
 DDRB |=(1<<DDB7); //pin13 output----->connect OSC 
 PORTB &=~(1<<PB7); //pull pin13 down
 
 DDRH |=(1<<DDH3); //pin6 output----->connect OSC
 PORTH &=~(1<<PH3); //pull pin6 down
 
 /*
  edgeDetectorDevice * bProductDetector;
  bProductDetector->previousIN=FALSE;
  bProductDetector->CLK=FALSE;
  bProductDetector->Q=FALSE;
  */
  
  /***************product counter edge filter device****************/
  edgeFilterDevice  filterDevice; //// create struct variable
  edgeFilterDevice *productDetectctorFilter=&filterDevice; // define a pointer with initialization
  edgeDetectorDevice device; // create struct variable
  productDetectctorFilter->edgeDetector=&device; //edgeDetector initialized to a valid address.( define a pointer with initialization)
  
  edgeDetectorDevice encoderE={.CLK=FALSE,.previousIN=FALSE,.Q=FALSE};
  edgeDetectorDevice *encoderEdgeDetector=&encoderE;
  
  productDetectctorFilter->edgeDetector->CLK=FALSE;
  productDetectctorFilter->edgeDetector->previousIN=FALSE;
  productDetectctorFilter->edgeDetector->Q=FALSE;
  productDetectctorFilter->bIn=FALSE;
  productDetectctorFilter->bIsFiltering=FALSE;
  productDetectctorFilter->bOut=FALSE;
  productDetectctorFilter->nLength=0;
  productDetectctorFilter->nActual=0;
  
  
  /****************PRINTER TRIGGER DEVICE************************/
  //trigGeneratorDevice  printerDevice;
  /*good practice to initialize struct*/
  trigGeneratorDevice  printerDevice={.bTrig=FALSE,.nBuffer={0},.nCount=0,.bOut=FALSE,.nIndexIn=0,.nIndexOut=0,.nValue=0,.nLength=0};
  trigGeneratorDevice *printerTriggerDevice=&printerDevice;
  DEVICE_STATE stateEnum=DEVICE_GET;/*good practice to initialize enum to default value*/
  printerTriggerDevice->deviceState=&stateEnum;
  
  
 
 
    while (1) 
    {   
		/*detect encoder rising edges*/
		
		
		encoderEdgeDetector->CLK=((PINH & (1<<ENCODER))>>ENCODER); //read encoder status
		FB_edgeDetector(encoderEdgeDetector,FALSE);  //detect encoder pulse
		if(encoderEdgeDetector->Q) encoderValue++;   //increament encoder pulses if edge was detected;
		
		/*******product Detection*********/
		productDetectorFilterValue= encoderValue;
		productDetectctorFilter->bIn=((PINH & (1<<PH5))>>PH5);  /*read the product detector sensor*/
		productDetectctorFilter->nActual=productDetectorFilterValue;
		productDetectctorFilter->nLength=productDtectorFilterLength;
		FB_edgeFilter(productDetectctorFilter,FALSE);   /*filter productDetector */
		
		/*****Trigger PRINTER******************/
		
		printerTriggerDevice->bTrig=productDetectctorFilter->bOut;
		printerTriggerDevice->nValue=encoderValue+printerDistanceValue;
		printerTriggerDevice->nActualSet=encoderValue;
		printerTriggerDevice->nActualReset=encoderValue;
		printerTriggerDevice->nLength=printerLength;
		FB_trigGenerator(printerTriggerDevice);
		
	   if(printerTriggerDevice->bOut){
		   PORTH |=(1<<TRIGPRINTER);
		   }else{
		   PORTH &=~(1<<TRIGPRINTER);
	   }
	   
		if(productDetectctorFilter->bOut){
		PORTB |=(1<<PB7);
		}else{
			PORTB &=~(1<<PB7);
		}
	
    }
}



