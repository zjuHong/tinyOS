#include "myOS.h"

unsigned int OSRdyTbl;//��������������
unsigned int OSPrioHighRdy;//��ǰ������������ȼ�
unsigned int OSPrioCur;//��ǰ���ȼ�
unsigned int CPU_ExceptStk[OS_EXCEPT_STK_SIZE];//����ջ
unsigned int IDLE_TASK_STK[IDLE_STK_SIZE];//���������ջ
unsigned int *CPU_ExceptStkBase;//ָ���������һ��Ԫ��
bool OSRunning;//OS�Ƿ�����

TCB TCB_Task[MAX_TASK_SIZE];//����TCB
TCB *p_TCB_Cur;//��ǰ����TCB
TCB *p_TCBHighRdy;//������ȼ�TCB

;/**************************************************************************************
;* ��������: OSSetPrioRdy
;*
;* ��������: �����û�������״̬
;*
;* ��    ��: prio
;*
;* �� �� ֵ: None
;***************************************************************************************/
void OSSetPrioRdy(unsigned int prio)
{
	if (prio == 0) return;//��ʱ����ֵΪ0�����ȼ�
	OSRdyTbl |= 0x01 << prio;
}

;/**************************************************************************************
;* ��������: OSDelPrioRdy
;*
;* ��������: ɾ���û�������״̬
;*
;* ��    ��: prio
;*
;* �� �� ֵ: None
;***************************************************************************************/
void OSDelPrioRdy(unsigned int prio)
{
	OSRdyTbl &= ~(0x01 << prio);
}

;/**************************************************************************************
;* ��������: OSGetHighRdy
;*
;* ��������: ɸѡ���ȼ��������
;*
;* ��    ��: None
;*
;* �� �� ֵ: None
;***************************************************************************************/
void OSGetHighRdy()
{
	unsigned int OSNextTaskPrio;
	for (OSNextTaskPrio = 0; (OSNextTaskPrio < MAX_TASK_SIZE) && (!(OSRdyTbl & (0x01 << OSNextTaskPrio))); OSNextTaskPrio++)
		;
	OSPrioHighRdy = OSNextTaskPrio;
}

;/**************************************************************************************
;* ��������: Task_End
;*
;* ��������: ����һ������
;*
;* ��    ��: None
;*
;* �� �� ֵ: None
;***************************************************************************************/
void Task_End(void)
{
	while (1)
	{}
}

;/**************************************************************************************
;* ��������: Task_Create
;*
;* ��������: �½�һ������
;*
;* ��    ��: task stk prio
;*
;* �� �� ֵ: None
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
;* ��������: OS_Sched
;*
;* ��������: ���̵���
;*
;* ��    ��: None
;*
;* �� �� ֵ: None
;***************************************************************************************/
void OS_Sched(void)
{
	OSGetHighRdy();
	if (OSPrioHighRdy != OSPrioCur)
	{
		p_TCBHighRdy = &TCB_Task[OSPrioHighRdy];
		OSCtxSw();
	}
	OSPrioCur = OSPrioHighRdy;//������Ҫ����OSPrioCur������OSTimeDly������������в�����
}

;/**************************************************************************************
;* ��������: OSTimeDly
;*
;* ��������: ��ʱ
;*
;* ��    ��: ticks
;*
;* �� �� ֵ: None
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
;* ��������: OSTimeTick
;*
;* ��������: OS�δ�
;*
;* ��    ��: void
;*
;* �� �� ֵ: None
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
;* ��������: OS_IdleTask
;*
;* ��������: ��������
;*
;* ��    ��: None
;*
;* �� �� ֵ: None
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
;* ��������: OS_Init
;*
;* ��������: OS��ʼ��
;*
;* ��    ��: None
;*
;* �� �� ֵ: None
;***************************************************************************************/
void OS_Init()
{
	CPU_ExceptStkBase = CPU_ExceptStk + OS_EXCEPT_STK_SIZE - 1;
	OS_ENTER_CRITICAL();
 	Task_Create(OS_IdleTask, &IDLE_TASK_STK[IDLE_STK_SIZE - 1], IDLE_TASK_PRIO);//������ʼ����
	OS_EXIT_CRITICAL();
	OSGetHighRdy();
	OSRunning = true;
	p_TCBHighRdy = &TCB_Task[OSPrioHighRdy];
	OSStartHighRdy();
}


