/*
 * InputFilter
 *
 * Created: 3/3/2023 1:19:16 PM
 * Author : jonah
 */ 

#include "packagin_line.h"

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
 



/*product detector variables*/
uint16_t productDetectorFilterValue=0; //current position/time 
uint16_t productDtectorFilterLength=5; //product Detector Filter length/duration to prevent glitches on signal
uint16_t encoderValue=0;   //holds encoder pulses

/*PRINNTER Var*/
uint16_t printerDistanceValue=10;
uint16_t printerLength=3;


	
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



