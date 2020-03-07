#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	
#include "myOS.h"
#include "mmc_sd.h"			   
#include "File.h"
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				128
//创建任务堆栈空间	
unsigned int START_TASK_STK[START_STK_SIZE];
//任务函数接口
void start_task(void);	
 			   
//LED1任务
//设置任务优先级
#define LED1_TASK_PRIO       			7 
//设置任务堆栈大小
#define LED1_STK_SIZE  		    		256
//创建任务堆栈空间	
unsigned int LED1_TASK_STK[LED1_STK_SIZE];
//任务函数接口
void led1_task(void);


//LED2任务
//设置任务优先级
#define LED2_TASK_PRIO       			6 
//设置任务堆栈大小
#define LED2_STK_SIZE  					256
//创建任务堆栈空间	
unsigned int LED2_TASK_STK[LED2_STK_SIZE];
//任务函数接口
void led2_task(void);

void Task_Init(void)
{
	OS_ENTER_CRITICAL();
	Task_Create(led1_task, &LED1_TASK_STK[LED1_STK_SIZE - 1], LED1_TASK_PRIO);//创建起始任务
 	Task_Create(led2_task, &LED2_TASK_STK[LED2_STK_SIZE - 1], LED2_TASK_PRIO);//创建起始任务
	OS_EXIT_CRITICAL();
}

int main(void)
{
	delay_init();	     //延时初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);
	LED_Init();		  	 //初始化与LED连接的硬件接口
	SD_SPI_Init();
	initFileSystem();
	Task_Init();
	OS_Init();
	return 0;
}

//LED1任务
void led1_task()
{	 	
	while(1)
	{
		LED0=0;
		printf("test1\n");
		LED0=1;
		delay_ms(500);
	};
}

//LED2任务
void led2_task()
{	  
	while(1)
	{
		LED0=0;
		printf("test2\n");
		LED0=1;
		delay_ms(1000);
	};
}


