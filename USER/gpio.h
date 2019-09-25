#ifndef __LED_H
#define __LED_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 

////////////////////////////////////////////////////////////////////////////////// 	
#if 0

//LED端口定义
#define LED1 PAout(1)
#define LED2 PAout(2)
#define LED3 PAout(3)
#define LED4 PAout(4)
#define XTAL22 PAout(5)
#define XTAL24 PAout(6)

#define LEDON	LED1=1
#define LEDOFF	LED1=0


#define USB_PULLUP_EN	;
#define USB_PULLUP_DIS	;
#define DAC_EN	;
#define DAC_DIS	;
#define APWR_EN	;
#define APWR_DIS	;

#define SEL_24	XTAL22=0;XTAL24=1;
#define SEL_22	XTAL24=0;XTAL22=1;
#define SEL_NONE	XTAL24=0;XTAL22=0;

#endif


//////////////////////////////////////////////////////////////////////////////////	 

////////////////////////////////////////////////////////////////////////////////// 	 	
#if 1

//LED端口定义
#define LED1 PBout(3)
#define LED2 PBout(4)
#define LED3 PBout(5)
#define LED4 PBout(6)

#define XTAL22 PAout(8)

#define LEDON	LED1=1
#define LEDOFF	LED1=0


#define USB_PULLUP_EN	;
#define USB_PULLUP_DIS	;
#define DAC_EN	PAout(3)=1;
#define DAC_DIS	PAout(3)=0;
#define APWR_EN		PAout(2)=1;
#define APWR_DIS	PAout(2)=0;

#define SEL_24	XTAL22=0;
#define SEL_22	XTAL22=1;
#define SEL_NONE	XTAL22=1;

#endif



void Board_Init(void);//初始化		 				    
#endif
