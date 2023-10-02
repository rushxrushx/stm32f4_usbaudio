/**
  ******************************************************************************
  * @file    usbd_audio_core.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   header file for the usbd_audio_core.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#ifndef __USB_AUDIO_CORE_H_
#define __USB_AUDIO_CORE_H_

#include "usbd_ioreq.h"
#include "usbd_req.h"
#include "usbd_desc.h"


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup usbd_audio
  * @brief This file is the Header file for USBD_audio.c
  * @{
  */ 


/** @defgroup usbd_audio_Exported_Defines
  * @{
  */ 

/*
 * ATT: with 3/4 bytes and 24/32bit it does not work on Windows: correct shown but error when tried to play
 */

#ifdef I2S_24BIT
//works: 48 KHz, 24bit - but disable all Enhancements on Windows Sound - mode 1
#define	USBD_AUDIO_FREQ							48000		//bits per seconds
#define HALF_WORD_BYTES							  3			//3 half word (one channel)
#define SAMPLE_BITS								 24			//24 bit per channel
#else
//works: 48 KHz, 16bit - mode 0
#define	USBD_AUDIO_FREQ								96000		//96000 : bits per seconds
#define HALF_WORD_BYTES								  2			//2 half word (one channel)
#define SAMPLE_BITS									 16			//16 bit per channel
//but 96 KHz, 24bit does not work on Windows, 96 KHz, 16bit is OK
#endif

//works: 48 KHz, 24bit - but disable all Enhancements on Windows Sound - mode 1
#define	USBD_AUDIO_FREQ2							48000		//bits per seconds
#define HALF_WORD_BYTES2							  3			//3 half word (one channel)
#define SAMPLE_BITS2								 24			//24 bit per channel

/* AudioFreq * DataSize (HALF_WORD_BYTES bytes) * NumChannels (Stereo: 2) */
/* ATT: make sure that the AUDIO_OUT_PACKET uses the largest value ! */
#ifndef USE_USB_OTG_HS
#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ  * 2 * HALF_WORD_BYTES)  /1000))
#else
#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ  * 2 * HALF_WORD_BYTES)  /8000))
#endif

/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
#define OUT_PACKET_NUM                                 4		//4
/* Total size of the audio transfer buffer */
#define TOTAL_OUT_BUF_SIZE                           ((uint32_t)(AUDIO_OUT_PACKET * OUT_PACKET_NUM))

#define AUDIO_CONFIG_DESC_SIZE                        152
#define AUDIO_INTERFACE_DESC_SIZE                     9
#define USB_AUDIO_DESC_SIZ                            0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_IP_VERSION_02_00                        0x20
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02
#define AUDIO_STREAMING_ENCODER                       0x03

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06
#define AUDIO_CONTROL_CLOCK_SOURCE                    0x0A

/* Audio Function Category Codes */
#define AUDIO_DESKTOP_SPEAKER                         0x01
#define AUDIO_HOME_THEATER                            0x02
#define AUDIO_MICROPHONE                              0x03
#define AUDIO_HEADSET                                 0x04

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x01
#define AUDIO_CONTROL_VOLUME                          0x02

/* UAC 2.0 */
#define AUDIO_20_CLK_SOURCE_DESC_SIZE                 0x08
#define AUDIO_20_IT_DESC_SIZE                         0x11
#define AUDIO_20_OT_DESC_SIZE                         0x0C
#define AUDIO_20_STREAMING_INTERFACE_DESC_SIZE        0x10

#define CONTROL_BITMAP_NONE                           (0x00)
#define CONTROL_BITMAP_RO                             (0x01)
#define CONTROL_BITMAP_PROG                           (0x03)

#define AUDIO_20_CTL_MUTE(bmaControl)                 (bmaControl)
#define AUDIO_20_CTL_VOLUME(bmaControl)               (bmaControl<<2)

#define AUDIO_20_STANDARD_ENDPOINT_DESC_SIZE          0x07
#define AUDIO_20_STREAMING_ENDPOINT_DESC_SIZE         0x08

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_II                          0x02
#define AUDIO_FORMAT_TYPE_III                         0x03
#define AUDIO_FORMAT_TYPE_IV                          0x04

#define USB_ENDPOINT_TYPE_ISOCHRONOUS                 0x01
#define USB_ENDPOINT_TYPE_ASYNCHRONOUS				  0x05
#define USB_ENDPOINT_TYPE_ADAPTIVE                    0x09
#define USB_ENDPOINT_TYPE_SYNCHRONOUS                 0x0D
#define AUDIO_ENDPOINT_GENERAL                        0x01

#define ENTITY_ID(wIndex)                             ((uint8_t)((wIndex & 0xFF00) >>8))
#define INTERFACE_ID(wIndex)                          ((uint8_t)(wIndex & 0x00FF))
#define CS_ID(wValue)                                 ((uint8_t)((wValue & 0xFF00) >>8))
#define CN_ID(wValue)                                 ((uint8_t)(wValue & 0x00FF))

#define AUDIO_REQ_CLASS_TYPE_MASK                     0x0F
#define AUDIO_REQ_INTERFACE                           0x01
#define AUDIO_REQ_ENDPOINT                            0x02

#define AUDIO_REQ_GET_MASK                            0x80
#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_GET_MIN                             0x82
#define AUDIO_REQ_GET_MAX                             0x83
#define AUDIO_REQ_GET_RES                             0x84

#define AUDIO_REQ_SET_CUR                             0x01
#define AUDIO_REQ_SET_MIN                             0x02
#define AUDIO_REQ_SET_MAX                             0x03
#define AUDIO_REQ_SET_RES                             0x04

#define AUDIO_REQ_CUR                                 0x01
#define AUDIO_REQ_RANGE                               0x02
#define AUDIO_REQ_MEM                                 0x03

#define AUDIO_IT_ID                                   0x01
#define AUDIO_FU_ID                                   0x02
#define AUDIO_OT_ID                                   0x03
#define AUDIO_CLK_ID                                  0x04

/* Clock Source Control Selectors */
#define CS_CONTROL_UNDEFINED                          0x00
#define CS_SAM_FREQ_CONTROL                           0x01
#define CS_CLOCK_VALID_CONTROL                        0x02

/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
typedef struct _Audio_Fops
{
    uint8_t  (*Init)         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
    uint8_t  (*DeInit)       (uint32_t options);
    uint8_t  (*AudioCmd)     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
    uint8_t  (*VolumeCtl)    (uint8_t vol);
    uint8_t  (*MuteCtl)      (uint8_t cmd);
    uint8_t  (*PeriodicTC)   (uint8_t cmd);
    uint8_t  (*GetState)     (void);
}AUDIO_FOPS_TypeDef;
/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

#ifndef USE_USB_OTG_HS
#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * 2 * HALF_WORD_BYTES)/1000) & 0xFF), \
                                       (uint8_t)((((frq * 2 * HALF_WORD_BYTES)/1000) >> 8) & 0xFF)
#else
#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * 2 * HALF_WORD_BYTES)/8000) & 0xFF), \
                                       (uint8_t)((((frq * 2 * HALF_WORD_BYTES)/8000) >> 8) & 0xFF)
#endif
#define AUDIO_OUT_PKTSIZE 200

#define SAMPLE_FREQ_NUM(num)           (uint8_t)(num), (uint8_t)((num >> 8))
#define SAMPLE_FREQ(frq)               (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))
#define SAMPLE_FREQ_4B(frq)            (uint8_t)(frq), (uint8_t)((frq >> 8)), \
                                       (uint8_t)((frq >> 16)), (uint8_t)((frq >> 24))
/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_Class_cb_TypeDef  AUDIO_cb;

/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */

void usbd_ConfigureAudio(int mode);

/**
  * @}
  */ 

#endif  // __USB_AUDIO_CORE_H_
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
