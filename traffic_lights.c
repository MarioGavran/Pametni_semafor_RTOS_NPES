/*************************************************************************************************************************
LIBRARY: 	traffic_lights.c

			Traffic light driver.

			Only for STM32F410RBT8 devices.

			Declares functions for mutually excluding states of traffic lights in	each track, 
			and blinking test for testing all traffic lights, used to ensure all lights are working properly.

AUTHOR: 	Mario Gavran, E5029604, University of Maribor - FERI
*************************************************************************************************************************/



#include "traffic_lights.h"	// Traffic lights driver header
#include "stm32f4xx.h"          // Device header

void blink_test(void)
{
	int c=0;
	while(c<10)		// Loop through test 10 time
	{
		c++;
		GPIOA->ODR	|= RED_H | YELLOW_H | GREEN_H;
		GPIOA->ODR	|= RED_V | YELLOW_V | GREEN_V;
		osDelay(300);
			
		GPIOA->ODR	&= ~(RED_H | YELLOW_H | GREEN_H);
		GPIOA->ODR	&= ~(RED_V | YELLOW_V | GREEN_V);
		osDelay(300);
	}
	c=0;
}
	
	
void HORon_VERoff(void)
{
	GPIOA->ODR	|= RED_H|YELLOW_H;
	GPIOA->ODR	&= ~(GREEN_H);
		
	GPIOA->ODR	&= ~(GREEN_V | RED_V);
	GPIOA->ODR	|= YELLOW_V;
		
	osDelay(1000);
		
	GPIOA->ODR	&= ~(YELLOW_H | RED_H);
	GPIOA->ODR	|= GREEN_H;
		
	GPIOA->ODR	&= ~(YELLOW_V | GREEN_V);
	GPIOA->ODR	|= RED_V;
}



void HORoff_VERon(void)
{
	GPIOA->ODR	|= RED_V | YELLOW_V;
	GPIOA->ODR	&= ~(GREEN_V);
		
	GPIOA->ODR	&= ~(GREEN_H | RED_H);
	GPIOA->ODR	|= YELLOW_H;
		
	osDelay(1000);
		
	GPIOA->ODR	&= ~(YELLOW_V | RED_V);
	GPIOA->ODR	|= GREEN_V;
	
	GPIOA->ODR	&= ~(YELLOW_H | GREEN_H);
	GPIOA->ODR	|= RED_H;		
}

