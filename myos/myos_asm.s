
.syntax unified
.cpu cortex-m4
.thumb

.equ    SCB_ICSR_ADDR, 0xE000ED04

.global myosCPUSaveSR
.global myosCPURestoreSR
.global myosStartHighRdy
.global myosCtxSw
.global PendSV_Handler
.global HardFault_Handler

.extern myosRunning
.extern myosTCBCur
.extern myosTCBHighRdy

.text

.type  myosCPUSaveSR, %function
myosCPUSaveSR:
    MOV     R1, #0x00000001
    MRS     R0, PRIMASK
    MSR     PRIMASK, R1
    BX      LR

.type  myosCPURestoreSR, %function
myosCPURestoreSR:
    MSR     PRIMASK, R0
    BX      LR

.type  myosStartHighRdy, %function
myosStartHighRdy:
    PUSH    {R0,R1,LR}
    CPSID   I
    LDR     R0, =myosRunning    @Set myosRunning to myos_TRUE
    MOV     R1, #0x01
    STR     R1, [R0]
    
    MOV     R1, #0x00
    MSR     PSP, R1
    BL      myosCtxSw
    CPSIE   I
    POP     {R0,R1,LR}
    BX      LR

.type  myosCtxSw, %function
myosCtxSw:                      @To trigger PendSV for real context switch
    PUSH    {R0,R1}
    LDR     R0, =SCB_ICSR_ADDR
    LDR     R1, [R0]
    ORR     R1, R1, #0x10000000
    STR     R1, [R0]
    POP     {R0,R1}
    BX      LR

.section .text.PendSV_Handler
    .type  PendSV_Handler, %function
PendSV_Handler:
    CPSID   I
    MRS     R0, PSP             @If PSP is empty means have no task running skip saving
    CBZ     R0, LOAD
    
    STMFD   R0!, {R4-R11}       @saving register
    
    LDR     R1, =myosTCBCur
    LDR     R2, [R1]
    STR     R0, [R2]            @update sp to TCB

LOAD:
    LDR     R0, =myosTCBCur
    LDR     R1, =myosTCBHighRdy
    LDR     R2, [R1]
    STR     R2, [R0]            @myosTCBHighRdy -> myosTCBCur
    
    LDR     R0, [R2]            @R2 = Address of TCB, [R2] = Address of TCB's Stk
    LDMFD   R0!,{R4-R11}
    
    MSR     PSP, R0
    
    ORR     LR, LR, #0x04       @Return to PSP
    CPSIE   I
    BX      LR
    .size  PendSV_Handler, .-PendSV_Handler
