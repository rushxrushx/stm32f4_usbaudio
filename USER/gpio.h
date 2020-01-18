#ifndef __LED_H
#define __LED_H
#include "sys.h"

//PCM1794 V0101带隔离主板	
#if HW_1794V1

#define LED1 PAout(1)
#define LED2 PAout(2)
#define LED3 PAout(3)
#define LED4 PAout(4)
#define LEDON	LED1=1
#define LEDOFF	LED1=0
#define USB_PULLUP_EN	PBout(13)=1;
#define USB_PULLUP_DIS	PBout(13)=0;
#define SEL_24	PAout(8)=1;
#define SEL_22	PAout(8)=0;
#define SEL_NONE	PAout(8)=0;
#define DAC_EN	PAout(12)=1;
#define DAC_DIS	PAout(12)=0;
#define APWR_STAGE1_EN	PBout(8)=1;
#define APWR_STAGE1_DIS	PBout(8)=0;
#define APWR_EN		PBout(9)=1;
#define APWR_DIS	PBout(9)=0;
    
#endif
 
//pcm1709 L001
#if HW_1794L1

//LED端口定义
#define LED1 PBout(3)
#define LED2 PBout(4)
#define LED3 PBout(5)
#define LED4 PBout(6)
#define LEDON	LED1=1
#define LEDOFF	LED1=0
#define USB_PULLUP_EN	;
#define USB_PULLUP_DIS	;
#define DAC_EN	PAout(3)=1;
#define DAC_DIS	PAout(3)=0;
#define APWR_STAGE1_EN	PAout(1)=1;
#define APWR_STAGE1_DIS	PAout(1)=0;
#define APWR_EN		PAout(2)=1;
#define APWR_DIS	PAout(2)=0;
#define SEL_24	PAout(8)=0;
#define SEL_22	PAout(8)=1;
#define SEL_NONE	PAout(8)=1;

#endif
 
//v4w子卡
#if HW_401V4W

#define LED1 PAout(1)
#define LED2 PAout(2)
#define LED3 PAout(3)
#define LED4 PAout(4)
//#define XTAL22 PAout(5)
//#define XTAL24 PAout(6)
#define SEL_24		PAout(5)=0;PAout(6)=1;
#define SEL_22		PAout(5)=1;PAout(6)=0;
#define SEL_NONE	PAout(5)=0;PAout(6)=0;
#define LEDON		LED1=1
#define LEDOFF	LED1=0
#define USB_PULLUP_EN	;
#define USB_PULLUP_DIS	;
#define DAC_EN	PBout(8)=0;
#define DAC_DIS	PBout(8)=1;
#define APWR_STAGE1_EN	;
#define APWR_STAGE1_DIS	;
#define APWR_EN		PBout(9)=1;
#define APWR_DIS	PBout(9)=0;

#endif


void Board_Init(void);//初始化		 				    
#endif
