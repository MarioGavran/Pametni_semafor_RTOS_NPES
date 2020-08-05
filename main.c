/*************************************************************************************************************************
DOCUMENT:		main.c file of CMSIS RTX RTOS student project.  

						CMSIS RTOS Smart Traffic Light:
						- 2 inputs for buttons, used by pedestrians to reduce time waiting for a green light
						- 2 inputs for vehicle counting sensors.
						- Driver for controlling traffic lights on a simple intersection with 2 roads
				
AUTHOR:			Mario Gavran, E5029604, University of Maribor - FERI
*************************************************************************************************************************/

#define osObjectsPublic                 // define objects in main module
#include "osObjects.h"                  // RTOS object definitions
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "stm32f4xx.h"                  // Device header
#include "uart2.h"			// USART2 header
#include "traffic_lights.h"		// Traffic light driver

void initUART2(void);			//function prototipe for initUART2
void u2_sendCh (unsigned char ch);	//function prototipe for u2_sendCh

// Create thread handles:
osThreadId main_id;
osThreadId thread1_id;
osThreadId thread2_id;
osThreadId thread3_id;

// Function prototypes for threads:
void thread1 (void const *argument); 
void thread2 (void const *argument); 
void thread3 (void const *argument); 

// Thread structure definitions:
osThreadDef(thread1, osPriorityNormal, 1, 0); 
osThreadDef(thread2, osPriorityNormal, 1, 0); 
osThreadDef(thread3, osPriorityNormal, 1, 0); 

// Declare Mutex container and Mutex ID:
osMutexDef (uart2_mutex);			// Declare mutex.
osMutexId  uart2_mutex_id; 			// Mutex ID.

// Declare message queue and message id:
osMessageQDef(message_q, 1, uint32_t);		// Declare a message queue
osMessageQId 	message_q_id;           	// Declare an ID for the message queue


//*************************************************|      THREAD 0      |*************************************************
//*************************************************|     MAIN THREAD    |*************************************************
int main (void)
{
	
	osKernelInitialize ();			// Initialize RTOS kernal.
	main_id = osThreadGetId ();		// main ID.
		
	// Create the threads, starts them running, read theri thread IDs:
	thread1_id = osThreadCreate(osThread(thread1), NULL);
	thread2_id = osThreadCreate(osThread(thread2), NULL);
	thread3_id = osThreadCreate(osThread(thread3), NULL);
	
	// Start RTOS kernal:
	osKernelStart();
	
	// Initialize UART2:
	initUART2();													
	
	// Create Mutex:
	uart2_mutex_id = osMutexCreate(osMutex(uart2_mutex));
	
	while(1)
	{
		osMutexWait(uart2_mutex_id, osWaitForever);	// Acquire the mutex when peripheral access is required.
		u2_sendStr("T0:TEST UART AND MUTEX \r");	// Send functionality affirmation.
		osMutexRelease(uart2_mutex_id);			// Release the Mutex when finished with UART2 peripheral.
		osDelay(5000);
	}
}
//************************************************************************************************************************
//************************************************************************************************************************


//*************************************************|      THREAD 1      |*************************************************
//*************************************************|   Vehicle_Count    |*************************************************
void thread1 (void const *argument)
{
	// Initialize timer TIM1:
	RCC->APB2ENR	|= RCC_APB2ENR_TIM1EN;			// Enable clock on TIM1.
	TIM1->CR1	|= TIM_CR1_CEN;				// Enable counter.
	TIM1->PSC 	=  30000;				// Set prescaler.
	
	// Create the message queue in a thread1:
	message_q_id = osMessageCreate(osMessageQ(message_q), thread3_id);		
	
	unsigned int data	= 0;				// Data that will be sent.
	unsigned int hvc	= 0;				// Horizontal vehicle count.
	unsigned int vvc	= 0;				// Vertical vehicle count.
	
	osStatus status;					// Data field, containing event status and error codes.
	
	while(1)
	{	
		if((GPIOA->IDR & VEHICLE_V) == VEHICLE_V)	// Test if vehicle count sensors are activated:
		{
			vvc++;
		}
		if((GPIOA->IDR & VEHICLE_H) == VEHICLE_H)
		{
			hvc++;
		}		
		if((TIM1->CNT) >= 65000)			// Every 2 min resets vehicle count variables:
		{
			hvc=0;
			vvc=0;
		}
		data = (hvc & (0x0000FFFF)) | ((vvc & (0x0000FFFF))<<16);	// Prepare data for sending:
			
		status = osMessagePut(message_q_id, data, 200);			// Try to send message every 200ms:
		
		if(status == osOK)						// Check status:
		{
			osMutexWait(uart2_mutex_id, osWaitForever);
			u2_sendStr("T1:SENT \r");
			osMutexRelease(uart2_mutex_id);
		}
		else if(status == osErrorTimeoutResource)
		{
			osMutexWait(uart2_mutex_id, osWaitForever);
			u2_sendStr("T1:timer exceeded\r");
			osMutexRelease(uart2_mutex_id);
		}
		
	}
}
//************************************************************************************************************************
//************************************************************************************************************************



//*************************************************|      THREAD 2      |*************************************************
//*************************************************| Pedestrian_Request |*************************************************
void thread2 (void const *argument)
{
	// Macros for signal flags:
	#define H_PEDESTRIAN	0x00000001
	#define V_PEDESTRIAN	0x00000002
		
	while(1)
	{	
		// Test if pedestrian buttons are pressed, and set signals accordingly:
		if(((GPIOA->IDR & PEDES_H) == PEDES_H) && ((GPIOA->IDR & RED_H_i) == RED_H_i) )
		{
			osSignalSet (thread3_id, H_PEDESTRIAN);
		}
		else if(((GPIOA->IDR & PEDES_V) == PEDES_V) && ((GPIOA->IDR & RED_V_i) == RED_V_i) )
		{
			osSignalSet (thread3_id, V_PEDESTRIAN);
		}
		
		osDelay(200);
		
	}
}
//************************************************************************************************************************
//************************************************************************************************************************



//***********************************************|        THREAD 3        |***********************************************
//***********************************************|Traffic_Light_Controller|***********************************************
void thread3 (void const *argument)
{
	// Initialize GPIO pins for driving traffic light: 
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER10_0	| GPIO_MODER_MODER6_0	| GPIO_MODER_MODER7_0;	
	GPIOA->MODER |=	GPIO_MODER_MODER0_0	| GPIO_MODER_MODER1_0	| GPIO_MODER_MODER4_0;
	
	// Delay variables:
	signed int hor_delay	= 4000;
	signed int ver_delay	= 4000;
	
	unsigned int message	= 0;			// Message temp variable
	unsigned int hvc1 	= 0;			// Variable for horizontal vehicle count.
	unsigned int vvc1	= 0;			// Variable for vertical vehicle count.
	
	int signal_flags=0;				// Variable for reciving flags.
	osEvent event_message;				// Define event structure for messages.
	osEvent event_signal;				// Define event structure for signals.
	
	// Start traffic light blinking test.
	blink_test();
	osDelay(2000);
	initUART2();
	
	while(1)
	{
		HORon_VERoff();							// Start with horizontal green and vertical red.	
		event_message = osMessageGet(message_q_id, osWaitForever);	// Wait for message until it arrives.
		
		if(event_message.status == osEventMessage)			// Check if message is recived properly.
		{
			osMutexWait(uart2_mutex_id, osWaitForever);
			u2_sendStr("T3:RECIVED \r");				// *debugging
			osMutexRelease(uart2_mutex_id);
			
			message = event_message.value.v;			// Reconstruct data from message.
			hvc1 = message & 0x0000FFFF;
			vvc1 = (message & 0xFFFF0000) >> 16;
			
			// Set delay for horizontal green:
			if(hvc1 > vvc1)		
			{
				hor_delay = 4000 + (200*(hvc1-vvc1));
			}
			else if(hvc1 < vvc1)		
			{
				hor_delay = 4000 - (200*(vvc1-hvc1));
			}
			
			// Reset vehicle count variables:
			hvc1=0;
			vvc1=0;
			
			// Green light must not be less than 500ms:
			if(hor_delay <= 500)
			{
				hor_delay = 500;
			}
		}
			
			// Reciveing signals:
			event_signal = osSignalWait(0,(hor_delay/2));	// Wait for signal to apear, for half of calculated delay.
						
			signal_flags = event_signal.value.signals;	// Save recived signal, from event structure.
			
			if(signal_flags == V_PEDESTRIAN)		// Test if it is proper flag.
			{
				osDelay((hor_delay / 4));		// If it is, do only 1/4 of calculated delay. 
			}
			else
			{
				osDelay(((hor_delay/2) + (hor_delay/4)));	// If it is not, do the other half of delay.
			}
			
			signal_flags = 0;				// Clear flags before reciving another signal.
			

			HORoff_VERon();					// Continue driving horizontal red and vertical green.
			
			
			event_message = osMessageGet(message_q_id, osWaitForever);	// Wait for message until it arrives.
		
			if(event_message.status == osEventMessage)		// Check if message is recived properly.
			{
				osMutexWait(uart2_mutex_id, osWaitForever);
				u2_sendStr("T3:PRIMLJENO \r");			// *debugging
				osMutexRelease(uart2_mutex_id);
				
				message = event_message.value.v;		// Reconstruct data from message.
				hvc1 = message & 0x0000FFFF;
				vvc1 = (message & 0xFFFF0000) >> 16;
				
				// Set delay for horizontal green:
				if(hvc1 > vvc1)
				{
					ver_delay = 3000 - (200*(hvc1-vvc1));
				}
				else if(hvc1 < vvc1)				//to je vertical
				{
					ver_delay = 3000 + (200*(vvc1-hvc1));
				}
				
				// Reset vehicle count variables:
				hvc1=0;
				vvc1=0;
				
				// Green light must not be less than 500ms:
				if(ver_delay <= 500)
				{
					ver_delay = 500;
				}
			}
			
			// Reciveing signals:
			event_signal = osSignalWait(0,(ver_delay/2));		// Wait for signal to apear, for half of the calculated delay.
			
			signal_flags = event_signal.value.signals;		// Save recived signal, from event structure.
			
			if(signal_flags == H_PEDESTRIAN)			// Test if it is proper flag.
			{
				osDelay((ver_delay/4));				// If it is, do only a 1/4 of calculated delay. 
			}
			else
			{
				osDelay(((ver_delay/2) + (ver_delay/4)));	// If it is not, do the other half of delay.
			}
			
			signal_flags = 0;					// Clear flags before reciving another signal.	
	}
}
//************************************************************************************************************************
//************************************************************************************************************************
