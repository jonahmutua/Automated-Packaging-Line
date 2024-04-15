/* maximum size of producct buffer*/
  #define MAX_FIFO_SIZE 256

/*Description: This Device is used to detects rising/falling edge of input signal
 *INPUT: CLK
 *OUTPUT: Q(of type bool)
 */
typedef struct edgeDetectorDevice{
	BOOL CLK; //input CLOCK signal
	BOOL previousIN; //previous CLOCK signal state
	
	BOOL Q; //output
	
	
	}edgeDetectorDevice;

 /*this function block is used to detect rising/falling edge of input signal
  *@param edgeDetectorDevicept - pointer to edge detector device structure
  *@detectFallingEdge - BOOL  - variable that tells the function wether to
  *detect risingedge / fallingEdge
  *OUTPUT: bOut(BOOL);
  */
 void FB_edgeDetector(edgeDetectorDevice *edgeDetectorDeviceptr, BOOL detectFallingEdge);

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

void FB_edgeFilter(edgeFilterDevice *filterDevice, BOOL detectFallingEdge);

/* Device states */
typedef enum DEVICE_STATE{
	 DEVICE_GET=0,
	 DEVICE_WAIT_SET,
	 DEVICE_WAIT_RESET
	
	}DEVICE_STATE;

/* A Trigger Generator Device structure */
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

/* generates signal of parameterizable length */
/* INPUT: pointer to triggerGenerator Device */
/* OUTPUT: void  */
void FB_trigGenerator(trigGeneratorDevice *triggerGenerator);
