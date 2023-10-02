// L1/L2 HW
// BOARD FILE

#include "gpio.h" 
#include "spi1.h" 


void Board_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOx时钟
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//不拉

// PA1 5VLDO VEE_CUK
// PA2 6VLDO -6V LDO 
// PA3 DAC RST
// PA8 H 22mhz L 24MHZ
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3| GPIO_Pin_8;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

//led:PB3 4 5 6
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

//PC6:45MHZ,PC7:49MHZ
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  PCout(6)=0;
  PCout(7)=0;
  
  
	SPI1_Init();//////add 20210614 for ad1955

//reset DAC
  DAC_DIS;

//enable 5v and VEE power  
  APWR_STAGE1_EN;

}

