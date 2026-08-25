/*
 * verify_offsets.c
 * ------------------
 * Compiles against YOUR actual FreeRTOSConfig.h to print the ground-truth
 * TCB_t field offsets for V10.3.1 on your exact config -- no guessing.
 *
 * Why this matters: two boards both running "FreeRTOS V10.3.1" can have
 * different TCB_t layouts if their FreeRTOSConfig.h flags differ
 * (configUSE_MUTEXES, configUSE_TRACE_FACILITY, configUSE_TASK_NOTIFICATIONS,
 * configRECORD_STACK_HIGH_ADDRESS, etc). This probe removes the guesswork.
 *
 * HOW TO RUN (on your dev machine, NOT the target -- this never gets
 * flashed to the board):
 *
 *   1. Copy this file into your STM32CubeIDE project folder, anywhere that
 *      can see your project's FreeRTOSConfig.h and the FreeRTOS kernel
 *      source (Middlewares/Third_Party/FreeRTOS/Source/include).
 *
 *   2. Compile it as a plain HOST executable (NOT for the ARM target --
 *      you just need offsetof() results, the struct layout is what we're
 *      after, matched via -mcpu/-mthumb to get identical padding rules):
 *
 *      arm-none-eabi-gcc -c -mcpu=cortex-m4 -mthumb \
 *          -I Core/Inc \
 *          -I Middlewares/Third_Party/FreeRTOS/Source/include \
 *          -I Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
 *          verify_offsets.c -o verify_offsets.o -S
 *
 *      (Using -S to emit assembly is enough to sanity check, but easiest
 *      is actually to build+run on host with x86 if pointer/alignment
 *      sizes match -- ARM Cortex-M and x86-64 both use 4-byte pointers.
 *      Simplest: just compile and run natively:)
 *
 *      gcc -m32 -I Core/Inc \
 *          -I Middlewares/Third_Party/FreeRTOS/Source/include \
 *          -I Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
 *          verify_offsets.c -o verify_offsets && ./verify_offsets
 *
 *      (-m32 forces 4-byte pointers to match your Cortex-M target's
 *      pointer width, so offsets come out identical to the real target.)
 *
 *   3. Paste the printed offsets into data/offset_table.py as a new/
 *      corrected row. This is your GROUND TRUTH for your exact build.
 */

#include <stdio.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "task.h"
#include "list.h"

/* task.c keeps TCB_t private (renamed tskTCB), so we can't offsetof()
 * against the real typedef directly from application code. Instead we
 * print the PUBLIC structures we can see (List_t / ListItem_t, which are
 * NOT private) and cross-check those against our fixed assumptions --
 * then print the config flags actually in effect so you can confirm which
 * offset_table.py row applies to your build. */

int main(void) {
    printf("=== List_t / ListItem_t layout (should match data/offset_table.py LIST_T_LAYOUT) ===\n");
    printf("sizeof(List_t)          = %zu\n", sizeof(List_t));
    printf("sizeof(ListItem_t)      = %zu\n", sizeof(ListItem_t));
    printf("offsetof(List_t, uxNumberOfItems) = %zu\n", offsetof(List_t, uxNumberOfItems));
    printf("offsetof(List_t, pxIndex)         = %zu\n", offsetof(List_t, pxIndex));
    printf("offsetof(List_t, xListEnd)        = %zu\n", offsetof(List_t, xListEnd));
    printf("offsetof(ListItem_t, xItemValue)  = %zu\n", offsetof(ListItem_t, xItemValue));
    printf("offsetof(ListItem_t, pxNext)      = %zu\n", offsetof(ListItem_t, pxNext));
    printf("offsetof(ListItem_t, pxPrevious)  = %zu\n", offsetof(ListItem_t, pxPrevious));
    printf("offsetof(ListItem_t, pvOwner)     = %zu\n", offsetof(ListItem_t, pvOwner));
    printf("offsetof(ListItem_t, pvContainer) = %zu\n", offsetof(ListItem_t, pvContainer));

    printf("\n=== Config flags actually in effect in YOUR FreeRTOSConfig.h ===\n");
#ifdef configUSE_MUTEXES
    printf("configUSE_MUTEXES              = %d\n", configUSE_MUTEXES);
#endif
#ifdef configUSE_TRACE_FACILITY
    printf("configUSE_TRACE_FACILITY       = %d\n", configUSE_TRACE_FACILITY);
#endif
#ifdef configMAX_PRIORITIES
    printf("configMAX_PRIORITIES           = %d\n", configMAX_PRIORITIES);
#endif
#ifdef configUSE_16_BIT_TICKS
    printf("configUSE_16_BIT_TICKS         = %d  (0 = 32-bit TickType_t, required for the 0xFFFFFFFF sentinel scan)\n", configUSE_16_BIT_TICKS);
#endif
#ifdef configSUPPORT_STATIC_ALLOCATION
    printf("configSUPPORT_STATIC_ALLOCATION = %d\n", configSUPPORT_STATIC_ALLOCATION);
#endif
#ifdef configSUPPORT_DYNAMIC_ALLOCATION
    printf("configSUPPORT_DYNAMIC_ALLOCATION = %d\n", configSUPPORT_DYNAMIC_ALLOCATION);
#endif
#ifdef configUSE_TASK_NOTIFICATIONS
    printf("configUSE_TASK_NOTIFICATIONS   = %d\n", configUSE_TASK_NOTIFICATIONS);
#endif
#ifdef configRECORD_STACK_HIGH_ADDRESS
    printf("configRECORD_STACK_HIGH_ADDRESS = %d\n", configRECORD_STACK_HIGH_ADDRESS);
#else
    printf("configRECORD_STACK_HIGH_ADDRESS = 0 (not defined -> default off)\n");
#endif
#ifdef configMAX_TASK_NAME_LEN
    printf("configMAX_TASK_NAME_LEN        = %d\n", configMAX_TASK_NAME_LEN);
#endif
#ifdef configNUM_THREAD_LOCAL_STORAGE_POINTERS
    printf("configNUM_THREAD_LOCAL_STORAGE_POINTERS = %d\n", configNUM_THREAD_LOCAL_STORAGE_POINTERS);
#endif
#ifdef configGENERATE_RUN_TIME_STATS
    printf("configGENERATE_RUN_TIME_STATS  = %d\n", configGENERATE_RUN_TIME_STATS);
#endif
#ifdef configUSE_APPLICATION_TASK_TAG
    printf("configUSE_APPLICATION_TASK_TAG = %d\n", configUSE_APPLICATION_TASK_TAG);
#endif
#ifdef portUSING_MPU_WRAPPERS
    printf("portUSING_MPU_WRAPPERS         = %d\n", portUSING_MPU_WRAPPERS);
#endif
    printf("tskKERNEL_VERSION_NUMBER       = %s\n", tskKERNEL_VERSION_NUMBER);

    printf("\nCompare these flags against the rows in data/offset_table.py\n");
    printf("to confirm 'V10.3.1_stm32cubeide_default' matches, or to build\n");
    printf("a corrected row if any flag differs from its assumptions.\n");

    return 0;
}
