#include "gpio.h" 


//////////////////
//401白色小板
//////////////////

#if 0
void Board_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOx时钟
  
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//不拉

//led:PA1-4

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

// PB8 VEE+DAC电源
// PB9 模拟前端电源
// PB13 隔离器上拉控制


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_13;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}
#endif

//////////////////
/////mini1794 L1
//////////////////
#if 1
void Board_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOx时钟
  
  
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

//reset DAC
  PAout(3)=0;

//enable 5v and VEE power  
  PAout(1)=1;

}
#endif



