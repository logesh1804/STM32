#include <stdint.h>
#include <string.h>

/* ─── RCC ─────────────────────────────────────────────── */
#define RCC_AHB1ENR  (*(volatile uint32_t*)0x40023830)
#define RCC_APB1ENR  (*(volatile uint32_t*)0x40023840)
#define RCC_APB2ENR  (*(volatile uint32_t*)0x40023844)

/* ─── GPIOA ───────────────────────────────────────────── */
#define GPIOA_MODER  (*(volatile uint32_t*)0x40020000)
#define GPIOA_AFRL   (*(volatile uint32_t*)0x40020020)
#define GPIOA_AFRH   (*(volatile uint32_t*)0x40020024)

/* ─── USART1 (ESP32) ──────────────────────────────────── */
#define USART1_SR    (*(volatile uint32_t*)0x40011000)
#define USART1_DR    (*(volatile uint32_t*)0x40011004)
#define USART1_BRR   (*(volatile uint32_t*)0x40011008)
#define USART1_CR1   (*(volatile uint32_t*)0x4001100C)

/* ─── USART2 (PC / TeraTerm) ──────────────────────────── */
#define USART2_SR    (*(volatile uint32_t*)0x40004400)
#define USART2_DR    (*(volatile uint32_t*)0x40004404)
#define USART2_BRR   (*(volatile uint32_t*)0x40004408)
#define USART2_CR1   (*(volatile uint32_t*)0x4000440C)

/* ─── NVIC ────────────────────────────────────────────── */
#define NVIC_ISER1   (*(volatile uint32_t*)0xE000E104)  /* IRQs 32-63 */
/* USART1 = IRQ37 → bit 5 of ISER1 */
/* USART2 = IRQ38 → bit 6 of ISER1 */

/* ═══════════════════════════════════════════════════════
   Ring buffer — 256 bytes, power-of-2 so wrap is a mask
   ═══════════════════════════════════════════════════════ */
#define RB_SIZE  256
#define RB_MASK  (RB_SIZE - 1)

typedef struct {
    volatile char     buf[RB_SIZE];
    volatile uint16_t head;   /* ISR writes here */
    volatile uint16_t tail;   /* main reads here */
} RingBuf;

static RingBuf rb_u1;   /* bytes arriving from ESP32  */
static RingBuf rb_u2;   /* bytes arriving from PC     */

static inline void rb_push(RingBuf *rb, char c)
{
    uint16_t next = (rb->head + 1) & RB_MASK;
    if (next != rb->tail)           /* drop if full */
    {
        rb->buf[rb->head] = c;
        rb->head = next;
    }
}

static inline int rb_pop(RingBuf *rb, char *c)
{
    if (rb->tail == rb->head) return 0;
    *c = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) & RB_MASK;
    return 1;
}

static inline int rb_available(const RingBuf *rb)
{
    return rb->head != rb->tail;
}

/* ═══════════════════════════════════════════════════════
   Line buffers – assembled from ring buffers in main loop
   ═══════════════════════════════════════════════════════ */
static char line_pc[128];
static char line_esp[128];
static uint8_t idx_pc  = 0;
static uint8_t idx_esp = 0;
static uint8_t line_pc_ready  = 0;
static uint8_t line_esp_ready = 0;

/* ═══════════════════════════════════════════════════════
   GPIO init
   ═══════════════════════════════════════════════════════ */
static void GPIO_Init(void)
{
    RCC_AHB1ENR |= (1 << 0);   /* GPIOA clock */

    /* PA2 = USART2_TX, PA3 = USART2_RX  →  AF7 */
    GPIOA_MODER &= ~((3u << 4) | (3u << 6));
    GPIOA_MODER |=  ((2u << 4) | (2u << 6));
    GPIOA_AFRL  &= ~((0xFu << 8) | (0xFu << 12));
    GPIOA_AFRL  |=  ((7u  << 8) | (7u  << 12));

    /* PA9 = USART1_TX, PA10 = USART1_RX →  AF7 */
    GPIOA_MODER &= ~((3u << 18) | (3u << 20));
    GPIOA_MODER |=  ((2u << 18) | (2u << 20));
    GPIOA_AFRH  &= ~((0xFu << 4) | (0xFu << 8));
    GPIOA_AFRH  |=  ((7u  << 4) | (7u  << 8));
}

/* ═══════════════════════════════════════════════════════
   USART helpers (TX only — RX is interrupt-driven)
   ═══════════════════════════════════════════════════════ */
static void USART1_WriteChar(char c)
{
    while (!(USART1_SR & (1 << 7)));
    USART1_DR = (uint32_t)c;
}
static void USART1_Print(const char *s)
{
    while (*s) USART1_WriteChar(*s++);
}

static void USART2_WriteChar(char c)
{
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = (uint32_t)c;
}
static void USART2_Print(const char *s)
{
    while (*s) USART2_WriteChar(*s++);
}

/* ═══════════════════════════════════════════════════════
   USART init — enable RXNE interrupt (bit 5 of CR1)
   ═══════════════════════════════════════════════════════ */
static void USART1_Init(void)
{
    RCC_APB2ENR |= (1 << 4);   /* USART1 clock */
    USART1_CR1   = 0;
    USART1_BRR   = 0x0683;     /* 9600 @ 16 MHz */
    USART1_CR1   = (1 << 13) | /* UE  */
                   (1 << 5)  | /* RXNEIE — RX interrupt enable */
                   (1 << 3)  | /* TE  */
                   (1 << 2);   /* RE  */

    /* Enable USART1 IRQ (IRQ37 = bit 5 of NVIC_ISER1) */
    NVIC_ISER1 |= (1u << 5);
}

static void USART2_Init(void)
{
    RCC_APB1ENR |= (1 << 17);  /* USART2 clock */
    USART2_CR1   = 0;
    USART2_BRR   = 0x0683;     /* 9600 @ 16 MHz */
    USART2_CR1   = (1 << 13) |
                   (1 << 5)  | /* RXNEIE */
                   (1 << 3)  |
                   (1 << 2);

    /* Enable USART2 IRQ (IRQ38 = bit 6 of NVIC_ISER1) */
    NVIC_ISER1 |= (1u << 6);
}

/* ═══════════════════════════════════════════════════════
   ISR — push received byte into ring buffer
   Names must match the vector table in startup_stm32f411xe.s
   ═══════════════════════════════════════════════════════ */
void USART1_IRQHandler(void)
{
    if (USART1_SR & (1 << 5))          /* RXNE */
    {
        char c = (char)USART1_DR;      /* clears RXNE */
        rb_push(&rb_u1, c);
    }
}

void USART2_IRQHandler(void)
{
    if (USART2_SR & (1 << 5))          /* RXNE */
    {
        char c = (char)USART2_DR;
        rb_push(&rb_u2, c);
    }
}

/* ═══════════════════════════════════════════════════════
   Main – non-blocking line assembler + router
   ═══════════════════════════════════════════════════════ */
int main(void)
{
    GPIO_Init();
    USART1_Init();
    USART2_Init();

    /* Small startup delay */
    for (volatile uint32_t i = 0; i < 400000; i++);

    USART2_Print("\r\n=============================\r\n");
    USART2_Print(" STM32F411RE  Full-Duplex Bridge\r\n");
    USART2_Print(" USART2 <-> PC   |   USART1 <-> ESP32\r\n");
    USART2_Print("=============================\r\n");

    while (1)
    {
        char c;

        /* ── Drain PC ring buffer ─────────────────────────── */
        while (!line_pc_ready && rb_pop(&rb_u2, &c))
        {
            /* Echo back to PC */
            USART2_WriteChar(c);

            if (c == '\r' || c == '\n')
            {
                if (idx_pc > 0)
                {
                    line_pc[idx_pc] = '\0';
                    line_pc_ready   = 1;
                    idx_pc          = 0;
                    USART2_Print("\r\n");
                }
            }
            else if (idx_pc < (uint8_t)(sizeof(line_pc) - 1))
            {
                line_pc[idx_pc++] = c;
            }
        }

        /* ── Drain ESP32 ring buffer ──────────────────────── */
        while (!line_esp_ready && rb_pop(&rb_u1, &c))
        {
            if (c == '\r' || c == '\n')
            {
                if (idx_esp > 0)
                {
                    line_esp[idx_esp] = '\0';
                    line_esp_ready    = 1;
                    idx_esp           = 0;
                }
            }
            else if (idx_esp < (uint8_t)(sizeof(line_esp) - 1))
            {
                line_esp[idx_esp++] = c;
            }
        }

        /* ── Forward PC line → ESP32 ──────────────────────── */
        if (line_pc_ready)
        {
            USART2_Print("[STM→ESP32] ");
            USART2_Print(line_pc);
            USART2_Print("\r\n");

            USART1_Print(line_pc);
            USART1_WriteChar('\n');

            line_pc_ready = 0;
        }

        /* ── Forward ESP32 line → PC ──────────────────────── */
        if (line_esp_ready)
        {
            USART2_Print("[ESP32→PC] ");
            USART2_Print(line_esp);
            USART2_Print("\r\n");

            line_esp_ready = 0;
        }
    }
}
