#include <stdint.h>
#include"joystic.h"
#define RCC_AHB1ENR     (*(volatile uint32_t*)0x40023830)

#define GPIOA_MODER     (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR       (*(volatile uint32_t*)0x40020014)

#define SYST_CSR        (*(volatile uint32_t*)0xE000E010)
#define SYST_RVR        (*(volatile uint32_t*)0xE000E014)
#define SYST_CVR        (*(volatile uint32_t*)0xE000E018)
#define ICSR            (*(volatile uint32_t*)0xE000ED04)
#define SHPR3           (*(volatile uint32_t*)0xE000ED20)

#define NUM_TASKS       3
#define STACK_SIZE      256


typedef enum
{
    READY   = 0,
    BLOCKED = 1
} TaskState_t;

typedef struct
{
    uint32_t       *PSP;
    uint32_t        delay;
    uint32_t        state;
    void          (*TaskFunc)(void);
} Task_t;

_Static_assert(sizeof(Task_t) == 16, "Task_t must be 16 bytes");


volatile uint32_t g_tick      = 0;
volatile uint8_t  CurrentTask = 0;

Task_t   Tasks[NUM_TASKS];

uint32_t Task1Stack[STACK_SIZE] __attribute__((aligned(8)));
uint32_t Task2Stack[STACK_SIZE] __attribute__((aligned(8)));
uint32_t IdleStack [STACK_SIZE] __attribute__((aligned(8)));


void Task1(void);
void Task2(void);
void IdleTask(void);
void TaskDelay(uint32_t ticks);
void SwitchTask(void);
void UpdateBlockedTasks(void);
void SysTick_Init(void);
void Hardware_Init(void);
void CreateTasks(void);
void InitTaskStack(uint8_t id, uint32_t *stack, void (*task)(void));


void Hardware_Init(void)
{
    RCC_AHB1ENR |= (1 << 0);
    RCC_AHB1ENR |= (1 << 1);

    GPIOA_MODER &= ~(3 << (5 * 2));
    GPIOA_MODER |=  (1 << (5 * 2));

}


void SysTick_Init(void)
{
    SYST_RVR = 16000 - 1;
    SYST_CVR = 0u;
    SYST_CSR = (1 << 2) | (1 << 1) | (1 << 0);
}


void TaskDelay(uint32_t ticks)
{
    Tasks[CurrentTask].delay = g_tick + ticks;
    Tasks[CurrentTask].state = BLOCKED;
    ICSR |= (1u << 28);
    __asm volatile("dsb" ::: "memory");
    __asm volatile("isb");
}


void Task1(void)
{
    while (1)
    {
    joystick();
    }
}

void Task2(void)
{
    while (1)
    {
    	GPIOA_ODR ^= (1<<5);
        TaskDelay(1500);
    }
}

void IdleTask(void)
{
    while (1)
    {
        __asm volatile("wfi");
    }
}

void SwitchTask(void)
{
    uint8_t next = CurrentTask;
    uint8_t i;
    for (i = 0; i < NUM_TASKS; i++)
    {
        next++;
        if (next >= NUM_TASKS) next = 0;
        if (Tasks[next].state == READY)
        {
            CurrentTask = next;
            return;
        }
    }
    CurrentTask = (uint8_t)(NUM_TASKS - 1);
}

void UpdateBlockedTasks(void)
{
    uint8_t i;
    for (i = 0; i < NUM_TASKS; i++)
    {
        if (Tasks[i].state == BLOCKED && g_tick >= Tasks[i].delay)
        {
            Tasks[i].state = READY;
        }
    }
}


void SysTick_Handler(void)
{
    g_tick++;
    UpdateBlockedTasks();
    ICSR |= (1u << 28);
}


void InitTaskStack(uint8_t id, uint32_t *stack, void (*task)(void))
{
    uint32_t *sp = &stack[STACK_SIZE - 1];

    /* Hardware frame */
    *(sp)     = 0x01000000u;
    *(sp - 1) = (uint32_t)task;
    *(sp - 2) = 0xFFFFFFFDu;
    *(sp - 3) = 0x00000000u;
    *(sp - 4) = 0x00000000u;
    *(sp - 5) = 0x00000000u;
    *(sp - 6) = 0x00000000u;
    *(sp - 7) = 0x00000000u;

    /* Software frame */
    *(sp - 8)  = 0x00000000u;
    *(sp - 9)  = 0x00000000u;
    *(sp - 10) = 0x00000000u;
    *(sp - 11) = 0x00000000u;
    *(sp - 12) = 0x00000000u;
    *(sp - 13) = 0x00000000u;
    *(sp - 14) = 0x00000000u;
    *(sp - 15) = 0x00000000u;

    Tasks[id].PSP = (sp - 15);
}


void CreateTasks(void)
{
    Tasks[0].state    = READY;
    Tasks[0].delay    = 0;
    Tasks[0].TaskFunc = Task1;
    InitTaskStack(0, Task1Stack, Task1);

    Tasks[1].state    = READY;
    Tasks[1].delay    = 0;
    Tasks[1].TaskFunc = Task2;
    InitTaskStack(1, Task2Stack, Task2);

    Tasks[2].state    = READY;
    Tasks[2].delay    = 0;
    Tasks[2].TaskFunc = IdleTask;
    InitTaskStack(2, IdleStack, IdleTask);
}

__attribute__((naked))
void SVC_Handler(void)
{
    __asm volatile(

        "LDR  R0, =Tasks          \n"
        "LDR  R0, [R0, #0]        \n"


        "MSR  PSP, R0             \n"


        "MOV  R1, #0x02           \n"
        "MSR  CONTROL, R1         \n"
        "ISB                      \n"


        "LDMIA R0!, {R4-R11}      \n"


        "MSR  PSP, R0             \n"


        "MOV  LR, #0xFFFFFFFD    \n"
        "BX   LR                  \n"
    );
}


__attribute__((naked))
void PendSV_Handler(void)
{
    __asm volatile(


        "MRS   R0, PSP                \n"
        "STMDB R0!, {R4-R11}          \n"

        "LDR   R1, =CurrentTask       \n"
        "LDRB  R2, [R1]               \n"
        "LDR   R3, =Tasks             \n"
        "LSL   R2, R2, #4             \n"
        "ADD   R3, R3, R2             \n"
        "STR   R0, [R3, #0]           \n"

        "PUSH  {LR}                   \n"
        "BL    SwitchTask             \n"
        "POP   {LR}                   \n"

        "LDR   R1, =CurrentTask       \n"
        "LDRB  R2, [R1]               \n"
        "LDR   R3, =Tasks             \n"
        "LSL   R2, R2, #4             \n"
        "ADD   R3, R3, R2             \n"
        "LDR   R0, [R3, #0]           \n"

        "LDMIA R0!, {R4-R11}          \n"
        "MSR   PSP, R0                \n"

        "BX    LR                     \n"
    );
}



int main(void)
{
    Hardware_Init();
    CreateTasks();


    SHPR3 &= ~(0xFFu << 16);
    SHPR3 &= ~(0xFFu << 24);
    SHPR3 |=  (0xFFu << 16);
    SHPR3 |=  (0xFEu << 24);

    SysTick_Init();


    __asm volatile("SVC #0");

    while (1) { }
}
