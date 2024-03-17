#ifndef __LED_H
#define __LED_H
#include "sys.h"
 
//v4w子卡

#define LED1 PAout(1)
#define LED2 PAout(2)
#define LED3 PAout(3)
#define LED4 PAout(4)

#define LEDON	LED1=0;LED2=1;
#define LEDOFF	LED1=0;LED2=0;
#define LED_YEL	LED1=1;LED2=1;
#define LED_RED	LED1=1;LED2=0;

#define LED2_GRN	LED3=0;LED4=1;
#define LED2_YEL	LED3=1;LED4=1;
#define LED2_RED	LED3=1;LED4=0;
#define LED2_OFF	LED3=0;LED4=0;
//#define XTAL22 PAout(5)
//#define XTAL24 PAout(6)
#define SEL_24		PAout(5)=0;PAout(6)=1;
#define SEL_22		PAout(5)=1;PAout(6)=0;
#define SEL_NONE	PAout(5)=0;PAout(6)=0;

#define USB_PULLUP_EN	;
#define USB_PULLUP_DIS	;
#define DAC_EN	if(working_samplerate>48000) PBout(9)=1;else PBout(9)=0;PBout(8)=0;
#define DAC_DIS	PBout(8)=1;
#define APWR_STAGE1_EN	;
#define APWR_STAGE1_DIS	;
#define APWR_EN		
#define APWR_DIS	



void Board_Init(void);//初始化		 				    
#endif
