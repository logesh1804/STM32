#include <stdint.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════
   RCC
   ═══════════════════════════════════════════════════════ */
#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)
#define RCC_APB1ENR   (*(volatile uint32_t*)0x40023840)
#define RCC_APB2ENR   (*(volatile uint32_t*)0x40023844)

/* ═══════════════════════════════════════════════════════
   GPIOA  (USART2 PA2/PA3, USART1 PA9/PA10, SPI1 PA5/PA7, CS PA4)
   ═══════════════════════════════════════════════════════ */
#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)
#define GPIOA_OTYPER  (*(volatile uint32_t*)0x40020004)
#define GPIOA_OSPEEDR (*(volatile uint32_t*)0x40020008)
#define GPIOA_PUPDR   (*(volatile uint32_t*)0x4002000C)
#define GPIOA_ODR     (*(volatile uint32_t*)0x40020014)
#define GPIOA_BSRR    (*(volatile uint32_t*)0x40020018)
#define GPIOA_AFRL    (*(volatile uint32_t*)0x40020020)
#define GPIOA_AFRH    (*(volatile uint32_t*)0x40020024)

/* ═══════════════════════════════════════════════════════
   GPIOB  (I2C1 PB6/PB7, TFT DC PB0, RST PB1, LED PB2)
   ═══════════════════════════════════════════════════════ */
#define GPIOB_MODER   (*(volatile uint32_t*)0x40020400)
#define GPIOB_OTYPER  (*(volatile uint32_t*)0x40020404)
#define GPIOB_OSPEEDR (*(volatile uint32_t*)0x40020408)
#define GPIOB_PUPDR   (*(volatile uint32_t*)0x4002040C)
#define GPIOB_ODR     (*(volatile uint32_t*)0x40020414)
#define GPIOB_BSRR    (*(volatile uint32_t*)0x40020418)
#define GPIOB_AFRL    (*(volatile uint32_t*)0x40020420)

/* ═══════════════════════════════════════════════════════
   USART1 (ESP32)  USART2 (PC)
   ═══════════════════════════════════════════════════════ */
#define USART1_SR     (*(volatile uint32_t*)0x40011000)
#define USART1_DR     (*(volatile uint32_t*)0x40011004)
#define USART1_BRR    (*(volatile uint32_t*)0x40011008)
#define USART1_CR1    (*(volatile uint32_t*)0x4001100C)

#define USART2_SR     (*(volatile uint32_t*)0x40004400)
#define USART2_DR     (*(volatile uint32_t*)0x40004404)
#define USART2_BRR    (*(volatile uint32_t*)0x40004408)
#define USART2_CR1    (*(volatile uint32_t*)0x4000440C)

/* ═══════════════════════════════════════════════════════
   SPI1  (TFT ILI9341)
   ═══════════════════════════════════════════════════════ */
#define SPI1_CR1      (*(volatile uint32_t*)0x40013000)
#define SPI1_CR2      (*(volatile uint32_t*)0x40013004)
#define SPI1_SR       (*(volatile uint32_t*)0x40013008)
#define SPI1_DR       (*(volatile uint32_t*)0x4001300C)

/* ═══════════════════════════════════════════════════════
   I2C1  (LCD 16x2 PCF8574)
   ═══════════════════════════════════════════════════════ */
#define I2C1_CR1      (*(volatile uint32_t*)0x40005400)
#define I2C1_CR2      (*(volatile uint32_t*)0x40005404)
#define I2C1_DR       (*(volatile uint32_t*)0x40005410)
#define I2C1_SR1      (*(volatile uint32_t*)0x40005414)
#define I2C1_SR2      (*(volatile uint32_t*)0x40005418)
#define I2C1_CCR      (*(volatile uint32_t*)0x4005041C)
#define I2C1_TRISE    (*(volatile uint32_t*)0x40005420)

/* ═══════════════════════════════════════════════════════
   NVIC
   ═══════════════════════════════════════════════════════ */
#define NVIC_ISER1    (*(volatile uint32_t*)0xE000E104)

/* ═══════════════════════════════════════════════════════
   TFT GPIO macros
   PA4 = TFT_CS   PB0 = TFT_DC   PB1 = TFT_RST   PB2 = TFT_LED
   ═══════════════════════════════════════════════════════ */
#define TFT_CS_LOW()   (GPIOA_BSRR = (1u<<(4+16)))
#define TFT_CS_HIGH()  (GPIOA_BSRR = (1u<<4))
#define TFT_DC_LOW()   (GPIOB_BSRR = (1u<<(0+16)))
#define TFT_DC_HIGH()  (GPIOB_BSRR = (1u<<0))
#define TFT_RST_LOW()  (GPIOB_BSRR = (1u<<(1+16)))
#define TFT_RST_HIGH() (GPIOB_BSRR = (1u<<1))
#define TFT_LED_ON()   (GPIOB_BSRR = (1u<<2))

/* ═══════════════════════════════════════════════════════
   ILI9341 commands
   ═══════════════════════════════════════════════════════ */
#define ILI9341_SWRESET   0x01
#define ILI9341_SLPOUT    0x11
#define ILI9341_DISPON    0x29
#define ILI9341_CASET     0x2A
#define ILI9341_PASET     0x2B
#define ILI9341_RAMWR     0x2C
#define ILI9341_MADCTL    0x36
#define ILI9341_COLMOD    0x3A
#define ILI9341_FRMCTR1   0xB1
#define ILI9341_PWCTR1    0xC0
#define ILI9341_PWCTR2    0xC1
#define ILI9341_VMCTR1    0xC5
#define ILI9341_VMCTR2    0xC7
#define ILI9341_GMCTRP1   0xE0
#define ILI9341_GMCTRN1   0xE1

/* RGB565 colors */
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_CYAN    0x07FF
#define COLOR_YELLOW  0xFFE0
#define COLOR_GREEN   0x07E0
#define COLOR_RED     0xF800
#define COLOR_BLUE    0x001F
#define COLOR_ORANGE  0xFD20
#define COLOR_GRAY    0x8410

/* PCF8574 I2C LCD */
#define LCD_I2C_ADDR  0x27
#define LCD_BL        0x08
#define LCD_EN        0x04
#define LCD_RS        0x01

/* ═══════════════════════════════════════════════════════
   Ring buffers for full-duplex UART
   ═══════════════════════════════════════════════════════ */
#define RB_SIZE  256
#define RB_MASK  (RB_SIZE-1)

typedef struct {
    volatile char     buf[RB_SIZE];
    volatile uint16_t head, tail;
} RingBuf;

static RingBuf rb_u1, rb_u2;

static inline void rb_push(RingBuf *r, char c) {
    uint16_t n = (r->head+1)&RB_MASK;
    if(n!=r->tail){ r->buf[r->head]=c; r->head=n; }
}
static inline int rb_pop(RingBuf *r, char *c) {
    if(r->tail==r->head) return 0;
    *c=r->buf[r->tail]; r->tail=(r->tail+1)&RB_MASK; return 1;
}

static char line_pc[128],  line_esp[128];
static uint8_t idx_pc=0,   idx_esp=0;
static uint8_t lpc_rdy=0,  lesp_rdy=0;

/* ═══════════════════════════════════════════════════════
   Font 5×7 — ASCII 32-127  (compact bitmap)
   Each char = 5 bytes (columns), rows are bits 0-6
   ═══════════════════════════════════════════════════════ */
static const uint8_t font5x7[][5] = {
  {0x00,0x00,0x00,0x00,0x00}, /* 32 space */
  {0x00,0x00,0x5F,0x00,0x00}, /* 33 ! */
  {0x00,0x07,0x00,0x07,0x00}, /* 34 " */
  {0x14,0x7F,0x14,0x7F,0x14}, /* 35 # */
  {0x24,0x2A,0x7F,0x2A,0x12}, /* 36 $ */
  {0x23,0x13,0x08,0x64,0x62}, /* 37 % */
  {0x36,0x49,0x55,0x22,0x50}, /* 38 & */
  {0x00,0x05,0x03,0x00,0x00}, /* 39 ' */
  {0x00,0x1C,0x22,0x41,0x00}, /* 40 ( */
  {0x00,0x41,0x22,0x1C,0x00}, /* 41 ) */
  {0x14,0x08,0x3E,0x08,0x14}, /* 42 * */
  {0x08,0x08,0x3E,0x08,0x08}, /* 43 + */
  {0x00,0x50,0x30,0x00,0x00}, /* 44 , */
  {0x08,0x08,0x08,0x08,0x08}, /* 45 - */
  {0x00,0x60,0x60,0x00,0x00}, /* 46 . */
  {0x20,0x10,0x08,0x04,0x02}, /* 47 / */
  {0x3E,0x51,0x49,0x45,0x3E}, /* 48 0 */
  {0x00,0x42,0x7F,0x40,0x00}, /* 49 1 */
  {0x42,0x61,0x51,0x49,0x46}, /* 50 2 */
  {0x21,0x41,0x45,0x4B,0x31}, /* 51 3 */
  {0x18,0x14,0x12,0x7F,0x10}, /* 52 4 */
  {0x27,0x45,0x45,0x45,0x39}, /* 53 5 */
  {0x3C,0x4A,0x49,0x49,0x30}, /* 54 6 */
  {0x01,0x71,0x09,0x05,0x03}, /* 55 7 */
  {0x36,0x49,0x49,0x49,0x36}, /* 56 8 */
  {0x06,0x49,0x49,0x29,0x1E}, /* 57 9 */
  {0x00,0x36,0x36,0x00,0x00}, /* 58 : */
  {0x00,0x56,0x36,0x00,0x00}, /* 59 ; */
  {0x08,0x14,0x22,0x41,0x00}, /* 60 < */
  {0x14,0x14,0x14,0x14,0x14}, /* 61 = */
  {0x00,0x41,0x22,0x14,0x08}, /* 62 > */
  {0x02,0x01,0x51,0x09,0x06}, /* 63 ? */
  {0x32,0x49,0x79,0x41,0x3E}, /* 64 @ */
  {0x7E,0x11,0x11,0x11,0x7E}, /* 65 A */
  {0x7F,0x49,0x49,0x49,0x36}, /* 66 B */
  {0x3E,0x41,0x41,0x41,0x22}, /* 67 C */
  {0x7F,0x41,0x41,0x22,0x1C}, /* 68 D */
  {0x7F,0x49,0x49,0x49,0x41}, /* 69 E */
  {0x7F,0x09,0x09,0x09,0x01}, /* 70 F */
  {0x3E,0x41,0x49,0x49,0x7A}, /* 71 G */
  {0x7F,0x08,0x08,0x08,0x7F}, /* 72 H */
  {0x00,0x41,0x7F,0x41,0x00}, /* 73 I */
  {0x20,0x40,0x41,0x3F,0x01}, /* 74 J */
  {0x7F,0x08,0x14,0x22,0x41}, /* 75 K */
  {0x7F,0x40,0x40,0x40,0x40}, /* 76 L */
  {0x7F,0x02,0x04,0x02,0x7F}, /* 77 M */
  {0x7F,0x04,0x08,0x10,0x7F}, /* 78 N */
  {0x3E,0x41,0x41,0x41,0x3E}, /* 79 O */
  {0x7F,0x09,0x09,0x09,0x06}, /* 80 P */
  {0x3E,0x41,0x51,0x21,0x5E}, /* 81 Q */
  {0x7F,0x09,0x19,0x29,0x46}, /* 82 R */
  {0x46,0x49,0x49,0x49,0x31}, /* 83 S */
  {0x01,0x01,0x7F,0x01,0x01}, /* 84 T */
  {0x3F,0x40,0x40,0x40,0x3F}, /* 85 U */
  {0x1F,0x20,0x40,0x20,0x1F}, /* 86 V */
  {0x3F,0x40,0x38,0x40,0x3F}, /* 87 W */
  {0x63,0x14,0x08,0x14,0x63}, /* 88 X */
  {0x07,0x08,0x70,0x08,0x07}, /* 89 Y */
  {0x61,0x51,0x49,0x45,0x43}, /* 90 Z */
  {0x00,0x7F,0x41,0x41,0x00}, /* 91 [ */
  {0x02,0x04,0x08,0x10,0x20}, /* 92 \ */
  {0x00,0x41,0x41,0x7F,0x00}, /* 93 ] */
  {0x04,0x02,0x01,0x02,0x04}, /* 94 ^ */
  {0x40,0x40,0x40,0x40,0x40}, /* 95 _ */
  {0x00,0x01,0x02,0x04,0x00}, /* 96 ` */
  {0x20,0x54,0x54,0x54,0x78}, /* 97 a */
  {0x7F,0x48,0x44,0x44,0x38}, /* 98 b */
  {0x38,0x44,0x44,0x44,0x20}, /* 99 c */
  {0x38,0x44,0x44,0x48,0x7F}, /* 100 d */
  {0x38,0x54,0x54,0x54,0x18}, /* 101 e */
  {0x08,0x7E,0x09,0x01,0x02}, /* 102 f */
  {0x0C,0x52,0x52,0x52,0x3E}, /* 103 g */
  {0x7F,0x08,0x04,0x04,0x78}, /* 104 h */
  {0x00,0x44,0x7D,0x40,0x00}, /* 105 i */
  {0x20,0x40,0x44,0x3D,0x00}, /* 106 j */
  {0x7F,0x10,0x28,0x44,0x00}, /* 107 k */
  {0x00,0x41,0x7F,0x40,0x00}, /* 108 l */
  {0x7C,0x04,0x18,0x04,0x78}, /* 109 m */
  {0x7C,0x08,0x04,0x04,0x78}, /* 110 n */
  {0x38,0x44,0x44,0x44,0x38}, /* 111 o */
  {0x7C,0x14,0x14,0x14,0x08}, /* 112 p */
  {0x08,0x14,0x14,0x18,0x7C}, /* 113 q */
  {0x7C,0x08,0x04,0x04,0x08}, /* 114 r */
  {0x48,0x54,0x54,0x54,0x20}, /* 115 s */
  {0x04,0x3F,0x44,0x40,0x20}, /* 116 t */
  {0x3C,0x40,0x40,0x20,0x7C}, /* 117 u */
  {0x1C,0x20,0x40,0x20,0x1C}, /* 118 v */
  {0x3C,0x40,0x30,0x40,0x3C}, /* 119 w */
  {0x44,0x28,0x10,0x28,0x44}, /* 120 x */
  {0x0C,0x50,0x50,0x50,0x3C}, /* 121 y */
  {0x44,0x64,0x54,0x4C,0x44}, /* 122 z */
  {0x00,0x08,0x36,0x41,0x00}, /* 123 { */
  {0x00,0x00,0x7F,0x00,0x00}, /* 124 | */
  {0x00,0x41,0x36,0x08,0x00}, /* 125 } */
  {0x10,0x08,0x08,0x10,0x08}, /* 126 ~ */
  {0x00,0x00,0x00,0x00,0x00}, /* 127 DEL */
};

/* ═══════════════════════════════════════════════════════
   Delay
   ═══════════════════════════════════════════════════════ */
static void delay_ms(uint32_t ms) {
    for(uint32_t i=0;i<ms;i++)
        for(volatile uint32_t j=0;j<4000;j++);
}
static void delay_us(uint32_t us) {
    for(volatile uint32_t i=0;i<us*4;i++);
}

/* ═══════════════════════════════════════════════════════
   GPIO Init
   ═══════════════════════════════════════════════════════ */
static void GPIO_Init(void)
{
    /* GPIOA + GPIOB clocks */
    RCC_AHB1ENR |= (1u<<0)|(1u<<1);

    /* ── USART2 PA2=TX PA3=RX AF7 ── */
    GPIOA_MODER &= ~((3u<<4)|(3u<<6));
    GPIOA_MODER |=  ((2u<<4)|(2u<<6));
    GPIOA_AFRL  &= ~((0xFu<<8)|(0xFu<<12));
    GPIOA_AFRL  |=  ((7u<<8)|(7u<<12));

    /* ── USART1 PA9=TX PA10=RX AF7 ── */
    GPIOA_MODER &= ~((3u<<18)|(3u<<20));
    GPIOA_MODER |=  ((2u<<18)|(2u<<20));
    GPIOA_AFRH  &= ~((0xFu<<4)|(0xFu<<8));
    GPIOA_AFRH  |=  ((7u<<4)|(7u<<8));

    /* ── SPI1 PA5=SCK PA7=MOSI AF5 ── */
    GPIOA_MODER &= ~((3u<<10)|(3u<<14));
    GPIOA_MODER |=  ((2u<<10)|(2u<<14));
    GPIOA_OSPEEDR |= (3u<<10)|(3u<<14);  /* high speed */
    GPIOA_AFRL  &= ~((0xFu<<20)|(0xFu<<28));
    GPIOA_AFRL  |=  ((5u<<20)|(5u<<28));

    /* ── PA4 = TFT CS — GPIO output ── */
    GPIOA_MODER &= ~(3u<<8);
    GPIOA_MODER |=  (1u<<8);
    GPIOA_BSRR   =  (1u<<4);   /* CS=HIGH */

    /* ── PB0=DC PB1=RST PB2=LED — GPIO outputs ── */
    GPIOB_MODER &= ~((3u<<0)|(3u<<2)|(3u<<4));
    GPIOB_MODER |=  ((1u<<0)|(1u<<2)|(1u<<4));
    GPIOB_OSPEEDR |= (3u<<0)|(3u<<2)|(3u<<4);

    /* ── I2C1 PB6=SCL PB7=SDA AF4 open-drain ── */
    GPIOB_MODER  &= ~((3u<<12)|(3u<<14));
    GPIOB_MODER  |=  ((2u<<12)|(2u<<14));
    GPIOB_OTYPER |=  (1u<<6)|(1u<<7);
    GPIOB_OSPEEDR|=  (3u<<12)|(3u<<14);
    GPIOB_AFRL   &= ~((0xFu<<24)|(0xFu<<28));
    GPIOB_AFRL   |=  ((4u<<24)|(4u<<28));
}

/* ═══════════════════════════════════════════════════════
   USART1 / USART2 with RXNE interrupts
   ═══════════════════════════════════════════════════════ */
static void USART1_Init(void) {
    RCC_APB2ENR |= (1u<<4);
    USART1_CR1 = 0;
    USART1_BRR = 0x0683;
    USART1_CR1 = (1u<<13)|(1u<<5)|(1u<<3)|(1u<<2);
    NVIC_ISER1 |= (1u<<5);   /* IRQ37 */
}
static void USART2_Init(void) {
    RCC_APB1ENR |= (1u<<17);
    USART2_CR1 = 0;
    USART2_BRR = 0x0683;
    USART2_CR1 = (1u<<13)|(1u<<5)|(1u<<3)|(1u<<2);
    NVIC_ISER1 |= (1u<<6);   /* IRQ38 */
}
static void USART1_WriteChar(char c) {
    while(!(USART1_SR&(1u<<7))); USART1_DR=(uint32_t)c;
}
static void USART1_Print(const char *s) { while(*s) USART1_WriteChar(*s++); }
static void USART2_WriteChar(char c) {
    while(!(USART2_SR&(1u<<7))); USART2_DR=(uint32_t)c;
}
static void USART2_Print(const char *s) { while(*s) USART2_WriteChar(*s++); }

void USART1_IRQHandler(void) {
    if(USART1_SR&(1u<<5)) rb_push(&rb_u1,(char)USART1_DR);
}
void USART2_IRQHandler(void) {
    if(USART2_SR&(1u<<5)) rb_push(&rb_u2,(char)USART2_DR);
}

/* ═══════════════════════════════════════════════════════
   SPI1  — hardware, CPOL=0 CPHA=0, MSB first
   APB2=16MHz, BR=001 → 8MHz for TFT
   ═══════════════════════════════════════════════════════ */
static void SPI1_Init(void) {
    RCC_APB2ENR |= (1u<<12);  /* SPI1 clock */
    SPI1_CR1 = 0;
    SPI1_CR1 = (1u<<2)   /* MSTR */
             | (0u<<3)   /* BR = 000 → fPCLK/2 = 8MHz */
             | (0u<<0)   /* CPOL=0 */
             | (0u<<1)   /* CPHA=0 */
             | (1u<<9)   /* SSM software slave management */
             | (1u<<8);  /* SSI */
    SPI1_CR1 |= (1u<<6); /* SPE enable */
}

static void SPI1_WriteByte(uint8_t d) {
    while(!(SPI1_SR&(1u<<1)));   /* TXE */
    SPI1_DR = d;
    while(SPI1_SR&(1u<<7));      /* BSY */
}

/* ═══════════════════════════════════════════════════════
   ILI9341 low-level
   ═══════════════════════════════════════════════════════ */
static void TFT_WriteCmd(uint8_t cmd) {
    TFT_DC_LOW();
    TFT_CS_LOW();
    SPI1_WriteByte(cmd);
    TFT_CS_HIGH();
}
static void TFT_WriteData8(uint8_t d) {
    TFT_DC_HIGH();
    TFT_CS_LOW();
    SPI1_WriteByte(d);
    TFT_CS_HIGH();
}
static void TFT_WriteData16(uint16_t d) {
    TFT_DC_HIGH();
    TFT_CS_LOW();
    SPI1_WriteByte(d>>8);
    SPI1_WriteByte(d&0xFF);
    TFT_CS_HIGH();
}

/* ═══════════════════════════════════════════════════════
   ILI9341 Init sequence
   ═══════════════════════════════════════════════════════ */
static void TFT_Init(void)
{
    /* Hardware reset */
    TFT_RST_LOW();  delay_ms(20);
    TFT_RST_HIGH(); delay_ms(150);

    TFT_WriteCmd(ILI9341_SWRESET); delay_ms(150);
    TFT_WriteCmd(ILI9341_SLPOUT);  delay_ms(500);

    TFT_WriteCmd(ILI9341_FRMCTR1);
    TFT_WriteData8(0x01); TFT_WriteData8(0x2C); TFT_WriteData8(0x2D);

    TFT_WriteCmd(ILI9341_PWCTR1);  TFT_WriteData8(0x23);
    TFT_WriteCmd(ILI9341_PWCTR2);  TFT_WriteData8(0x10);
    TFT_WriteCmd(ILI9341_VMCTR1);  TFT_WriteData8(0x3E); TFT_WriteData8(0x28);
    TFT_WriteCmd(ILI9341_VMCTR2);  TFT_WriteData8(0x86);

    /* Landscape 320×240 — MADCTL: MX+MV */
    TFT_WriteCmd(ILI9341_MADCTL);  TFT_WriteData8(0x48);  /* portrait 240×320 */

    TFT_WriteCmd(ILI9341_COLMOD);  TFT_WriteData8(0x55);  /* 16-bit RGB565 */

    TFT_WriteCmd(ILI9341_GMCTRP1);
    TFT_WriteData8(0x0F); TFT_WriteData8(0x31); TFT_WriteData8(0x2B);
    TFT_WriteData8(0x0C); TFT_WriteData8(0x0E); TFT_WriteData8(0x08);
    TFT_WriteData8(0x4E); TFT_WriteData8(0xF1); TFT_WriteData8(0x37);
    TFT_WriteData8(0x07); TFT_WriteData8(0x10); TFT_WriteData8(0x03);
    TFT_WriteData8(0x0E); TFT_WriteData8(0x09); TFT_WriteData8(0x00);

    TFT_WriteCmd(ILI9341_GMCTRN1);
    TFT_WriteData8(0x00); TFT_WriteData8(0x0E); TFT_WriteData8(0x14);
    TFT_WriteData8(0x03); TFT_WriteData8(0x11); TFT_WriteData8(0x07);
    TFT_WriteData8(0x31); TFT_WriteData8(0xC1); TFT_WriteData8(0x48);
    TFT_WriteData8(0x08); TFT_WriteData8(0x0F); TFT_WriteData8(0x0C);
    TFT_WriteData8(0x31); TFT_WriteData8(0x36); TFT_WriteData8(0x0F);

    TFT_WriteCmd(ILI9341_DISPON);
    TFT_LED_ON();
    delay_ms(100);
}

/* ═══════════════════════════════════════════════════════
   ILI9341 drawing primitives
   ═══════════════════════════════════════════════════════ */
static void TFT_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    TFT_WriteCmd(ILI9341_CASET);
    TFT_WriteData8(x0>>8); TFT_WriteData8(x0&0xFF);
    TFT_WriteData8(x1>>8); TFT_WriteData8(x1&0xFF);

    TFT_WriteCmd(ILI9341_PASET);
    TFT_WriteData8(y0>>8); TFT_WriteData8(y0&0xFF);
    TFT_WriteData8(y1>>8); TFT_WriteData8(y1&0xFF);

    TFT_WriteCmd(ILI9341_RAMWR);
}

static void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    TFT_SetWindow(x, y, x+w-1, y+h-1);
    TFT_DC_HIGH();
    TFT_CS_LOW();
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0; i<(uint32_t)w*h; i++) {
        SPI1_WriteByte(hi);
        SPI1_WriteByte(lo);
    }
    TFT_CS_HIGH();
}

/* Draw single 5×7 character scaled ×scale */
static void TFT_DrawChar(uint16_t x, uint16_t y, char c,
                         uint16_t fg, uint16_t bg, uint8_t scale)
{
    if(c < 32 || c > 127) c = '?';
    const uint8_t *glyph = font5x7[c - 32];

    for(uint8_t col=0; col<5; col++) {
        for(uint8_t row=0; row<7; row++) {
            uint16_t color = (glyph[col] & (1u<<row)) ? fg : bg;
            TFT_FillRect(x + col*scale, y + row*scale, scale, scale, color);
        }
    }
    /* 1-pixel gap column */
    TFT_FillRect(x + 5*scale, y, scale, 7*scale, bg);
}

/* Draw string */
static void TFT_DrawString(uint16_t x, uint16_t y, const char *s,
                           uint16_t fg, uint16_t bg, uint8_t scale)
{
    uint16_t cx = x;
    while(*s) {
        TFT_DrawChar(cx, y, *s++, fg, bg, scale);
        cx += 6 * scale;
        if(cx + 6*scale > 240) break;  /* clip to screen width */
    }
}

/* Fill screen */
static void TFT_FillScreen(uint16_t color) {
    TFT_FillRect(0, 0, 240, 320, color);
}

/* ═══════════════════════════════════════════════════════
   TFT UI — draw the complete ESP32 message screen
   Screen layout (portrait 240×320):
   ┌────────────────────────┐
   │  Header bar (blue)     │  y=0..29
   │  "ESP32 → STM32"       │
   ├────────────────────────┤
   │  Source label (gray)   │  y=40..59
   │  "Received:"           │
   ├────────────────────────┤
   │                        │
   │  Message  (big, cyan)  │  y=70..150
   │  (word-wrapped)        │
   │                        │
   ├────────────────────────┤
   │  Divider               │
   │  Previous msg (small)  │  y=200..320
   └────────────────────────┘
   ═══════════════════════════════════════════════════════ */

/* store last two messages */
static char msg_curr[128] = "Waiting...";
static char msg_prev[128] = "";
static uint32_t msg_count = 0;

static void TFT_itoa(uint32_t n, char *buf)
{
    if(n == 0) { buf[0]='0'; buf[1]='\0'; return; }
    char tmp[12]; int i=0;
    while(n){ tmp[i++]='0'+(n%10); n/=10; }
    int j=0; while(i>0) buf[j++]=tmp[--i]; buf[j]='\0';
}

static void TFT_DrawUI(void)
{
    /* ── Header bar ── */
    TFT_FillRect(0, 0, 240, 30, COLOR_BLUE);
    TFT_DrawString(4, 8, "ESP32", COLOR_WHITE, COLOR_BLUE, 2);
    TFT_DrawString(76, 8, "->", COLOR_YELLOW, COLOR_BLUE, 2);
    TFT_DrawString(106, 8, "STM32", COLOR_WHITE, COLOR_BLUE, 2);

    /* ── Message count badge ── */
    char cnt[12];
    TFT_itoa(msg_count, cnt);
    TFT_FillRect(195, 4, 41, 22, COLOR_ORANGE);
    TFT_DrawString(197, 8, "#", COLOR_BLACK, COLOR_ORANGE, 2);
    TFT_DrawString(209, 8, cnt, COLOR_BLACK, COLOR_ORANGE, 2);

    /* ── "RECEIVED:" label ── */
    TFT_FillRect(0, 32, 240, 22, COLOR_GRAY);
    TFT_DrawString(4, 37, "RECEIVED:", COLOR_WHITE, COLOR_GRAY, 1);

    /* ── Message area background ── */
    TFT_FillRect(0, 56, 240, 130, COLOR_BLACK);

    /* Draw current message word-wrapped at scale 2 (12px wide per char) */
    /* Each line: 240/(6*2) = 20 chars */
    uint16_t cx = 4, cy = 62;
    const char *s = msg_curr;
    uint8_t lc = 0;
    while(*s && cy < 180) {
        TFT_DrawChar(cx, cy, *s++, COLOR_CYAN, COLOR_BLACK, 2);
        cx += 12;
        lc++;
        if(lc >= 19 || cx+12 > 236) {
            cx = 4; cy += 18; lc = 0;
        }
    }

    /* ── Divider ── */
    TFT_FillRect(0, 190, 240, 2, COLOR_GRAY);

    /* ── "PREVIOUS:" label ── */
    TFT_FillRect(0, 194, 240, 14, COLOR_BLACK);
    TFT_DrawString(4, 194, "PREV:", COLOR_GRAY, COLOR_BLACK, 1);

    /* ── Previous message (small, dim) ── */
    TFT_FillRect(0, 210, 240, 110, COLOR_BLACK);
    cx=4; cy=210; lc=0;
    s = msg_prev;
    while(*s && cy < 310) {
        TFT_DrawChar(cx, cy, *s++, COLOR_GRAY, COLOR_BLACK, 1);
        cx += 6; lc++;
        if(lc >= 38 || cx+6 > 236) { cx=4; cy+=10; lc=0; }
    }

    /* ── Status bar ── */
    TFT_FillRect(0, 312, 240, 8, COLOR_BLUE);
}

static void TFT_ShowBootScreen(void)
{
    TFT_FillScreen(COLOR_BLACK);
    TFT_FillRect(0, 0, 240, 30, COLOR_BLUE);
    TFT_DrawString(20, 8, "STM32F411RE", COLOR_WHITE, COLOR_BLUE, 2);
    TFT_DrawString(10, 50, "Full-Duplex Bridge", COLOR_CYAN, COLOR_BLACK, 1);
    TFT_DrawString(10, 65, "USART2  <->  PC", COLOR_GREEN, COLOR_BLACK, 1);
    TFT_DrawString(10, 80, "USART1  <->  ESP32", COLOR_GREEN, COLOR_BLACK, 1);
    TFT_DrawString(10, 100, "SPI  TFT  240x320", COLOR_YELLOW, COLOR_BLACK, 1);
    TFT_DrawString(10, 115, "I2C  LCD  16x2", COLOR_YELLOW, COLOR_BLACK, 1);
    TFT_FillRect(0, 312, 240, 8, COLOR_BLUE);
}

/* ═══════════════════════════════════════════════════════
   I2C1 init
   ═══════════════════════════════════════════════════════ */
static void I2C1_Init(void)
{
    RCC_APB1ENR |= (1u<<21);
    I2C1_CR1 |= (1u<<15); I2C1_CR1 &= ~(1u<<15); /* reset */
    I2C1_CR2   = 16;
    I2C1_CCR   = 80;
    I2C1_TRISE = 17;
    I2C1_CR1  |= (1u<<0);
}

static void I2C_Write(uint8_t addr, uint8_t data)
{
    volatile uint32_t t;
    I2C1_CR1 |= (1u<<8);
    t=10000; while(!(I2C1_SR1&(1u<<0))&&--t);
    I2C1_DR = (uint32_t)(addr<<1);
    t=10000; while(!(I2C1_SR1&(1u<<1))&&--t);
    (void)I2C1_SR2;
    I2C1_DR = data;
    t=10000; while(!(I2C1_SR1&(1u<<7))&&--t);
    I2C1_CR1 |= (1u<<9);
}

/* ═══════════════════════════════════════════════════════
   PCF8574 → HD44780 16x2 LCD
   ═══════════════════════════════════════════════════════ */
static void LCD_Pulse(uint8_t d) {
    I2C_Write(LCD_I2C_ADDR, d|LCD_EN); delay_us(2);
    I2C_Write(LCD_I2C_ADDR, d&~LCD_EN); delay_us(50);
}
static void LCD_SendNibble(uint8_t n, uint8_t rs) {
    LCD_Pulse((n<<4)|LCD_BL|rs);
}
static void LCD_SendByte(uint8_t b, uint8_t rs) {
    LCD_SendNibble(b>>4, rs);
    LCD_SendNibble(b&0x0F, rs);
    delay_us(500);
}
#define LCD_CMD(c)  LCD_SendByte(c,0)
#define LCD_DATA(c) LCD_SendByte(c,LCD_RS)

static void LCD_Init(void)
{
    delay_ms(50);
    LCD_SendNibble(0x03,0); delay_ms(5);
    LCD_SendNibble(0x03,0); delay_us(200);
    LCD_SendNibble(0x03,0); delay_us(200);
    LCD_SendNibble(0x02,0); delay_us(200);
    LCD_CMD(0x28); LCD_CMD(0x0C);
    LCD_CMD(0x06); LCD_CMD(0x01);
    delay_ms(3);
}
static void LCD_SetCursor(uint8_t row, uint8_t col) {
    LCD_CMD(row==0 ? (0x80+col) : (0xC0+col));
}
static void LCD_PrintStr(const char *s, uint8_t max) {
    uint8_t n=0;
    while(*s && n<max){ LCD_DATA((uint8_t)*s++); n++; }
    while(n<max){ LCD_DATA(' '); n++; }
}

/* Show ESP32 message on 16x2 LCD */
static void LCD_ShowESP32(const char *msg)
{
    LCD_SetCursor(0, 0);
    LCD_PrintStr("ESP32:", 6);

    uint8_t len = 0;
    while(msg[len]) len++;

    if(len <= 10) {
        LCD_SetCursor(0, 6);
        LCD_PrintStr(msg, 10);
        LCD_SetCursor(1, 0);
        LCD_PrintStr("                ", 16);
    } else {
        LCD_SetCursor(0, 6);
        LCD_PrintStr(msg, 10);
        LCD_SetCursor(1, 0);
        LCD_PrintStr(msg + 10, 16);
    }
}

/* ═══════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════ */
int main(void)
{
    GPIO_Init();
    SPI1_Init();
    TFT_Init();
    I2C1_Init();
    LCD_Init();
    USART1_Init();
    USART2_Init();

    delay_ms(50);

    /* Boot screens */
    TFT_ShowBootScreen();

    LCD_SetCursor(0, 0); LCD_PrintStr("STM32 Ready     ", 16);
    LCD_SetCursor(1, 0); LCD_PrintStr("Waiting ESP32...", 16);

    /* USART2 PC banner */
    USART2_Print("\r\n=============================\r\n");
    USART2_Print(" STM32F411RE Full-Duplex\r\n");
    USART2_Print(" TFT + LCD + UART Bridge\r\n");
    USART2_Print("=============================\r\n");

    delay_ms(2000);
    TFT_DrawUI();   /* draw initial UI */

    while(1)
    {
        char c;

        /* ── Drain PC ring buffer ── */
        while(!lpc_rdy && rb_pop(&rb_u2, &c)) {
            USART2_WriteChar(c);
            if(c=='\r'||c=='\n') {
                if(idx_pc>0){ line_pc[idx_pc]='\0'; lpc_rdy=1; idx_pc=0; USART2_Print("\r\n"); }
            } else if(idx_pc<127) { line_pc[idx_pc++]=c; }
        }

        /* ── Drain ESP32 ring buffer ── */
        while(!lesp_rdy && rb_pop(&rb_u1, &c)) {
            if(c=='\r'||c=='\n') {
                if(idx_esp>0){ line_esp[idx_esp]='\0'; lesp_rdy=1; idx_esp=0; }
            } else if(idx_esp<127) { line_esp[idx_esp++]=c; }
        }

        /* ── PC → ESP32 ── */
        if(lpc_rdy) {
            USART1_Print(line_pc);
            USART1_WriteChar('\n');
            USART2_Print("[STM→ESP32] "); USART2_Print(line_pc); USART2_Print("\r\n");
            lpc_rdy = 0;
        }

        /* ── ESP32 → PC + TFT + LCD ── */
        if(lesp_rdy) {
            /* Forward to PC */
            USART2_Print("[ESP32→PC] "); USART2_Print(line_esp); USART2_Print("\r\n");

            /* Update message history */
            strncpy(msg_prev, msg_curr, 127); msg_prev[127]='\0';
            strncpy(msg_curr, line_esp, 127); msg_curr[127]='\0';
            msg_count++;

            /* Update TFT display */
            TFT_DrawUI();

            /* Update I2C LCD */
            LCD_ShowESP32(line_esp);

            lesp_rdy = 0;
        }
    }
}
