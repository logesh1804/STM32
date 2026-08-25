\#include<stdint.h>
#define RCC_AHB1ENR    (*(volatile uint32_t*)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR (*(volatile uint32_t*)0x40020014)
#define STACK_SIZE 128
#define SYSTICK_LOAD  (*(volatile uint32_t*)0xE000E014)
#define SYSTICK_VAL   (*(volatile uint32_t*)0xE000E018)
#define SYSTICK_CTRL  (*(volatile uint32_t*)0xE000E010)
volatile uint32_t gtick=0;

typedef struct
{
  uint32_t *psp_value;
}TCB_t;
uint32_t task1_stack[STACK_SIZE];
uint32_t task2_stack[STACK_SIZE];
TCB_t task1_tcb;
TCB_t task2_tcb;
TCB_t *current_tcb;
void delay(){
	for(int i=0;i<500000;i++){
	}
}
void GPIO_Init(void)
{
	RCC_AHB1ENR |=(1<<0);
	GPIOA_MODER &=~(3<<10);//PA5 CLEAR
	GPIOA_MODER |=(1<<10);//PA5 SET
	GPIOA_MODER &=~(3<<12);//PA6 CLEAR
	GPIOA_MODER |=(1<<12);//PA6 SET
}
//-----------TASK1---------------------//
void Task1(void)
{
	while(1)
	{
		{
		GPIOA_ODR |=(1<<5);
	    gtick=0;
        GPIOA_ODR &=~(1<<5);
	}
}
}
//-----------TASK2---------------------//
void Task2(void)
{
	while(1)
	{   if(gtick==1000){
		GPIOA_ODR |=(1<<6);
		 gtick=0;
        GPIOA_ODR &=~(1<<6);
	}
}
}
//------------PSP STACK---------------//
void Init_task_stack(TCB_t*tcb,uint32_t*stack,void(*task)(void))
{
	uint32_t*psp=0;
	psp = stack+STACK_SIZE;
//---------------stack frame 1------------------------//
	*(--psp)=0x01000000;                        //xpsr
	*(--psp)=(uint32_t)task;                    //pc
	*(--psp)= 0xFFFFFFFD;                       //LR
     *(--psp)=0;                                //r12
	 *(--psp)=0;                                //r3
	 *(--psp)=0;                                //r2
	 *(--psp)=0;                                //r1
	 *(--psp)=0;                                //r0
//---------------stack frame 2 ------------------------//
	 *(--psp)=0;                                //r11
	 *(--psp)=0;                                //r10
	 *(--psp)=0;                                //r9
	 *(--psp)=0;                                //r8
	 *(--psp)=0;                                //r7
	 *(--psp)=0;                                //r6
	 *(--psp)=0;                                //r5
	 *(--psp)=0;                                //r4
	 tcb->psp_value=psp;
}
//---------------------------psp setup-----------------//
void set_psp(uint32_t psp_val)
{
	__asm volatile (
		"MSR PSP,%0"
		:
		:"r"(psp_val)
	);
}
//-------------- switch MSP ---TO----PSP------------------//
void Switch_to_psp(void)
{
	__asm volatile(
	"MOV R0,#0x02  \n"
	"MSR CONTROL ,R0 \n"
	"ISB   \n"
);
}
//-----------SYSTICK INIT------------------------------//
void SysTick_Init(void)
{
	SYSTICK_CTRL = 0;
	SYSTICK_LOAD = (16000000/100 )- 1;
	SYSTICK_VAL  = 0;
	SYSTICK_CTRL = 0x07;
}
//-----------SCHEDULER------------------------------//
void schedule(void)
{
	if(current_tcb == &task1_tcb)
		if(gtick==1000){
		current_tcb = &task2_tcb;
		gtick=0;
		}
	else
		if(gtick==1000){
		current_tcb = &task1_tcb;
		gtick=0;}
}
//-----------CONTEXT SWITCH (full asm)-----------------//
__attribute__((naked)) void SysTick_Handler(void)
{


	__asm volatile(
		// 1. SAVE current task context
		"ADD %0 #1      \n"
		"MRS R0, PSP              \n"   // get current PSP
		"STMDB R0!, {R4-R11}      \n"   // push r4-r11 onto task stack

		// 2. Save PSP into current TCB
		"LDR R1, =current_tcb     \n"   // load address of current_tcb pointer
		"LDR R2, [R1]             \n"   // load current_tcb value (pointer to TCB)
		"STR R0, [R2]             \n"   // save PSP into tcb->psp_value

		// 3. Call scheduler to switch current_tcb
		"PUSH {LR}                \n"   // save LR before function call
		"BL schedule              \n"   // switch current_tcb
		"POP {LR}                 \n"   // restore LR

		// 4. RESTORE next task context
		"LDR R1, =current_tcb     \n"   // reload current_tcb (now updated)
		"LDR R2, [R1]             \n"   // load new TCB pointer
		"LDR R0, [R2]             \n"   // load new task's PSP

		"LDMIA R0!, {R4-R11}      \n"   // pop r4-r11 from new task's stack
		"MSR PSP, R0              \n"   // set PSP to new task's stack pointer

		// 5. Return from exception
		"BX LR                    \n"
	    ::"r"(gtick)
	);
}
//-----------MAIN-----------------------------------------//
int main(void)
{

	GPIO_Init();
	    Init_task_stack(&task1_tcb,task1_stack,Task1);
	    Init_task_stack(&task2_tcb,task2_stack,Task2);
	current_tcb = &task1_tcb;
	set_psp((uint32_t)task1_tcb.psp_value);
	Switch_to_psp();
	SysTick_Init();
	Task1();
	while(1);
}
