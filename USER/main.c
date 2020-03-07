#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	
#include "myOS.h"
#include "mmc_sd.h"			   
#include "File.h"
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				128
//���������ջ�ռ�	
unsigned int START_TASK_STK[START_STK_SIZE];
//�������ӿ�
void start_task(void);	
 			   
//LED1����
//�����������ȼ�
#define LED1_TASK_PRIO       			7 
//���������ջ��С
#define LED1_STK_SIZE  		    		256
//���������ջ�ռ�	
unsigned int LED1_TASK_STK[LED1_STK_SIZE];
//�������ӿ�
void led1_task(void);


//LED2����
//�����������ȼ�
#define LED2_TASK_PRIO       			6 
//���������ջ��С
#define LED2_STK_SIZE  					256
//���������ջ�ռ�	
unsigned int LED2_TASK_STK[LED2_STK_SIZE];
//�������ӿ�
void led2_task(void);

void Task_Init(void)
{
	OS_ENTER_CRITICAL();
	Task_Create(led1_task, &LED1_TASK_STK[LED1_STK_SIZE - 1], LED1_TASK_PRIO);//������ʼ����
 	Task_Create(led2_task, &LED2_TASK_STK[LED2_STK_SIZE - 1], LED2_TASK_PRIO);//������ʼ����
	OS_EXIT_CRITICAL();
}

int main(void)
{
	delay_init();	     //��ʱ��ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);
	LED_Init();		  	 //��ʼ����LED���ӵ�Ӳ���ӿ�
	SD_SPI_Init();
	initFileSystem();
	Task_Init();
	OS_Init();
	return 0;
}

//LED1����
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

//LED2����
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


