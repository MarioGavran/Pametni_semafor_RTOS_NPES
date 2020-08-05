/************************************************************************************************************************
LIBRARY:	uart2.c

			- transmitter and reciever trough UART2 with virtual COM port.

			- only for STM32F410RBT8 devices.

			- declares functions for initializing the interface, sending chars, sending strings and recieving chars.

AUTHOR: 	Mario Gavran, E5029604, University of Maribor - FERI
*************************************************************************************************************************/

#include "stm32f4xx.h"		// Device header.
#include "uart2.h"		// UART2 header file.


void initUART2(void)
{
	// Clock initialization for USART2 peripheral.
	RCC->APB1ENR	|= RCC_APB1ENR_USART2EN;	//enable USART2
	RCC->AHB1ENR	|= RCC_AHB1ENR_GPIOAEN;		//enable PORTA
	
	// Initialize USART2 GPIO pins: 
	GPIOA->MODER	|= GPIO_MODER_MODE2_1;						//Set PA2 AF.
	GPIOA ->AFR[0]	|= GPIO_AFRL_AFRL2_0 | GPIO_AFRL_AFRL2_1 | GPIO_AFRL_AFRL2_2;	// turn on gpio AF7  on PA2.
	GPIOA->MODER	|= GPIO_MODER_MODE3_1;						//Set PA3 AF.
	GPIOA ->AFR[0] 	|= GPIO_AFRL_AFRL3_0 | GPIO_AFRL_AFRL3_1 | GPIO_AFRL_AFRL3_2;	// turn on gpio AF7  on PA3.
		
	USART2->CR1 |= USART_CR1_UE;							// USART enable.
	USART2->BRR |= 1667;								// 9600bps.
	USART2->CR1 |= USART_CR1_TE;							// Transmitter enable.
	USART2->CR1 |= USART_CR1_RE;							// Reciver enable.

}
//************************************************************************************************************************
//**************************************************|     SEND CHAR    |**************************************************
void u2_sendCh (unsigned char ch)
{
	while (!(USART2->SR & USART_SR_TXE)){}
  	USART2->DR = ch;
}
//************************************************************************************************************************


//************************************************************************************************************************
//**************************************************|    SEND STRING   |**************************************************
void u2_sendStr(unsigned char *s)
{
	while (*s)
	{
		u2_sendCh(*s++);
	}
}
//************************************************************************************************************************


//************************************************************************************************************************
//**************************************************|    RECIVE CHAR   |**************************************************
unsigned char  u2_getCh(void)
{
 	while (!(USART2->SR & USART_SR_RXNE));
	return (int)USART2->DR;
}
//************************************************************************************************************************

