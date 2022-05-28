#ifndef __LED_H
#define __LED_H
#include "sys.h"

//平台通用LED端口定义
#define LED1 PBout(5)
#define LED2 PBout(6)
#define LED3 PBout(3)
#define LED4 PBout(4)
#define LEDON	LED1=1
#define LEDOFF	LED1=0
#define USB_PULLUP_EN	;
#define USB_PULLUP_DIS	;
//根据DAC配置
#define DAC_EN	PAout(3)=1;
#define DAC_DIS	PAout(3)=0;

#define APWR_STAGE1_EN	PAout(1)=1;
#define APWR_STAGE1_DIS	PAout(1)=0;
#define APWR_EN		PAout(2)=1;
#define APWR_DIS	PAout(2)=0;
#define SEL_24		PAout(8)=0;PCout(6)=0;PCout(7)=1;
#define SEL_22		PAout(8)=1;PCout(6)=1;PCout(7)=0;
#define SEL_NONE	PAout(8)=1;PCout(6)=0;PCout(7)=0;


void Board_Init(void);//初始化		 				    
#endif
