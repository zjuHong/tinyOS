#ifndef _OY_OS_H
#define _OY_OS_H
#include<stdbool.h>

/************************************    宏    **************************************/
#define OS_EXCEPT_STK_SIZE 1024
#define MAX_TASK_SIZE	 	32
#define OS_TICKS_PER_SEC	200

#define OS_ENTER_CRITICAL() __disable_irq()
#define OS_EXIT_CRITICAL() __enable_irq()
#define OSIntEnter() __disable_irq()
#define OSIntExit() __enable_irq()
#define OS_CRITICAL_METHOD

#define IDLE_TASK_PRIO      		MAX_TASK_SIZE - 1 //开始任务的优先级设置为最低
#define IDLE_STK_SIZE  				128
/************************************************************************************/


/***********************************    结构    *************************************/
typedef struct os_tcb
{
	unsigned int *StkPtr;
	unsigned int Dly;
}TCB;
/************************************************************************************/


/***********************************    变量    *************************************/
extern bool OSRunning;
extern TCB TCB_Task[MAX_TASK_SIZE];
extern TCB *p_TCB_Cur;//当前任务TCB
extern TCB *p_TCBHighRdy;//最高优先级TCB
extern unsigned int CPU_ExceptStk[OS_EXCEPT_STK_SIZE];//主堆栈
extern unsigned int *CPU_ExceptStkBase;//指向数组最后一个元素
/************************************************************************************/


/***********************************    函数    *************************************/
void Task_Create(void (*task)(void), unsigned int *stk, unsigned int prio);
void Task_End(void);
void OSCtxSw(void);
void OSStartHighRdy(void);
void OSTimeDly(unsigned int ticks);
void OSTimeTick(void);
void OS_Init(void);
/************************************************************************************/
#endif

