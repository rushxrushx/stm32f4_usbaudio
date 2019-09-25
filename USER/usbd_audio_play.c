//////////////////////////////////////////////////////////////////////////////////	 

//USB声卡播放程序   
//By Rush,personal use only! 仅供学习，禁止商用

////////////////////////////////////////////////////////////////////////////////// 	
#include "usbd_audio_play.h"
#include "usbd_audio_core.h"
#include "i2s.h"
#include "gpio.h"
 
u8 volume=0;								//当前音量  
u32 working_samplerate=44100;				//当前采样频率
u8 audiostatus=0;							//bit0:0,暂停播放;1,继续播放   
u16 Play_ptr=0;							//即将播放的音频帧缓冲编号
u16 Write_ptr=0;							//当前保存到的音频缓冲编号 
u32 underrun_counter=0;
u32 const i2s_BUFSIZE=4000;								
u32 i2s_buf[i2s_BUFSIZE+1]; 					//音频缓冲

#ifndef USE_USB_OTG_HS 
//full speed usb,401 --->> spi2 

/* I2S DMA Stream definitions */
	#define AUDIO_I2S_DMA_CLOCK     RCC_AHB1Periph_DMA1
	#define AUDIO_I2S_DMA_STREAM    DMA1_Stream4
	#define AUDIO_I2S_DMA_CHANNEL   DMA_Channel_0
	#define AUDIO_I2S_DMA_IRQ       DMA1_Stream4_IRQn
	#define AUDIO_I2S_DMA_IT_TC     DMA_IT_TCIF4
	#define AUDIO_I2S_SPI_ADDR 		(u32)&SPI2->DR
	#define AUDIO_I2S_SPI_SPIX		RCC_APB1Periph_SPI2
	
#else
//HS usb ,205 ,use spi3

/* I2S DMA Stream definitions */
	#define AUDIO_I2S_DMA_CLOCK     RCC_AHB1Periph_DMA1
	#define AUDIO_I2S_DMA_STREAM    DMA1_Stream7
	#define AUDIO_I2S_DMA_CHANNEL   DMA_Channel_0
	#define AUDIO_I2S_DMA_IRQ       DMA1_Stream7_IRQn
	#define AUDIO_I2S_DMA_IT_TC     DMA_IT_TCIF7
	#define AUDIO_I2S_SPI_ADDR 		(u32)&SPI3->DR
	#define AUDIO_I2S_SPI_SPIX		RCC_APB1Periph_SPI3
	
#endif		


//I2S2 TX DMA NVIC配置
void I2S2_DMA_Init1()
{  
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = AUDIO_I2S_DMA_IRQ; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置

} 	

//I2S2 TX DMA配置
void I2S2_DMA_Reconf()
{  
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK,ENABLE);//DMA1时钟使能 
	
	DMA_DeInit(AUDIO_I2S_DMA_STREAM);
	while (DMA_GetCmdStatus(AUDIO_I2S_DMA_STREAM) != DISABLE){}//等待DMA1_Stream1可配置 

  DMA_InitStructure.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;  //通道0 SPI3_TX通道 
  DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_I2S_SPI_ADDR;//外设地址
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32) &(i2s_buf[Play_ptr]);//存储器地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//存储器到外设模式
  DMA_InitStructure.DMA_BufferSize = 2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据长度:16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;//存储器数据长度：32位 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 非循环模式 
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//高优先级
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable; //使用FIFO模式        
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//突发关闭
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//突发关闭
  DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);//初始化DMA Stream
 
  DMA_ITConfig(AUDIO_I2S_DMA_STREAM,DMA_IT_TC,ENABLE);//开启传输完成中断

}  


    

//DMA开始播放
void EVAL_AUDIO_Play(void)
{

	I2S2_DMA_Reconf();
	//更新 DMA 读取入口地址
	DMA_MemoryTargetConfig(AUDIO_I2S_DMA_STREAM, (u32) &(i2s_buf[Play_ptr]) ,DMA_Memory_0);
	//设置好数据量
	DMA_SetCurrDataCounter(AUDIO_I2S_DMA_STREAM, 4);//内部精度32位，不受usb控制
	//重启DMA
	DMA_Cmd(AUDIO_I2S_DMA_STREAM,ENABLE);
	I2S2_Reconf(working_samplerate);
	DAC_EN;
	LEDON;
	audiostatus=1;
}
 
 
//DMA停止播放
void EVAL_AUDIO_Stop(void)
{
	RCC_APB1PeriphClockCmd(AUDIO_I2S_SPI_SPIX, DISABLE);//停止SPI2时钟，无需等待
	RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK,DISABLE);//停止DMA1时钟，无需等待
	DAC_DIS;
	LEDOFF;
	audiostatus=0;
}


//音频数据I2S DMA传输回调函数
void audio_i2s_dma_callback(void) 
{ 
u16 next_Playptr;

	DMA_Cmd(AUDIO_I2S_DMA_STREAM,DISABLE);//关闭DMA才能修改

	//算下一个数据帧
	if (Play_ptr < i2s_BUFSIZE ) next_Playptr = Play_ptr + 2;

	else  next_Playptr = 0;          //循环回头部

    
	if( next_Playptr == Write_ptr)	//撞尾了
	{ 
		EVAL_AUDIO_Stop();
		underrun_counter++;
		return;
	}else
	{
		Play_ptr = next_Playptr;//可以开开心心去下一个数据帧
	}

	//更新 DMA 读取入口地址
	DMA_MemoryTargetConfig(AUDIO_I2S_DMA_STREAM, (u32) &(i2s_buf[Play_ptr]) ,DMA_Memory_0);

	//设置好数据量
	DMA_SetCurrDataCounter(AUDIO_I2S_DMA_STREAM, 4);//内部精度32位，不受usb控制

	//重启DMA
	DMA_Cmd(AUDIO_I2S_DMA_STREAM,ENABLE);

}

//配置音频接口
//AudioFreq:音频采样率
uint32_t EVAL_AUDIO_Init()
{ 
	I2s_GPInit();
	I2S2_DMA_Init1();
	EVAL_AUDIO_Stop();
	return 0; 
}

#ifndef USE_USB_OTG_HS 
//full speed usb,401 --->> spi2 

//DMA1_Stream4中断服务函数
void DMA1_Stream4_IRQHandler(void)
{      
	if(DMA_GetITStatus(AUDIO_I2S_DMA_STREAM,AUDIO_I2S_DMA_IT_TC)==SET)////DMA1_Stream4,传输完成标志
	{ 
		DMA_ClearITPendingBit(AUDIO_I2S_DMA_STREAM,AUDIO_I2S_DMA_IT_TC);

		audio_i2s_dma_callback();
	}   											 
}

#else
//HS usb ,205 ,use spi3

//DMA1_Stream7中断服务函数
void DMA1_Stream7_IRQHandler(void)
{      
	if(DMA_GetITStatus(AUDIO_I2S_DMA_STREAM,AUDIO_I2S_DMA_IT_TC)==SET)////DMA1_Stream7,传输完成标志
	{ 
		DMA_ClearITPendingBit(AUDIO_I2S_DMA_STREAM,AUDIO_I2S_DMA_IT_TC);

		audio_i2s_dma_callback();
	}   											 
}
#endif
















