#include <stdint.h>

/********************************************************
 * RCC
 ********************************************************/
#define RCC_AHB1ENR     (*(volatile uint32_t*)0x40023830)

/********************************************************
 * GPIOA
 ********************************************************/
#define GPIOA_MODER     (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR       (*(volatile uint32_t*)0x40020014)

/********************************************************
 * GPIOB
 ********************************************************/
#define GPIOB_MODER     (*(volatile uint32_t*)0x40020400)
#define GPIOB_ODR       (*(volatile uint32_t*)0x40020414)

/********************************************************
 * SysTick  (base: 0xE000E010)
 ********************************************************/
#define SYST_CSR        (*(volatile uint32_t*)0xE000E010)
#define SYST_RVR        (*(volatile uint32_t*)0xE000E014)
#define SYST_CVR        (*(volatile uint32_t*)0xE000E018)

/********************************************************
 * SCB
 ********************************************************/
#define ICSR            (*(volatile uint32_t*)0xE000ED04)
#define SHPR3           (*(volatile uint32_t*)0xE000ED20)  /* PendSV[23:16] SysTick[31:24] */

/********************************************************
 * Config
 ********************************************************/
#define NUM_TASKS       3
#define STACK_SIZE      256   /* words — 256*4 = 1 KB per task */

/********************************************************
 * Task States
 ********************************************************/
typedef enum
{
    READY   = 0,
    BLOCKED = 1
} TaskState_t;

/********************************************************
 * TCB  — must stay 16 bytes so LSLS #4 offset works
 ********************************************************/
typedef struct
{
    uint32_t       *PSP;       /* offset 0  */
    uint32_t        delay;     /* offset 4  */
    uint32_t        state;     /* offset 8  (TaskState_t stored as uint32) */
    void          (*TaskFunc)(void); /* offset 12 */
} Task_t;

_Static_assert(sizeof(Task_t) == 16, "Task_t must be 16 bytes");

/********************************************************
 * Globals
 ********************************************************/
volatile uint32_t g_tick      = 0;
volatile uint8_t  CurrentTask = 0;

Task_t   Tasks[NUM_TASKS];

uint32_t Task1Stack[STACK_SIZE] __attribute__((aligned(8)));
uint32_t Task2Stack[STACK_SIZE] __attribute__((aligned(8)));
uint32_t IdleStack [STACK_SIZE] __attribute__((aligned(8)));

/********************************************************
 * Forward Declarations
 ********************************************************/
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

/* Implemented in this file with __attribute__((naked)) */
void SVC_Handler(void);
void PendSV_Handler(void);

/********************************************************
 * Hardware Init
 * PA5  — onboard LED (LD2, active HIGH)
 * PB0  — second LED / GPIO
 ********************************************************/
void Hardware_Init(void)
{
    /* Enable GPIOA and GPIOB clocks */
    RCC_AHB1ENR |= (1u << 0);
    RCC_AHB1ENR |= (1u << 1);

    /* PA5 → output */
    GPIOA_MODER &= ~(3u << (5 * 2));
    GPIOA_MODER |=  (1u << (5 * 2));

    /* PB0 → output */
    GPIOB_MODER &= ~(3u << (0 * 2));
    GPIOB_MODER |=  (1u << (0 * 2));
}

/********************************************************
 * SysTick — 1 ms tick at 16 MHz HSI (reset default)
 * If PLL is enabled, change SYST_RVR accordingly:
 *   84 MHz → 84000-1
 *  100 MHz → 100000-1
 ********************************************************/
void SysTick_Init(void)
{
    SYST_RVR = 16000u - 1u;   /* 16 000 cycles = 1 ms @ 16 MHz */
    SYST_CVR = 0u;
    SYST_CSR = (1u << 2)      /* CLKSOURCE: processor clock */
             | (1u << 1)      /* TICKINT:   enable interrupt */
             | (1u << 0);     /* ENABLE */
}

/********************************************************
 * Task Delay
 * Marks caller BLOCKED and immediately triggers PendSV
 * so another task can run while this one waits.
 ********************************************************/
void TaskDelay(uint32_t ticks)
{
    Tasks[CurrentTask].delay = g_tick + ticks;
    Tasks[CurrentTask].state = BLOCKED;

    /* Pend PendSV — will fire after this function returns to the task */
    ICSR |= (1u << 28);

    /* Ensure the write is visible before returning */
    __asm volatile("dsb" ::: "memory");
    __asm volatile("isb");
}

/********************************************************
 * Task1 — toggle PA5 every 1000 ms
 ********************************************************/
void Task1(void)
{
    while (1)
    {
        GPIOA_ODR ^= (1u << 5);
        TaskDelay(1000);
    }
}

/********************************************************
 * Task2 — toggle PB0 every 1500 ms
 ********************************************************/
void Task2(void)
{
    while (1)
    {
        GPIOB_ODR ^= (1u << 0);
        TaskDelay(1500);
    }
}

/********************************************************
 * Idle Task — runs when nothing else is READY
 ********************************************************/
void IdleTask(void)
{
    while (1)
    {
        __asm volatile("wfi");
    }
}

/********************************************************
 * Round-Robin Scheduler
 * Searches for the next READY task starting after
 * CurrentTask.  Falls back to Idle (index 2) if none
 * found — Idle is always READY.
 ********************************************************/
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

    /* Should never reach here because Idle is always READY,
       but be safe */
    CurrentTask = (uint8_t)(NUM_TASKS - 1);
}

/********************************************************
 * Update Blocked Tasks
 * Called from SysTick_Handler every 1 ms.
 * Unblocks any task whose delay has expired.
 ********************************************************/
void UpdateBlockedTasks(void)
{
    uint8_t i;
    for (i = 0; i < NUM_TASKS; i++)
    {
        if (Tasks[i].state == BLOCKED)
        {
            if (g_tick >= Tasks[i].delay)
            {
                Tasks[i].state = READY;
            }
        }
    }
}

/********************************************************
 * SysTick Handler
 ********************************************************/
void SysTick_Handler(void)
{
    g_tick++;
    UpdateBlockedTasks();
    /* Pend a context switch — PendSV fires after SysTick returns */
    ICSR |= (1u << 28);
}

/********************************************************
 * InitTaskStack
 *
 * Cortex-M4 full-descending stack.  On exception entry
 * the hardware automatically pushes (high → low addr):
 *
 *   xPSR  ← sp+28  (Thumb bit set: 0x01000000)
 *   PC    ← sp+24  (task entry point, bit0 = 0 for PC)
 *   LR    ← sp+20  (EXC_RETURN = 0xFFFFFFFD → Thread/PSP)
 *   R12   ← sp+16
 *   R3    ← sp+12
 *   R2    ← sp+8
 *   R1    ← sp+4
 *   R0    ← sp+0
 *
 * PendSV additionally saves/restores (software):
 *   R11 R10 R9 R8 R7 R6 R5 R4   ← pushed BELOW the hw frame
 *
 * InitTaskStack pre-fills this so the very first PendSV
 * restore works correctly.
 *
 * Stack grows DOWN.  We start from the top word of the
 * array and work towards index 0.
 *
 * Final PSP stored in Tasks[id].PSP points to the lowest
 * address (where R4 sits), because LDMIA in PendSV pops
 * upward.
 ********************************************************/
void InitTaskStack(uint8_t id, uint32_t *stack, void (*task)(void))
{
    /* sp points to the last (highest-address) word in the array */
    uint32_t *sp = &stack[STACK_SIZE - 1];

    /* --- Hardware-saved frame (pushed by CPU on exception entry) --- */
    *(sp--) = 0x01000000u;              /* xPSR: Thumb bit set            */
    *(sp--) = (uint32_t)task;           /* PC:   task entry (LSB=0 for PC)*/
    *(sp--) = 0xFFFFFFFDu;             /* LR:   EXC_RETURN Thread/PSP    */
    *(sp--) = 0x00000000u;             /* R12                            */
    *(sp--) = 0x00000000u;             /* R3                             */
    *(sp--) = 0x00000000u;             /* R2                             */
    *(sp--) = 0x00000000u;             /* R1                             */
    *(sp--) = 0x00000000u;             /* R0                             */

    /* --- Software-saved frame (pushed by PendSV STMDB) --- */
    *(sp--) = 0x00000000u;             /* R11 */
    *(sp--) = 0x00000000u;             /* R10 */
    *(sp--) = 0x00000000u;             /* R9  */
    *(sp--) = 0x00000000u;             /* R8  */
    *(sp--) = 0x00000000u;             /* R7  */
    *(sp--) = 0x00000000u;             /* R6  */
    *(sp--) = 0x00000000u;             /* R5  */
    *(sp--) = 0x00000000u;             /* R4  */

    /*
     * sp now points ONE BELOW the R4 slot.
     * PSP must point TO R4 (the lowest saved word),
     * so we add 1 back.
     */
    Tasks[id].PSP = sp + 1;
}

/********************************************************
 * CreateTasks
 ********************************************************/
void CreateTasks(void)
{
    Tasks[0].state    = READY;
    Tasks[0].TaskFunc = Task1;
    InitTaskStack(0, Task1Stack, Task1);

    Tasks[1].state    = READY;
    Tasks[1].TaskFunc = Task2;
    InitTaskStack(1, Task2Stack, Task2);

    Tasks[2].state    = READY;         /* Idle is always READY */
    Tasks[2].TaskFunc = IdleTask;
    InitTaskStack(2, IdleStack, IdleTask);
}

/********************************************************
 * SVC_Handler — used ONLY to start the very first task.
 *
 * Strategy:
 *   main() calls  __asm("SVC 0")  which is a real
 *   exception entry.  We are now inside an exception,
 *   so EXC_RETURN values are legal.
 *
 *   1. Load Tasks[0].PSP
 *   2. Switch to PSP / unprivileged Thread mode
 *   3. Pop software frame (R4-R11)
 *   4. Set PSP past the software frame
 *   5. BX with EXC_RETURN — CPU pops hw frame and
 *      jumps to Task1 entry point.
 ********************************************************/
__attribute__((naked))
void SVC_Handler(void)
{
    __asm volatile(
        /* ---- Load Tasks[0].PSP ---- */
        "LDR  R0, =Tasks          \n"  /* R0 = &Tasks[0]          */
        "LDR  R0, [R0, #0]        \n"  /* R0 = Tasks[0].PSP       */

        /* ---- Switch stack pointer to PSP ---- */
        "MSR  PSP, R0             \n"  /* PSP = Tasks[0].PSP      */

        /* ---- Switch to PSP, unprivileged Thread mode ---- */
        "MOV  R1, #0x3            \n"  /* SPSEL=1 nPRIV=1         */
        "MSR  CONTROL, R1         \n"
        "ISB                      \n"  /* flush pipeline           */

        /* ---- Pop software-saved R4-R11 from the initial frame ---- */
        "LDMIA R0!, {R4-R11}      \n"  /* R0 now past R4-R11      */

        /* ---- Update PSP to point at hardware frame ---- */
        "MSR  PSP, R0             \n"

        /* ---- EXC_RETURN: Thread mode, PSP ---- */
        "LDR  LR, =0xFFFFFFFD    \n"
        "BX   LR                  \n"  /* CPU pops hw frame → Task1*/
    );
}

/********************************************************
 * PendSV_Handler — context switch
 *
 * Entry: CPU already saved R0-R3, R12, LR, PC, xPSR
 *        onto the task stack (PSP).  LR = EXC_RETURN.
 *
 * Save:
 *   MRS R0, PSP               — get current task PSP
 *   STMDB R0!, {R4-R11}       — push software frame
 *   Tasks[CurrentTask].PSP=R0 — save new PSP top
 *
 * Switch:
 *   SwitchTask()               — update CurrentTask
 *
 * Restore:
 *   R0 = Tasks[CurrentTask].PSP
 *   LDMIA R0!, {R4-R11}        — pop software frame
 *   MSR PSP, R0                — set PSP for new task
 *   BX LR                      — CPU pops hw frame → new task
 ********************************************************/
__attribute__((naked))
void PendSV_Handler(void)
{
    __asm volatile(

        /* ========== Save current task context ========== */

        "MRS   R0, PSP               \n"  /* R0 = current PSP            */
        "STMDB R0!, {R4-R11}         \n"  /* push sw frame; R0 updated   */

        /* Tasks[CurrentTask].PSP = R0 */
        "LDR   R1, =CurrentTask      \n"
        "LDRB  R2, [R1]              \n"  /* R2 = CurrentTask (uint8)    */
        "LDR   R3, =Tasks            \n"
        "LSL   R2, R2, #4            \n"  /* R2 *= 16 (sizeof Task_t)    */
        "ADD   R3, R3, R2            \n"  /* R3 = &Tasks[CurrentTask]    */
        "STR   R0, [R3, #0]          \n"  /* Tasks[cur].PSP = R0         */

        /* ========== Select next task ========== */

        "PUSH  {LR}                  \n"  /* preserve EXC_RETURN         */
        "BL    SwitchTask            \n"
        "POP   {LR}                  \n"

        /* ========== Load new task context ========== */

        "LDR   R1, =CurrentTask      \n"
        "LDRB  R2, [R1]              \n"  /* R2 = new CurrentTask        */
        "LDR   R3, =Tasks            \n"
        "LSL   R2, R2, #4            \n"
        "ADD   R3, R3, R2            \n"  /* R3 = &Tasks[new]            */
        "LDR   R0, [R3, #0]          \n"  /* R0 = Tasks[new].PSP         */

        "LDMIA R0!, {R4-R11}         \n"  /* pop sw frame; R0 updated    */
        "MSR   PSP, R0               \n"  /* set PSP for new task        */

        "BX    LR                    \n"  /* EXC_RETURN → new task runs  */
    );
}

/********************************************************
 * main
 ********************************************************/
int main(void)
{
    Hardware_Init();
    CreateTasks();

    /*
     * Priority setup (SHPR3):
     *   Bits [23:16] = PendSV  priority → 0xFF (lowest)
     *   Bits [31:24] = SysTick priority → 0xFE (one above PendSV)
     *
     * Lower numerical value = higher priority on Cortex-M.
     * PendSV must be LOWEST so it fires after all ISRs finish.
     */
    SHPR3 &= ~(0xFFu << 16);
    SHPR3 &= ~(0xFFu << 24);
    SHPR3 |=  (0xFFu << 16);   /* PendSV  = 0xFF (lowest)  */
    SHPR3 |=  (0xFEu << 24);   /* SysTick = 0xFE           */

    SysTick_Init();

    /*
     * Trigger SVC to start first task.
     * SVC_Handler loads Task0 context and does EXC_RETURN.
     * Execution never returns here.
     */
    __asm volatile("SVC 0");

    /* Should never reach here */
    while (1) { }
}
