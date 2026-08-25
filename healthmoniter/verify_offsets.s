	.cpu cortex-m4
	.arch armv7e-m
	.fpu fpv4-sp-d16
	.eabi_attribute 27, 1
	.eabi_attribute 28, 1
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 6
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"verify_offsets.c"
	.text
	.section	.rodata
	.align	2
.LC0:
	.ascii	"=== List_t / ListItem_t layout (should match data/o"
	.ascii	"ffset_table.py LIST_T_LAYOUT) ===\000"
	.align	2
.LC1:
	.ascii	"sizeof(List_t)          = %zu\012\000"
	.align	2
.LC2:
	.ascii	"sizeof(ListItem_t)      = %zu\012\000"
	.align	2
.LC3:
	.ascii	"offsetof(List_t, uxNumberOfItems) = %zu\012\000"
	.align	2
.LC4:
	.ascii	"offsetof(List_t, pxIndex)         = %zu\012\000"
	.align	2
.LC5:
	.ascii	"offsetof(List_t, xListEnd)        = %zu\012\000"
	.align	2
.LC6:
	.ascii	"offsetof(ListItem_t, xItemValue)  = %zu\012\000"
	.align	2
.LC7:
	.ascii	"offsetof(ListItem_t, pxNext)      = %zu\012\000"
	.align	2
.LC8:
	.ascii	"offsetof(ListItem_t, pxPrevious)  = %zu\012\000"
	.align	2
.LC9:
	.ascii	"offsetof(ListItem_t, pvOwner)     = %zu\012\000"
	.align	2
.LC10:
	.ascii	"offsetof(ListItem_t, pvContainer) = %zu\012\000"
	.align	2
.LC11:
	.ascii	"\012=== Config flags actually in effect in YOUR Fre"
	.ascii	"eRTOSConfig.h ===\000"
	.align	2
.LC12:
	.ascii	"configUSE_MUTEXES              = %d\012\000"
	.align	2
.LC13:
	.ascii	"configUSE_TRACE_FACILITY       = %d\012\000"
	.align	2
.LC14:
	.ascii	"configMAX_PRIORITIES           = %d\012\000"
	.align	2
.LC15:
	.ascii	"configUSE_16_BIT_TICKS         = %d  (0 = 32-bit Ti"
	.ascii	"ckType_t, required for the 0xFFFFFFFF sentinel scan"
	.ascii	")\012\000"
	.align	2
.LC16:
	.ascii	"configSUPPORT_STATIC_ALLOCATION = %d\012\000"
	.align	2
.LC17:
	.ascii	"configSUPPORT_DYNAMIC_ALLOCATION = %d\012\000"
	.align	2
.LC18:
	.ascii	"configUSE_TASK_NOTIFICATIONS   = %d\012\000"
	.align	2
.LC19:
	.ascii	"configRECORD_STACK_HIGH_ADDRESS = %d\012\000"
	.align	2
.LC20:
	.ascii	"configMAX_TASK_NAME_LEN        = %d\012\000"
	.align	2
.LC21:
	.ascii	"configNUM_THREAD_LOCAL_STORAGE_POINTERS = %d\012\000"
	.align	2
.LC22:
	.ascii	"configGENERATE_RUN_TIME_STATS  = %d\012\000"
	.align	2
.LC23:
	.ascii	"configUSE_APPLICATION_TASK_TAG = %d\012\000"
	.align	2
.LC24:
	.ascii	"portUSING_MPU_WRAPPERS         = %d\012\000"
	.align	2
.LC25:
	.ascii	"V10.3.1\000"
	.align	2
.LC26:
	.ascii	"tskKERNEL_VERSION_NUMBER       = %s\012\000"
	.align	2
.LC27:
	.ascii	"\012Compare these flags against the rows in data/of"
	.ascii	"fset_table.py\000"
	.align	2
.LC28:
	.ascii	"to confirm 'V10.3.1_stm32cubeide_default' matches, "
	.ascii	"or to build\000"
	.align	2
.LC29:
	.ascii	"a corrected row if any flag differs from its assump"
	.ascii	"tions.\000"
	.text
	.align	1
	.global	main
	.syntax unified
	.thumb
	.thumb_func
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	add	r7, sp, #0
	ldr	r0, .L3
	bl	puts
	movs	r1, #20
	ldr	r0, .L3+4
	bl	printf
	movs	r1, #20
	ldr	r0, .L3+8
	bl	printf
	movs	r1, #0
	ldr	r0, .L3+12
	bl	printf
	movs	r1, #4
	ldr	r0, .L3+16
	bl	printf
	movs	r1, #8
	ldr	r0, .L3+20
	bl	printf
	movs	r1, #0
	ldr	r0, .L3+24
	bl	printf
	movs	r1, #4
	ldr	r0, .L3+28
	bl	printf
	movs	r1, #8
	ldr	r0, .L3+32
	bl	printf
	movs	r1, #12
	ldr	r0, .L3+36
	bl	printf
	movs	r1, #16
	ldr	r0, .L3+40
	bl	printf
	ldr	r0, .L3+44
	bl	puts
	movs	r1, #1
	ldr	r0, .L3+48
	bl	printf
	movs	r1, #1
	ldr	r0, .L3+52
	bl	printf
	movs	r1, #56
	ldr	r0, .L3+56
	bl	printf
	movs	r1, #0
	ldr	r0, .L3+60
	bl	printf
	movs	r1, #1
	ldr	r0, .L3+64
	bl	printf
	movs	r1, #1
	ldr	r0, .L3+68
	bl	printf
	movs	r1, #1
	ldr	r0, .L3+72
	bl	printf
	movs	r1, #0
	ldr	r0, .L3+76
	bl	printf
	movs	r1, #16
	ldr	r0, .L3+80
	bl	printf
	movs	r1, #0
	ldr	r0, .L3+84
	bl	printf
	movs	r1, #0
	ldr	r0, .L3+88
	bl	printf
	movs	r1, #0
	ldr	r0, .L3+92
	bl	printf
	movs	r1, #0
	ldr	r0, .L3+96
	bl	printf
	ldr	r1, .L3+100
	ldr	r0, .L3+104
	bl	printf
	ldr	r0, .L3+108
	bl	puts
	ldr	r0, .L3+112
	bl	puts
	ldr	r0, .L3+116
	bl	puts
	movs	r3, #0
	mov	r0, r3
	pop	{r7, pc}
.L4:
	.align	2
.L3:
	.word	.LC0
	.word	.LC1
	.word	.LC2
	.word	.LC3
	.word	.LC4
	.word	.LC5
	.word	.LC6
	.word	.LC7
	.word	.LC8
	.word	.LC9
	.word	.LC10
	.word	.LC11
	.word	.LC12
	.word	.LC13
	.word	.LC14
	.word	.LC15
	.word	.LC16
	.word	.LC17
	.word	.LC18
	.word	.LC19
	.word	.LC20
	.word	.LC21
	.word	.LC22
	.word	.LC23
	.word	.LC24
	.word	.LC25
	.word	.LC26
	.word	.LC27
	.word	.LC28
	.word	.LC29
	.size	main, .-main
	.ident	"GCC: (GNU Tools for STM32 13.3.rel1.20240926-1715) 13.3.1 20240614"
