#include<stdint.h>
#define RCC_AHB1ENR (*(volatile uint32_t*)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR (*(volatile uint32_t*)0x40020014)
#define GPIOB_MODER (*(volatile uint32_t*)0x40020400)

#define GPIOB_ODR (*(volatile uint32_t*)0x40020414)
#define SYST_CSR (*(volatile uint32_t*)0xE000E010)
#define SYST_RVR (*(volatile uint32_t*)0xE000E014)
#define SYST_CVR (*(volatile uint32_t*)0xE000E018)
#define ICSR (*(volatile uint32_t*)0xE000ED04)
#define SHPR3 (*(volatile uint32_t*)0xE000ED20)
#define TASK_SIZE sizeof(Task_t)
#define NUM_TASKS 3
volatile uint32_t g_tick=0;

volatile uint32_t CurrentTask = 0;

void SysTick_Init(void)
{
    SYST_RVR = 16000 - 1;

    SYST_CVR = 0;

    SYST_CSR = (1<<0)|(1<<1)|(1<<2);
}

void Hardware_Init(void)
{
    RCC_AHB1ENR |= (1u << 0);   /* GPIOA clock */
    RCC_AHB1ENR |= (1u << 1);   /* GPIOB clock */

    /* PA5 output */
    GPIOA_MODER &= ~(3u << (5 * 2));
    GPIOA_MODER |=  (1u << (5 * 2));

    /* PB0 output */
    GPIOB_MODER &= ~(3u << (0 * 2));
    GPIOB_MODER |=  (1u << (0 * 2));
}
typedef enum
{
   REDY,BLOCKED,
}TaskState_t;
typedef struct

{

	uint32_t *PSP;
    uint32_t delay;
    TaskState_t state;
    void (*TaskFunc)(void);

}Task_t;

Task_t Tasks[2];
uint32_t Task1Stack[128];
uint32_t Task2Stack[128];



void task1(void)
{
    while(1)
    {
        GPIOA_ODR ^= (1<<5);

        TaskDelay(1000);
    }
}

void task2(void)
{
    while(1)
    {
        GPIOB_ODR ^= (1<<0);

        TaskDelay(1500);
    }
}
void IdleTask(void)
{
    while(1)
    {
        __asm volatile("WFI");
    }
}

void CreateTasks(void)
{
    Tasks[0].state = REDY;
    Tasks[0].delay = 0;
    Tasks[0].TaskFunc = Task1;

    Tasks[1].state = REDY;
    Tasks[1].delay = 0;
    Tasks[1].TaskFunc = Task2;

    Tasks[2].state = REDY;
    Tasks[2].delay = 0;
    Tasks[2].TaskFunc = IdleTask;

    InitTaskStack(0,Task1Stack,Task1);
    InitTaskStack(1,Task2Stack,Task2);
    InitTaskStack(2,IdleStack,IdleTask);
}


void SysTick_Handler(void)
{
    g_tick++;
    UpdateBlockedTasks();
    ICSR |= (1<<28);
}
void TaskDelay(uint32_t ticks)
{
    Tasks[CurrentTask].delay = g_tick + ticks;

    Tasks[CurrentTask].state = BLOCKED;

    ICSR |= (1<<28);

    __asm volatile("DSB");
    __asm volatile("ISB");
}
void SwitchTask(void)
{
    uint8_t next = CurrentTask;

    for(int i=0;i<NUM_TASKS;i++)
    {
        next++;

        if(next >= NUM_TASKS)
            next = 0;

        if(Tasks[next].state == REDY)
        {
            CurrentTask = next;
            return;
        }
    }
}


__attribute__((naked)) void PendSV_Handler(void)
    {
        __asm volatile(
            "MRS R0, PSP           \n"

            "STMDB R0!, {R4-R11}   \n"

            "LDR R1, =CurrentTask  \n"
            "LDR R2, [R1]          \n"

            "LDR R3, =Tasks        \n"

            "LSLS R2, R2, #4       \n"

            "ADD R3, R3, R2        \n"

            "STR R0, [R3]          \n"

            "BL SwitchTask         \n"

            "LDR R1, =CurrentTask  \n"
            "LDR R2, [R1]          \n"

            "LDR R3, =Tasks        \n"

            "LSLS R2, R2, #4       \n"

            "ADD R3, R3, R2        \n"

            "LDR R0, [R3]          \n"

            "LDMIA R0!, {R4-R11}   \n"

            "MSR PSP, R0           \n"

            "BX LR                 \n"
        );
    }
__attribute__((naked)) void StartFirstTask(void)
{
    __asm volatile(
        "LDR R0, =Tasks        \n"
        "LDR R0, [R0]          \n"

        "LDMIA R0!, {R4-R11}   \n"

        "MSR PSP, R0           \n"

        "MOVS R0,#2            \n"
        "MSR CONTROL,R0        \n"
        "ISB                   \n"

        "LDR LR,=0xFFFFFFFD    \n"
        "BX LR                 \n"
    );
}




void PSP_Init()
{
	 uint32_t psp = (uint32_t)Tasks[0].PSP;

	    __asm volatile(
	        "MSR PSP, %0"
	        :
	        : "r"(psp)
	    );

	    __asm volatile(
	        "MRS R0, CONTROL \n"
	        "ORR R0, R0, #2 \n"
	        "MSR CONTROL, R0 \n"
	        :::"r0"
	    );
}
void UpdateBlockedTasks(void)
{
    for(int i=0;i<2;i++)
    {
        if(Tasks[i].state == BLOCKED)
        {
            if(g_tick >= Tasks[i].delay)
            {
                Tasks[i].state = REDY;
            }
        }
    }
}




int main(void)
{
    Hardware_Init();
    Task_t Tasks[NUM_TASKS];

    uint32_t Task1Stack[128];
    uint32_t Task2Stack[128];
    uint32_t IdleStack[128];

    CreateTasks();

    SHPR3 &= ~(0xFF<<16);
    SHPR3 &= ~(0xFF<<24);

    SHPR3 |= (0xFF<<16);
    SHPR3 |= (0xFE<<24);

    SysTick_Init();

    StartFirstTask();

    while(1);
}


