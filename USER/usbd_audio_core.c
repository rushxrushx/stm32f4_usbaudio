/* Includes ------------------------------------------------------------------*/

#include "usbd_audio_core.h"
#include "audio_out.h"

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
static void AUDIOCLASS_REQ(void *pdev, USB_SETUP_REQ *req);
static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length);


//#define bitdepth 32
#define bitdepth 24
#define bpf (bitdepth/8*2) //byte per frame  (8 for 32bits) 帧字节数
#define bpfs (bitdepth/8)  //sub frame size  (4 for 32bits) 单声道字节数

// 接收数据包临时缓冲区，相当于103的PMA
u32 IsocOutBuff [AUDIO_OUT_PKTSIZE / 4];   //以整数为单位，拒绝左右声道错乱
                                           //AUDIO_OUT_PKTSIZE u8 , 这里是u32 所以除以4
u32 overrun_counter=0;
u32 fb_success=0;
u32 fb_incomplt=0;
u32 rx_incomplt=0;
u8 fb_buf[4];
u8 alt_setting_now=0;
u8 audioIsMute=0;
u8 audioVol=100; 

/* Main Buffer for Audio Control Rrequests transfers and its relative variables */
uint8_t  AudioCtl[64];
uint8_t  AudioCtlReq = 0;
uint8_t  AudioCtlCS = 0;

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
  bpfs,                                 /* bSubFrameSize (3 Bytes per frame) */
  bitdepth,                             /* bBitResolution (24bits per sample) */ 
  4,                             		/* bSamFreqType 4 frequencys supported */ 
  SAMPLE_FREQ(44100),         			/* Audio sampling frequency coded on 3 bytes */
  SAMPLE_FREQ(48000),
  SAMPLE_FREQ(88200),
  SAMPLE_FREQ(96000),
  /* 20 byte*/
  
  /* Endpoint 0x01 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType */
  AUDIO_OUT_EP,                         /* bEndpointAddress 1 out endpoint*/
  0x05,        /* iso async */
//  LOBYTE(AUDIO_OUT_PKTSIZE),
//  HIBYTE(AUDIO_OUT_PKTSIZE),    /* wMaxPacketSize in Bytes  */
 LOBYTE(588),
  HIBYTE(588),    /* wMaxPacketSize in Bytes  */
  0x01,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  AUDIO_IN_EP,                                 /* bSynchAddress */
  /* 09 byte*/
  
  /* Endpoint - Audio Streaming Descriptor*/
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType */
  AUDIO_ENDPOINT_GENERAL,               /* bDescriptor */
  0x01,                                 /* bmAttributes */
									//D0: Sampling Frequency Control
  0x00,                                 /* bLockDelayUnits */
  0x00,                                 /* wLockDelay */
  0x00,
  /* 07 byte*/

  /* Endpoint 0x81 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType */
  AUDIO_IN_EP,                         /* bEndpointAddress */
  USB_ENDPOINT_TYPE_ISOCHRONOUS,        /* bmAttributes */
  03,    /* wMaxPacketSize 3 Bytes 10.10 */
  00,
  0x01,                                 /* bInterval */
  0x02,                                 /* bRefresh 16ms */
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
        AUDIOCLASS_REQ(pdev, req);
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
      USBD_CtlSendData (pdev,(uint8_t *)&usbd_audio_AltSet,1);
      break;
      
    case USB_REQ_SET_INTERFACE :		//switch alt settings
      if ((uint8_t)(req->wValue) < AUDIO_TOTAL_IF_NUM)
      {
        usbd_audio_AltSet = (uint8_t)(req->wValue);

			if (usbd_audio_AltSet == 1) {
				alt_setting_now=usbd_audio_AltSet;
				DCD_EP_Open(pdev, AUDIO_OUT_EP, AUDIO_OUT_PKTSIZE, USB_OTG_EP_ISOC);
				DCD_EP_Open(pdev, AUDIO_IN_EP, 3, USB_OTG_EP_ISOC);
				DCD_EP_Flush(pdev,AUDIO_IN_EP);
				DCD_EP_PrepareRx(pdev , AUDIO_OUT_EP , (uint8_t*)IsocOutBuff , AUDIO_OUT_PKTSIZE ); 
				fb(working_samplerate);
			}
			else	//zero bandwidth
			{
				alt_setting_now=0;
				DCD_EP_Close (pdev , AUDIO_OUT_EP);
				//DCD_EP_Flush(pdev,AUDIO_OUT_EP);
				DCD_EP_Close (pdev , AUDIO_IN_EP);
				//DCD_EP_Flush(pdev,AUDIO_IN_EP);
			}

      }
      else
      {
        /* Call the error management function (command will be STALL) */
        USBD_CtlError (pdev, req);
      }
      break;
    }
  }
  return USBD_OK;
}

//audio class req
static void AUDIOCLASS_REQ(void *pdev, USB_SETUP_REQ *req)
{ 
uint8_t req_type = 	req->bmRequest & 0x1f; /* Set the request type. See UAC Spec 1.0 - 5.2.1 Request Layout */
uint8_t cmd = 		req->bRequest;//AUDIO_REQ_SET_CUR;          /* Set the request value */
uint8_t len = 		(uint8_t)req->wLength;      /* data length */
uint8_t unit = 		HIBYTE(req->wIndex);       /* target unit */
uint8_t index = 	req->wIndex & 0xf;		/* endpoint number */
uint8_t CS = 	HIBYTE(req->wValue);         /* control selector (high byte) */
uint8_t CN = 	LOBYTE(req->wValue);         /* control number (low byte) */  

//request to audio tune control --> FeatureUnit 0x2    0200:2号FU,通道0
if (req_type == AUDIO_CONTROL_REQ  &&  unit == 0x2 ){
  
	if (CS == AUDIO_CONTROL_VOLUME && CN == 0) {
    switch (cmd) {
        case AUDIO_REQ_SET_CUR:
          if (len)
          {
            /* Prepare the reception of the buffer over EP0 */
            USBD_CtlPrepareRx (pdev, AudioCtl, len);
    
            /* Set the global variables indicating current request and its length 
            to the function usbd_audio_EP0_RxReady() which will process the request */
            AudioCtlReq = req_type;  
            AudioCtlCS  = CS;
          }
          break;  

        case AUDIO_REQ_SET_MAX:
          break;
        
        case AUDIO_REQ_SET_MIN:
          break;

        case AUDIO_REQ_SET_RES:
          break;	
	
        case AUDIO_REQ_GET_CUR:
          AudioCtl[0] = audioVol;
          break;

        case AUDIO_REQ_GET_MAX:
          AudioCtl[0] = 100;
          break;
        
        case AUDIO_REQ_GET_MIN:
          AudioCtl[0] = 0;
          break;
        
        case AUDIO_REQ_GET_RES:
          AudioCtl[0] = 1;
          break;
        
        default:
          USBD_CtlError (pdev, req);
          return;
		}	
	
	}
	else if (CS == AUDIO_CONTROL_MUTE && CN == 0) {
    switch (cmd) {
        case AUDIO_REQ_SET_CUR:
          if (len)
          {
            /* Prepare the reception of the buffer over EP0 */
            USBD_CtlPrepareRx (pdev, AudioCtl, len);
    
            /* Set the global variables indicating current request and its length 
            to the function usbd_audio_EP0_RxReady() which will process the request */
            AudioCtlReq = req_type;  
            AudioCtlCS  = CS;
          }
          break;
		  
        case AUDIO_REQ_GET_CUR:
          AudioCtl[0] = audioIsMute;
          break;
        
        default:
          USBD_CtlError (pdev, req);
          return;
		}
	}
	else{
		USBD_CtlError (pdev, req);
        return;
	}
	
	//audio class final, 如果需要回应get请求
	if (cmd & AUDIO_REQ_GET_MASK) {
		USBD_CtlSendData (pdev, AudioCtl, len);    
		}
	}
	//else,request to endpoint,that EP is AUDIO_OUT_EP     0001:端点，1号端点
	else if (req_type == AUDIO_STREAMING_REQ && index == AUDIO_OUT_EP) {
      /* Frequency Control */
		if (CS == AUDIO_STREAMING_REQ_FREQ_CTRL) {
		switch (cmd) {
			case AUDIO_REQ_SET_CUR:
				/* Prepare the reception of the buffer over EP0 */
				USBD_CtlPrepareRx (pdev, AudioCtl, len);
		
				/* Set the global variables indicating current request and its length 
				to the function usbd_audio_EP0_RxReady() which will process the request */
				AudioCtlReq = req_type;  
				AudioCtlCS  = CS;
				break;
				
			case AUDIO_REQ_GET_CUR:
				AudioCtl[0] = (uint8_t)(working_samplerate);
				AudioCtl[1] = (uint8_t)(working_samplerate >> 8);
				AudioCtl[2] = (uint8_t)(working_samplerate >> 16);
				USBD_CtlSendData (pdev,AudioCtl,3);
				break;
			
			default:
				USBD_CtlError (pdev, req);
				return;					
			
			}
		}
	}
	else
		USBD_CtlError (pdev, req);
	
}



//控制端点接收数据
static uint8_t  usbd_audio_EP0_RxReady (void  *pdev)
{
u32 tmpfreq=0;
  
	/* request to audio */
	if (AudioCtlReq == AUDIO_CONTROL_REQ){
		if (AudioCtlCS == AUDIO_CONTROL_VOLUME)
		{
			audioVol=AudioCtl[0];
		}
		else if (AudioCtlCS == AUDIO_CONTROL_MUTE)
		{
			audioIsMute=AudioCtl[0];
		}
	}	
		
	/* request to endpoint */  //采样频率控制
	else if (AudioCtlReq == AUDIO_STREAMING_REQ && AudioCtlCS == AUDIO_STREAMING_REQ_FREQ_CTRL ){

		tmpfreq=((AudioCtl[2]<<16)|(AudioCtl[1]<<8)|AudioCtl[0]);
	 
		EVAL_AUDIO_Stop();//停止播放，并清空缓存，防止显示红灯
		Play_ptr=0;
		Write_ptr=0;
		overrun_counter=0;
		underrun_counter=0;
		if(tmpfreq==44100) working_samplerate=44100;
		if(tmpfreq==96000) working_samplerate=96000;
		if(tmpfreq==88200) working_samplerate=88200;
		if(tmpfreq==48000) working_samplerate=48000;
		 
	}
	/* Reset the AudioCtlCmd variable to prevent re-entering this function */
	AudioCtlReq = 0;
	AudioCtlCS = 0; 
  
  return USBD_OK;
}

//输入已完成
static uint8_t  usbd_audio_DataIn (void *pdev, uint8_t epnum)
{
        if (epnum == (AUDIO_IN_EP&0x7f))
        {
				fb_success++;//fb packet successful in.
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
vu16 nextbuf;
vu16 data_remain;

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
	
	if (data_remain > i2s_BUFSIZE/3*2 ) {fb(working_samplerate-100);}//like 44000
	else if (data_remain > i2s_BUFSIZE/3*1 ) {fb(working_samplerate);}//like 44100
	else {fb(working_samplerate+100);}//like 44200


//if (((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame ==0) 

	//DCD_EP_Tx(pdev, AUDIO_IN_EP, fb_buf, 3);
	

    /* Toggle the frame index */  
    ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame = 
      (((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame)? 0:1;
      
    //设定OUT端点准备接受下一数据包
    DCD_EP_PrepareRx(pdev,AUDIO_OUT_EP,(uint8_t*)IsocOutBuff,AUDIO_OUT_PKTSIZE);      
	
  }
  return USBD_OK;
}

//帧首中断(SOF)
static uint8_t  usbd_audio_SOF (void *pdev)
{	
	if(alt_setting_now){
	DCD_EP_Tx(pdev, AUDIO_IN_EP, fb_buf, 3);
	}
  return USBD_OK;
}

//接收出错
static uint8_t  usbd_audio_OUT_Incplt (void  *pdev)
{
	rx_incomplt++;
  return USBD_OK;
}

//发送出错
static uint8_t  usbd_audio_IN_Incplt (void  *pdev)
{
	fb_incomplt++;
  return USBD_OK;
}

/******************************************************************************
     AUDIO Class requests management
******************************************************************************/



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


