#include "gpio.h" 
#include "usart.h"  
 
//HS board

void Board_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
// PA1-4 led
// PB8 22mhz
// PB9 24mhz
// PC10-11 GPO
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_4;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
//PA8 clock out 24MHZ  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  if (SystemCoreClock == 96000000)	RCC_MCO1Config(RCC_MCO1Source_PLLCLK,RCC_MCO1Div_4);//96/4=24
  else if (SystemCoreClock == 48000000)	RCC_MCO1Config(RCC_MCO1Source_PLLCLK,RCC_MCO1Div_2);//48/2=24
  else printf("clock must 48m/96m for HS.\r\n");
  
//reset DAC
  DAC_DIS;

//enable 5v and VEE power 	
  APWR_STAGE1_EN;
  
  

}

