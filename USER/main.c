//USB异步声卡实验
//数码之家 RUSH 设计修改
//感谢正点原子的例程

#include "usart.h"   
#include "gpio.h"  
#include "usbd_audio_core.h"
#include "usbd_audio_play.h"
#include "usbd_usr.h"
#include "usb_conf.h" 	

USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 bDeviceState;		//USB连接 情况

int main(void)
{        
	u8 Divece_STA=0XFF;
	
	Board_Init();					//初始化LED 
	SEL_NONE;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	uart_init(115200);		//初始化串口波特率为115200

	LED4=1;
	APWR_EN;
	
	printf("USB waiting... \n");//提示正在建立连接 	
	    

#ifdef USE_USB_OTG_FS  		
 	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&AUDIO_cb,&USR_cb); 
#endif
#ifdef USE_USB_OTG_HS  	
 	USBD_Init(&USB_OTG_dev,USB_OTG_HS_CORE_ID,&USR_desc,&AUDIO_cb,&USR_cb);    
#endif
	USB_PULLUP_EN;//打开隔离器上拉电阻
	
	
	EVAL_AUDIO_Init();
	
	while(1)
	{  
		if(Divece_STA!=bDeviceState)//状态改变了
		{
			if(bDeviceState==1)
			{printf("USB Connected.\n");//提示USB连接已经建立
			LED2=1;
			}
			else
			{printf("USB DisConnected.\n");//提示USB连接失败
			LED2=0;
			}
			Divece_STA=bDeviceState;
		}

		if(alt_setting_now!=0)
		{
			if (overrun_counter| underrun_counter)
			{LED4=0;LED3=1;}
		}
		else
		{
			LED4=1;LED3=0;overrun_counter=0;underrun_counter=0;
		}

	} 
}






