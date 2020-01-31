#include "myOS.h"

unsigned int OSRdyTbl;//就绪任务描述符
unsigned int OSPrioHighRdy;//当前就绪的最高优先级
unsigned int OSPrioCur;//当前优先级
unsigned int CPU_ExceptStk[OS_EXCEPT_STK_SIZE];//主堆栈
unsigned int IDLE_TASK_STK[IDLE_STK_SIZE];//空闲任务堆栈
unsigned int *CPU_ExceptStkBase;//指向数组最后一个元素
bool OSRunning;//OS是否启动

TCB TCB_Task[MAX_TASK_SIZE];//任务TCB
TCB *p_TCB_Cur;//当前任务TCB
TCB *p_TCBHighRdy;//最高优先级TCB

;/**************************************************************************************
;* 函数名称: OSSetPrioRdy
;*
;* 功能描述: 设置用户就绪表状态
;*
;* 参    数: prio
;*
;* 返 回 值: None
;***************************************************************************************/
void OSSetPrioRdy(unsigned int prio)
{
	if (prio == 0) return;//暂时不用值为0的优先级
	OSRdyTbl |= 0x01 << prio;
}

;/**************************************************************************************
;* 函数名称: OSDelPrioRdy
;*
;* 功能描述: 删除用户就绪表状态
;*
;* 参    数: prio
;*
;* 返 回 值: None
;***************************************************************************************/
void OSDelPrioRdy(unsigned int prio)
{
	OSRdyTbl &= ~(0x01 << prio);
}

;/**************************************************************************************
;* 函数名称: OSGetHighRdy
;*
;* 功能描述: 筛选优先级最高任务
;*
;* 参    数: None
;*
;* 返 回 值: None
;***************************************************************************************/
void OSGetHighRdy()
{
	unsigned int OSNextTaskPrio;
	for (OSNextTaskPrio = 0; (OSNextTaskPrio < MAX_TASK_SIZE) && (!(OSRdyTbl & (0x01 << OSNextTaskPrio))); OSNextTaskPrio++)
		;
	OSPrioHighRdy = OSNextTaskPrio;
}

;/**************************************************************************************
;* 函数名称: Task_End
;*
;* 功能描述: 结束一个任务
;*
;* 参    数: None
;*
;* 返 回 值: None
;***************************************************************************************/
void Task_End(void)
{
	while (1)
	{}
}

;/**************************************************************************************
;* 函数名称: Task_Create
;*
;* 功能描述: 新建一个任务
;*
;* 参    数: task stk prio
;*
;* 返 回 值: None
;***************************************************************************************/
void Task_Create(void (*task)(void), unsigned int *stk, unsigned int prio)
{
	unsigned int *p_stk;
	p_stk = stk;
	p_stk = (unsigned int *) ((unsigned int)(p_stk) & (0xFFFFFFF8u));
	*(--p_stk) = (unsigned int)0x01000000L;				  /* xPSR                                               */
	*(--p_stk) = (unsigned int)task;					  /* Entry Point                                        */
	*(--p_stk) = (unsigned int)Task_End;//0xFFFFFFFEuL;   /* R14                                                */
	*(--p_stk) = (unsigned int)0x12121212uL; 			  /* R12                                                */
	*(--p_stk) = (unsigned int)0x03030303uL; 			  /* R3                                                 */
	*(--p_stk) = (unsigned int)0x02020202uL; 			  /* R2                                                 */
	*(--p_stk) = (unsigned int)0x01010101uL; 			  /* R1                                                 */
	*(--p_stk) = (unsigned int)0x00000000u; 			  /* R0                                                 */
	
	*(--p_stk)  = (unsigned int)0x11111111uL;             /* R11                                                */
    *(--p_stk)  = (unsigned int)0x10101010uL;             /* R10                                                */
    *(--p_stk)  = (unsigned int)0x09090909uL;             /* R9                                                 */
    *(--p_stk)  = (unsigned int)0x08080808uL;             /* R8                                                 */
    *(--p_stk)  = (unsigned int)0x07070707uL;             /* R7                                                 */
    *(--p_stk)  = (unsigned int)0x06060606uL;             /* R6                                                 */
    *(--p_stk)  = (unsigned int)0x05050505uL;             /* R5                                                 */
    *(--p_stk)  = (unsigned int)0x04040404uL;             /* R4                                                 */
	
	TCB_Task[prio].StkPtr = p_stk;
	OSSetPrioRdy(prio);
}

;/**************************************************************************************
;* 函数名称: OS_Sched
;*
;* 功能描述: 进程调度
;*
;* 参    数: None
;*
;* 返 回 值: None
;***************************************************************************************/
void OS_Sched(void)
{
	OSGetHighRdy();
	if (OSPrioHighRdy != OSPrioCur)
	{
		p_TCBHighRdy = &TCB_Task[OSPrioHighRdy];
		OSCtxSw();
	}
	OSPrioCur = OSPrioHighRdy;//这里需要更新OSPrioCur，下面OSTimeDly函数里会对其进行操作。
}

;/**************************************************************************************
;* 函数名称: OSTimeDly
;*
;* 功能描述: 延时
;*
;* 参    数: ticks
;*
;* 返 回 值: None
;***************************************************************************************/
void OSTimeDly(unsigned int ticks)
{
    if (ticks > 0u) 			
	{                            
        OS_ENTER_CRITICAL();
		OSDelPrioRdy(OSPrioCur);
		TCB_Task[OSPrioCur].Dly = ticks;
		OS_EXIT_CRITICAL();
        OS_Sched();            
    }
}

;/**************************************************************************************
;* 函数名称: OSTimeTick
;*
;* 功能描述: OS滴答
;*
;* 参    数: void
;*
;* 返 回 值: None
;***************************************************************************************/
void OSTimeTick(void)
{
	char i;
	for (i = 0; i < MAX_TASK_SIZE; i++)
	{
		if (TCB_Task[i].Dly > 0)
		{
			TCB_Task[i].Dly--;
			if (TCB_Task[i].Dly == 0)
			{
				OSSetPrioRdy(i);
			}
		}
	}
}

;/**************************************************************************************
;* 函数名称: OS_IdleTask
;*
;* 功能描述: 空闲任务
;*
;* 参    数: None
;*
;* 返 回 值: None
;***************************************************************************************/

void OS_IdleTask()
{
	unsigned int Idle_count;
	while (1)
	{
		Idle_count++;
		Idle_count--;
		OS_Sched();
	}
}

;/**************************************************************************************
;* 函数名称: OS_Init
;*
;* 功能描述: OS初始化
;*
;* 参    数: None
;*
;* 返 回 值: None
;***************************************************************************************/
void OS_Init()
{
	CPU_ExceptStkBase = CPU_ExceptStk + OS_EXCEPT_STK_SIZE - 1;
	OS_ENTER_CRITICAL();
 	Task_Create(OS_IdleTask, &IDLE_TASK_STK[IDLE_STK_SIZE - 1], IDLE_TASK_PRIO);//创建起始任务
	OS_EXIT_CRITICAL();
	OSGetHighRdy();
	OSRunning = true;
	p_TCBHighRdy = &TCB_Task[OSPrioHighRdy];
	OSStartHighRdy();
}


