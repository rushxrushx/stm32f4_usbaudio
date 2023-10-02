#include "usbd_usr.h" 
#include <stdio.h> 
#include <usart.h> 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//USBD-USR 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/7/21
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   

//表示USB连接状态
//0,没有连接;
//1,已经连接;
vu8 bDeviceState=0;		//默认没有连接  

//指向DEVICE_PROP结构体
//USB Device 用户回调函数. 
USBD_Usr_cb_TypeDef USR_cb =
{
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,
  USBD_USR_DeviceConnected,
  USBD_USR_DeviceDisconnected,    
};
//USB Device 用户自定义初始化函数
void USBD_USR_Init(void)
{
	//printf("USBD_USR_Init\r\n");
} 
//USB Device 复位
//speed:USB速度,0,高速;1,全速;其他,错误.
void USBD_USR_DeviceReset (uint8_t speed)
{
	switch (speed)
	{
		case USB_OTG_SPEED_HIGH:
			printf("dReset[HS]\r\n");
			break; 
		case USB_OTG_SPEED_FULL: 
			printf("dReset[FS]\r\n");
			break;
		default:
			printf("dReset[??]\r\n"); 
			break;
	}
}
//USB Device 配置成功
void USBD_USR_DeviceConfigured (void)
{
	bDeviceState=1;
	printf("dConfigured\r\n"); 
} 
//USB Device挂起
void USBD_USR_DeviceSuspended(void)
{
	bDeviceState=10;
	printf("dSuspend\r\n");
} 
//USB Device恢复
void USBD_USR_DeviceResumed(void)
{ 
	bDeviceState=1;
	printf("dResumed\r\n");
}
//USB Device连接成功
void USBD_USR_DeviceConnected (void)
{
	printf("dConnected\r\n");
}
//USB Device未连接
void USBD_USR_DeviceDisconnected (void)
{
	printf("dDisconn\r\n");
} 


















