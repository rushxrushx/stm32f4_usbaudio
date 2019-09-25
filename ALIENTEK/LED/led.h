#ifndef __LED_H
#define __LED_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


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

#define SEL_24	XTAL22=0;XTAL24=1;
#define SEL_22	XTAL24=0;XTAL22=1;
#define SEL_NONE	XTAL24=0;XTAL22=0;


void LED_Init(void);//初始化		 				    
#endif
