#ifndef __STM324xG_USB_AUDIOCODEC_H
#define __STM324xG_USB_AUDIOCODEC_H

#include "stm32f4xx.h" 
						 
extern vu8 audiostatus;							//bit0:0,暂停播放;1,继续播放 
extern vu32 working_samplerate;  
extern vu32 Play_ptr;							//即将播放的音频帧缓冲编号
extern vu32 Write_ptr;							//当前保存到的音频缓冲编号 
extern u32 underrun_counter;
extern u32 const i2s_BUFSIZE;								
extern u32 i2s_buf[]; 					//音频缓冲数组 


/* Exported functions ------------------------------------------------------- */

uint32_t EVAL_AUDIO_Init(void);
uint32_t EVAL_AUDIO_DeInit(void);
uint32_t EVAL_AUDIO_Freq(uint32_t AudioFreq);
void EVAL_AUDIO_Play(void);
void EVAL_AUDIO_Stop(void);

#endif 






















