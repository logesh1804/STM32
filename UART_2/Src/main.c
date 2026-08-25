#include <stdint.h>

/* RCC */
#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)
#define RCC_APB2ENR   (*(volatile uint32_t*)0x40023844)

/* GPIOC */
#define GPIOC_MODER   (*(volatile uint32_t*)0x40020800)
#define GPIOC_AFRL    (*(volatile uint32_t*)0x40020820)

/* USART6 */
#define USART6_SR     (*(volatile uint32_t*)0x40011400)
#define USART6_DR     (*(volatile uint32_t*)0x40011404)
#define USART6_BRR    (*(volatile uint32_t*)0x40011408)
#define USART6_CR1    (*(volatile uint32_t*)0x4001140C)

/* Functions */
void UART6_SendChar(char c)
{
    while (!(USART6_SR & (1 << 7))); // TXE
    USART6_DR = c;
}

void UART6_SendString(char *str)
{
    while (*str)
    {
        UART6_SendChar(*str++);
    }
}

char UART6_ReadChar(void)
{
    while (!(USART6_SR & (1 << 5))); // RXNE
    return USART6_DR;
}


int main(void)
{
    char buffer[50];
    int i = 0;

    /* 1. Enable clocks */
    RCC_AHB1ENR |= (1 << 2);   // GPIOC
    RCC_APB2ENR |= (1 << 5);   // USART6

    /* 2. PC6, PC7 → Alternate function */
    GPIOC_MODER &= ~(3 << 12);
    GPIOC_MODER |=  (1<< 12);

        GPIOC_MODER &= ~(3 << 14);
        GPIOC_MODER |=  (1<< 14);
    /* 3. AF8 (USART6) */
    GPIOC_AFRL &= ~(15 << 24);
    GPIOC_AFRL |=  (8 << 24);

    /* 4. Baud rate (9600 @16MHz) */
    USART6_BRR = 0x0683;

    /* 5. Enable UART */
    USART6_CR1 |= (1 << 3);   // TX
    USART6_CR1 |= (1 << 2);   // RX
    USART6_CR1 |= (1 << 13);  // Enable

    UART6_SendString("usart6 ready\r\n");

    while (1)
    {
        char c = UART6_ReadChar();

        if (c == '\r')  // Enter key
        {
            buffer[i] = '\0';

            UART6_SendString("\r\nbharath: ");
            UART6_SendString(buffer);
            UART6_SendString("\r\n");

            i = 0;
        }
        else
        {
            buffer[i++] = c;
            UART6_SendChar(c); // echo
        }
    }
}
