#include "wkup.h"
#include "led.h"
#include "delay.h"	
#include "usart.h"		
#include "beep.h"

//系统进入待机模式
void Sys_Enter_Standby(void)
{		
  while(WKUP_KD);			//等待WK_UP按键松开(在有RTC中断时,必须等WK_UP松开再进入待机)
 
	RCC_AHB1PeriphResetCmd(0X04FF,ENABLE);//复位所有IO口
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟		 
	 
	PWR_BackupAccessCmd(ENABLE);//后备区域访问使能

	//这里我们就直接关闭相关RTC中断
	RTC_ITConfig(RTC_IT_TS|RTC_IT_WUT|RTC_IT_ALRB|RTC_IT_ALRA,DISABLE);//关闭RTC相关中断，可能在RTC实验打开了。
	RTC_ClearITPendingBit(RTC_IT_TS|RTC_IT_WUT|RTC_IT_ALRB|RTC_IT_ALRA);//清楚RTC相关中断标志位。
	
	PWR_ClearFlag(PWR_FLAG_WU);//清除Wake-up 标志
	
	PWR_WakeUpPinCmd(ENABLE);//设置WKUP用于唤醒
	
	PWR_EnterSTANDBYMode();	//进入待机模式
	 
}

//中断,检测到PA0脚的一个上升沿.	  
//中断线0线上的中断检测
void EXTI0_IRQHandler(void)
{ 		    		    				     		    
  EXTI_ClearITPendingBit(EXTI_Line0); // 清除LINE10上的中断标志位
	if(WKUP_KD)//关机?
	{	
		LED0=1;
		Sys_Enter_Standby(); //进入待机模式 
	}
} 

//PA0 WKUP唤醒初始化
void WKUP_Init(void)
{	  
  GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //PA0
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化	

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);//PA0 连接到中断线0
	
 
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;//LINE0
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE0
  EXTI_Init(&EXTI_InitStructure);//配置
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//外部中断0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置NVIC
	
	LED0=0;
}
















