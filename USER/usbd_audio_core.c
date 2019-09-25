/**
  ******************************************************************************
  * @file    usbd_audio_core.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB Audio Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as Audio Streaming Device
  *           - Audio Streaming data transfer
  *           - AudioControl requests management
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                Audio Class Driver Description
  *          =================================================================== 
  *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
  *           Audio Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Standard AC Interface Descriptor management
  *             - 1 Audio Streaming Interface (with single channel, PCM, Stereo mode)
  *             - 1 Audio Streaming Endpoint
  *             - 1 Audio Terminal Input (1 channel)
  *             - Audio Class-Specific AC Interfaces
  *             - Audio Class-Specific AS Interfaces
  *             - AudioControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
  *             - Audio Feature Unit (limited to Mute control)
  *             - Audio Synchronization type: Asynchronous
  *             - Single fixed audio sampling rate (configurable in usbd_conf.h file)
  *          
  *           @note
  *            The Audio Class 1.0 is based on USB Specification 1.0 and thus supports only
  *            Low and Full speed modes and does not allow High Speed transfers.
  *            Please refer to "USB Device Class Definition for Audio Devices V1.0 Mar 18, 98"
  *            for more details.
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - AudioControl Endpoint management
  *             - AudioControl requsests other than SET_CUR and GET_CUR
  *             - Abstraction layer for AudioControl requests (only Mute functionality is managed)
  *             - Audio Synchronization type: Adaptive
  *             - Audio Compression modules and interfaces
  *             - MIDI interfaces and modules
  *             - Mixer/Selector/Processing/Extension Units (Feature unit is limited to Mute control)
  *             - Any other application-specific modules
  *             - Multiple and Variable audio sampling rates
  *             - Out Streaming Endpoint/Interface (microphone)
  *      
  *  @endverbatim
  *                                  
  */ 

/* Includes ------------------------------------------------------------------*/

#include "usbd_audio_core.h"
#include "usbd_audio_play.h"

/*********************************************
   AUDIO Device library callbacks
 *********************************************/
static uint8_t  usbd_audio_Init       (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_audio_DeInit     (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_audio_Setup      (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_audio_EP0_RxReady(void *pdev);
static uint8_t  usbd_audio_DataIn     (void *pdev, uint8_t epnum);  //发出数据
static uint8_t  usbd_audio_DataOut    (void *pdev, uint8_t epnum);  //收到数据
static uint8_t  usbd_audio_SOF        (void *pdev);
static uint8_t  usbd_audio_IN_Incplt (void  *pdev);
static uint8_t  usbd_audio_OUT_Incplt (void  *pdev);  //收数据未成功

/*********************************************
   AUDIO Requests management functions
 *********************************************/
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req);
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req);
static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length);


//#define bitdepth 32
#define bitdepth 24
#define bpf (bitdepth/8*2) //byte per frame  (8 for 32bits) 帧字节数
#define bpfs (bitdepth/8)  //sub frame size  (4 for 32bits) 单声道字节数

// 接收数据包临时缓冲区，相当于103的PMA
u32 IsocOutBuff [AUDIO_OUT_PKTSIZE / 4];   //以整数为单位，拒绝左右声道错乱
                                           //AUDIO_OUT_PKTSIZE u8 , 这里是u32 所以除以4
u8 fb_buf[64];
u8 fb_exist=0;
u8 alt_setting_now=0;
u32 overrun_counter=0;

/* Main Buffer for Audio Control Rrequests transfers and its relative variables */
uint8_t  AudioCtl[64];
uint8_t  AudioCtlCmd = 0;
uint32_t AudioCtlLen = 0;
uint8_t  AudioCtlIndex = 0;


#include "gpio.h"

static __IO uint32_t  usbd_audio_AltSet = 0;
static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE];

/* AUDIO interface class callbacks structure */
USBD_Class_cb_TypeDef  AUDIO_cb = 
{
  usbd_audio_Init,
  usbd_audio_DeInit,
  usbd_audio_Setup,
  NULL, 		/* EP0 已发出 */
  usbd_audio_EP0_RxReady,
  usbd_audio_DataIn,
  usbd_audio_DataOut,
  usbd_audio_SOF,
  usbd_audio_IN_Incplt,               /* ISO IN不成功 */
  usbd_audio_OUT_Incplt,    /* ISO OUT不成功 */
  USBD_audio_GetCfgDesc,
#ifdef USB_OTG_HS_CORE  
  USBD_audio_GetCfgDesc, /* use same config as per FS */
#endif    
};

/* USB AUDIO device Configuration Descriptor */
static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE] =
{
  /* Configuration 1 */
  0x09,                                 /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,    /* bDescriptorType */
  LOBYTE(AUDIO_CONFIG_DESC_SIZE),       /* wTotalLength */
  HIBYTE(AUDIO_CONFIG_DESC_SIZE),      
  0x02,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0x40,                                 /* bmAttributes  self Powred*/
  0x32,                                 /* bMaxPower = 100 mA*/
  /* 09 byte*/
  
  /* USB Speaker Standard interface descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/
  
  /* USB Speaker Class-specific AC Interface Descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
  0x00,          /* 1.00 */             /* bcdADC */
  0x01,
  0x27,                                 /* wTotalLength = 39*/
  0x00,
  0x01,                                 /* bInCollection */
  0x01,                                 /* baInterfaceNr */
  /* 09 byte*/
  
  /* USB Speaker Input Terminal Descriptor */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype */
  0x01,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
  0x01,
  0x00,                                 /* bAssocTerminal */
  0x02,                                 /* bNrChannels */
  0x03,                                 /* wChannelConfig */
  0x00,
  0x00,                                 /* iChannelNames */
  0x00,                                 /* iTerminal */
  /* 12 byte*/
  
  /* USB Speaker Audio Feature Unit Descriptor */
  0x09,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
  AUDIO_OUT_STREAMING_CTRL,             /* bUnitID */
  0x01,                                 /* bSourceID */
  0x01,                                 /* bControlSize */
  AUDIO_CONTROL_MUTE,                   /* bmaControls(0) */
  0x00,               					/* bmaControls(1) */
  0x00,                                 /* iTerminal */
  /* 10 byte*/
  
  /*USB Speaker Output Terminal Descriptor */
  0x09,      /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype */
  0x03,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType  0x0301*/
  0x03,
  0x00,                                 /* bAssocTerminal */
  0x02,                                 /* bSourceID */
  0x00,                                 /* iTerminal */
  /* 09 byte*/
  
  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
  /* Interface 1, Alternate Setting 0                                             */
  AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/
  
  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
  /* Interface 1, Alternate Setting 1                                           */
  AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x01,                                 /* bAlternateSetting */
  0x02,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/
  
  /* USB Speaker Audio Streaming Interface Descriptor */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype */
  0x01,                                 /* bTerminalLink */
  0x01,                                 /* bDelay */
  0x01,                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
  0x00,
  /* 07 byte*/
  
  /* USB Speaker Audio Type I Format Interface Descriptor */
  20,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype */
  AUDIO_FORMAT_TYPE_I,                	/* bFormatType */ 
  0x02,                                 /* bNrChannels */
  bpfs,                                 /* bSubFrameSize (4 Bytes per frame) */
  bitdepth,                                   /* bBitResolution (32bits per sample) */ 
  4,                                 /* bSamFreqType 2 frequencys supported */ 
  SAMPLE_FREQ(44100),         			/* Audio sampling frequency coded on 3 bytes */
  SAMPLE_FREQ(48000),
  SAMPLE_FREQ(88200),
  SAMPLE_FREQ(96000),
  /* 20 byte*/
  
  /* Endpoint 1 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType */
  AUDIO_OUT_EP,                         /* bEndpointAddress 1 out endpoint*/
  0x05,        /* iso async */
  LOBYTE(AUDIO_OUT_PKTSIZE),
  HIBYTE(AUDIO_OUT_PKTSIZE),    /* wMaxPacketSize in Bytes  */
  0x01,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  AUDIO_IN_EP,                                 /* bSynchAddress */
  /* 09 byte*/
  
  /* Endpoint - Audio Streaming Descriptor*/
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType */
  AUDIO_ENDPOINT_GENERAL,               /* bDescriptor */
  0x00,                                 /* bmAttributes */
  0x00,                                 /* bLockDelayUnits */
  0x00,                                 /* wLockDelay */
  0x00,
  /* 07 byte*/

  /* Endpoint 0x82 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType */
  AUDIO_IN_EP,                         /* bEndpointAddress 0x82*/
  USB_ENDPOINT_TYPE_ISOCHRONOUS,        /* bmAttributes */
  03,    /* wMaxPacketSize 3 Bytes 10.10 */
  00,
  0x01,                                 /* bInterval */
  0x01,                                 /* bRefresh 2ms */
  0x00,                                 /* bSynchAddress */
  /* 09 byte*/

} ;



//计算10.10数据小程序
void fb(u32 freq)
{
u32 fbvalue, nInt,nFrac;

	// example freq:44100
	// 10.10fb=  44 << 14 + 100 << 4
	
	nInt=freq/1000;
	nFrac=(freq - nInt*1000);
		
	fbvalue=(nInt <<14) | (nFrac<<4);	
	
	fb_buf[0]=fbvalue & 0xff;
	fb_buf[1]=(fbvalue>>8) & 0xff;
	fb_buf[2]=(fbvalue>>16) & 0xff;

//{fb_buf[0]=0x66;fb_buf[1]=0x06;fb_buf[2]=0x0b;}
}


/**
* @brief  usbd_audio_Init
*         Initilaizes the AUDIO interface.
* @param  pdev: device instance
* @param  cfgidx: Configuration index
* @retval status
*/
static uint8_t  usbd_audio_Init (void  *pdev, uint8_t cfgidx)
{  
  return USBD_OK;
}

/**
* @brief  usbd_audio_Init
*         DeInitializes the AUDIO layer.
* @param  pdev: device instance
* @param  cfgidx: Configuration index
* @retval status
*/
static uint8_t  usbd_audio_DeInit (void  *pdev, uint8_t cfgidx)
{ 

  
//  if (EVAL_AUDIO_DeInit() != USBD_OK)
//  {
//    return USBD_FAIL;
//  }
  
  return USBD_OK;
}

/**
  * @brief  usbd_audio_Setup
  *         Handles the Audio control request parsing.
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  usbd_audio_Setup (void  *pdev, 
                                  USB_SETUP_REQ *req)
{
  uint16_t len=USB_AUDIO_DESC_SIZ;
  uint8_t  *pbuf=usbd_audio_CfgDesc + 18;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    /* AUDIO Class Requests -------------------------------*/
  case USB_REQ_TYPE_CLASS :    
    switch (req->bRequest)
    {
    case AUDIO_REQ_GET_CUR:
      AUDIO_Req_GetCurrent(pdev, req);
      break;
      
    case AUDIO_REQ_SET_CUR:
      AUDIO_Req_SetCurrent(pdev, req);   
      break;

    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL;
    }
    break;
    
    /* Standard Requests -------------------------------*/
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
      {
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pbuf = usbd_audio_CfgDesc ;   
#else
        pbuf = usbd_audio_CfgDesc + 18;
#endif 
        len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&usbd_audio_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :                         // 切换alt set，这里还要补充播放/停止控制语句或者采样率控制语句
      if ((uint8_t)(req->wValue) < AUDIO_TOTAL_IF_NUM)
      {
        usbd_audio_AltSet = (uint8_t)(req->wValue);

			if (usbd_audio_AltSet == 1) {
				alt_setting_now=usbd_audio_AltSet;
				DCD_EP_Open(pdev, AUDIO_OUT_EP, AUDIO_OUT_PKTSIZE, USB_OTG_EP_ISOC);
				DCD_EP_Open(pdev, AUDIO_IN_EP, 3, USB_OTG_EP_ISOC);
				DCD_EP_Flush(pdev,AUDIO_IN_EP);
				DCD_EP_PrepareRx(pdev , AUDIO_OUT_EP , (uint8_t*)IsocOutBuff , AUDIO_OUT_PKTSIZE ); 
				fb_exist=0;
				fb(working_samplerate);
			}
			else	//zero bandwidth
			{
				alt_setting_now=0;
				DCD_EP_Close (pdev , AUDIO_IN_EP);
				DCD_EP_Close (pdev , AUDIO_OUT_EP);
			}



      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;
    }
  }
  return USBD_OK;
}


//控制端点接收数据
static uint8_t  usbd_audio_EP0_RxReady (void  *pdev)
{ 
u32 tmpfreq=0;
  /* Check if an AudioControl request has been issued */
  if (AudioCtlCmd == AUDIO_REQ_SET_CUR)
  {/* In this driver, to simplify code, only SET_CUR request is managed */
    /* Check for which addressed unit the AudioControl request has been issued */

	 if ((AudioCtlIndex& 0xf) == AUDIO_OUT_EP) {    /* request to endpoint */
	 
	//采样频率控制	
	 tmpfreq=((AudioCtl[2]<<16)|(AudioCtl[1]<<8)|AudioCtl[0]);
	 
	EVAL_AUDIO_Stop();//停止播放，并清空缓存，防止显示红灯
	
	Play_ptr=0;
	Write_ptr=0;
	 
	 if (tmpfreq==44100) working_samplerate=44100;
	 if (tmpfreq==96000) working_samplerate=96000;
	 if (tmpfreq==88200) working_samplerate=88200;
	 if (tmpfreq==48000) working_samplerate=48000;
	 
     }  
	
	else {			/* request to interface */
		AudioCtlIndex = HIBYTE(AudioCtlIndex);
		if (AudioCtlIndex == AUDIO_OUT_STREAMING_CTRL)
		{
		  /* Call the audio interface mute function */
	 //因为这个驱动只支持静音控制，这里st直接就调用静音程序，不做判断
	//      AUDIO_OUT_fops.MuteCtl(AudioCtl[0]);
		}
	
	}
	/* Reset the AudioCtlCmd variable to prevent re-entering this function */
	  AudioCtlCmd = 0;
	  AudioCtlLen = 0;
          
  } 
  
  return USBD_OK;
}

//输入已完成
static uint8_t  usbd_audio_DataIn (void *pdev, uint8_t epnum)
{


        if (epnum == (AUDIO_IN_EP&0x7f))
        {
//                fb_exist=0;
        };


return USBD_OK;

}

//32bit音频,需要处理一下
u32 switchHL(u32 input)
{
	u32 tmpchrH;
	u32 tmpchrL;

	tmpchrL=(input>>16) & 0xffff;

	tmpchrH =(input& 0xffff) <<16;

	tmpchrH = tmpchrH + tmpchrL;

	return tmpchrH; 
}
//24bit音频,需要处理一下
u32 switchI24O32(u8 *Ibuffer,u16 offset)
{
	u8 Hbyte;
	u8 Mbyte;
	u8 Lbyte;

	u32 tmpchr=0x00000000;
	
	Lbyte=Ibuffer[offset+0];
	Mbyte=Ibuffer[offset+1];
	Hbyte=Ibuffer[offset+2];
	
	tmpchr= (Hbyte<<8) | (Mbyte<<0) |  (Lbyte<<24);
	tmpchr=tmpchr & 0xff00ffff;

	return tmpchr; 
}


//输出已完成
u16 rx_bytes_count=0;
u16 rx_frames_count;
static uint8_t  usbd_audio_DataOut (void *pdev, uint8_t epnum)
{ 

u16 i;
u16 nextbuf;
u16 data_remain;

    
  if (epnum == AUDIO_OUT_EP)
  { 
	rx_bytes_count	=	USBD_GetRxCount(pdev ,AUDIO_OUT_EP);//实际收到了windows给的多少bytes

	if(rx_bytes_count % bpf != 0)	/// 左右声道数据量不配套？！ 
		{
         //砸了电脑！
		}

    rx_frames_count	=	rx_bytes_count / bpf; // 32bit*2=8byte,,24bit*2=6byte,,16bit*2=4byte

	//PMA到环形缓存拷贝程序
	for (i=0; i<rx_frames_count; i++ )   //CPU屌啊，一个字一个字的拷
	{
	//计算即将写入的位置
		if (Write_ptr < i2s_BUFSIZE ) nextbuf=Write_ptr+2 ;
		else nextbuf=0;
	   
		if (nextbuf != Play_ptr ) //如果没有追尾，把这一帧数据写入到环形缓存
		{
			Write_ptr = nextbuf;
			
		#if bitdepth==32	
			i2s_buf[Write_ptr] = switchHL( IsocOutBuff[i*2] );//32bit left
			i2s_buf[Write_ptr+1] = switchHL( IsocOutBuff[i*2+1] );//32bit right
		#else	
			i2s_buf[Write_ptr]   = switchI24O32( (uint8_t*)IsocOutBuff ,i*6 );//24bit to 32bit left
			i2s_buf[Write_ptr+1] = switchI24O32( (uint8_t*)IsocOutBuff,(i*6+3) );//24bit to 32bit right
		#endif
		}
		else //撞上了
		{
			overrun_counter++;//立OVER-RUN FLAG

			break;//吃吐了，剩下数据全部不要了
		}
	}
   

      
	//计算环形数据容量
	if (Write_ptr > Play_ptr) data_remain = Write_ptr - Play_ptr;

	else data_remain = i2s_BUFSIZE - Play_ptr + Write_ptr;

//如果目前是停止状态，有一半容量开启播放
	if ( (audiostatus==0) && (data_remain > i2s_BUFSIZE/2 ) ) EVAL_AUDIO_Play();


  }

	if (data_remain > i2s_BUFSIZE/3*2 ) {fb(working_samplerate-100);}//like 44000

	else if (data_remain > i2s_BUFSIZE/3*1 ) {fb(working_samplerate);}//like 44100

	else {fb(working_samplerate+100);}//like 44200

	DCD_EP_Tx(pdev, AUDIO_IN_EP, &fb_buf[0], 3);
    
    /* Toggle the frame index */  
    ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame = 
      (((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame)? 0:1;
      
    //设定OUT端点准备接受下一数据包
    DCD_EP_PrepareRx(pdev,AUDIO_OUT_EP,(uint8_t*)IsocOutBuff,AUDIO_OUT_PKTSIZE);	

  
  return USBD_OK;
}

//帧首中断(SOF)
static uint8_t  usbd_audio_SOF (void *pdev)
{	
  return USBD_OK;
}

//接收出错
static uint8_t  usbd_audio_OUT_Incplt (void  *pdev)
{
  return USBD_OK;
}

//发送出错
static uint8_t  usbd_audio_IN_Incplt (void  *pdev)
{
  return USBD_OK;
}

/******************************************************************************
     AUDIO Class requests management
******************************************************************************/
/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req)
{  
  /* Send the current mute state */
  USBD_CtlSendData (pdev, 
                    AudioCtl,
                    req->wLength);
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req)
{ 
  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx (pdev, 
                       AudioCtl,
                       req->wLength);
    
    /* Set the global variables indicating current request and its length 
    to the function usbd_audio_EP0_RxReady() which will process the request */
    AudioCtlCmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    AudioCtlLen = req->wLength;          /* Set the request data length */
    AudioCtlIndex = req->wIndex;  /* Set the request target unit */
  }
  
  
        
}

/**
  * @brief  USBD_audio_GetCfgDesc 
  *         Returns configuration descriptor.
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_audio_CfgDesc);
  return usbd_audio_CfgDesc;
}


