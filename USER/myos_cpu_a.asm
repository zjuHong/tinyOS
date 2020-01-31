;/*********************** (C) COPYRIGHT 2010 Libraworks *************************
;* File Name	: os_cpu_a.asm 
;* Author		: Librae 
;* Version		: V1.0
;* Date			: 06/10/2010
;* Description	: μCOS-II asm port	for STM32
;*******************************************************************************/

        EXTERN  CPU_ExceptStkBase ;主堆栈
        EXTERN  p_TCB_Cur ;指向TCB的指针
        EXTERN  p_TCBHighRdy ;指向最高优先级任务的指针
			
        EXPORT  OSStartHighRdy               
        EXPORT  OSCtxSw
        EXPORT  OSIntCtxSw      
        EXPORT  PendSV_Handler
        	
     
NVIC_INT_CTRL   	EQU     0xE000ED04  ; 中断控制寄存器
NVIC_SYSPRI2    	EQU     0xE000ED22  ; 0xE000ED20 系统优先级寄存器(2)
NVIC_PENDSV_PRI 	EQU     0xFF        ; 0xFFFF0000 PendSV中断和系统节拍中断
                                        ; (都为最低，0xff).
NVIC_PENDSVSET  	EQU     0x10000000  ; 触发软件中断的值.


		PRESERVE8 
		
		AREA    |.text|, CODE, READONLY
		THUMB 
    
;/**************************************************************************************
;* 函数名称: OSStartHighRdy
;*
;* 功能描述: 使用调度器运行第一个任务
;* 
;* 参    数: None
;*
;* 返 回 值: None
;**************************************************************************************/  

OSStartHighRdy
        LDR     R4, =NVIC_SYSPRI2      ; set the PendSV exception priority
        LDR     R5, =NVIC_PENDSV_PRI
        STRB    R5, [R4]

        MOVS    R4, #0                 ; set the PSP to 0 for initial context switch call
        MSR     PSP, R4

        LDR     R4, =CPU_ExceptStkBase         ; 初始化主堆栈为CPU_ExceptStkBase
        LDR     R5, [R4]
        MSR     MSP, R5

                                       ;切换到最高优先级的任务
        LDR     R4, =NVIC_INT_CTRL     ;rigger the PendSV exception (causes context switch)
        LDR     R5, =NVIC_PENDSVSET
        STR     R5, [R4]

        CPSIE   I                      ;enable interrupts at processor level
OSStartHang
        B       OSStartHang            ;should never get here

;/**************************************************************************************
;* 函数名称: OSCtxSw
;*
;* 功能描述: 任务级上下文切换         
;*
;* 参    数: None
;*
;* 返 回 值: None
;***************************************************************************************/
  
OSCtxSw
        LDR     R4, =NVIC_INT_CTRL  	;触发PendSV异常 (causes context switch)
        LDR     R5, =NVIC_PENDSVSET
        STR     R5, [R4]
        BX      LR

;/**************************************************************************************
;* 函数名称: OSIntCtxSw
;*
;* 功能描述: 中断级任务切换
;*
;* 参    数: None
;*
;* 返 回 值: None
;***************************************************************************************/

OSIntCtxSw
        LDR     R4, =NVIC_INT_CTRL      ;触发PendSV异常 (causes context switch)
        LDR     R5, =NVIC_PENDSVSET
        STR     R5, [R4]
        BX      LR
        NOP

;/**************************************************************************************
;* 函数名称: OSPendSV
;*
;* 功能描述: OSPendSV is used to cause a context switch.
;*
;* 参    数: None
;*
;* 返 回 值: None
;***************************************************************************************/

PendSV_Handler
    CPSID   I                                                   ; Prevent interruption during context switch
    MRS     R0, PSP                                             ; PSP is process stack pointer 如果在用PSP堆栈,则可以忽略保存寄存器,参考CM3权威中的双堆栈-白菜注
    CBZ     R0, PendSV_Handler_Nosave		                    ; Skip register save the first time

    SUBS    R0, R0, #0x20                                       ; Save remaining regs r4-11 on process stack
    STM     R0, {R4-R11}

    LDR     R1, =p_TCB_Cur                                       ; OSTCBCur->OSTCBStkPtr = SP;
    LDR     R1, [R1]											; 这三句让p_TCB_Cur->StkPtr = SP,指向新的栈顶
    STR     R0, [R1]                                            ; R0 is SP of process being switched out

                                                                ; At this point, entire context of process has been saved
PendSV_Handler_Nosave
    LDR     R0, =p_TCB_Cur                                      ; P_TCB_Cur = p_TCBHighRdy;
    LDR     R1, =p_TCBHighRdy
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     R0, [R2]                                            ; R0 is new process SP; SP = OSTCBHighRdy->OSTCBStkPtr;
    LDM     R0, {R4-R11}                                        ; Restore r4-11 from new process stack
    ADDS    R0, R0, #0x20
    
	MSR     PSP, R0                                             ; Load PSP with new process SP
    ORR     LR, LR, #0x04                                       ; Ensure exception return uses process stack
    
	CPSIE   I
    BX      LR                                                  ; Exception return will restore remaining context
 end  