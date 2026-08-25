#include <stdint.h>

#define RCC_AHB1ENR  (*(volatile uint32_t*)0x40023830)
#define RCC_APB2ENR  (*(volatile uint32_t*)0x40023844)

#define GPIOA_MODER  (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR    (*(volatile uint32_t*)0x40020014)

#define ADC_SR       (*(volatile uint32_t*)0x40012000)
#define ADC_CR1      (*(volatile uint32_t*)0x40012004)
#define ADC_CR2      (*(volatile uint32_t*)0x40012008)
#define ADC_SMPR2    (*(volatile uint32_t*)0x40012010)
#define ADC_SQR3     (*(volatile uint32_t*)0x40012034)
#define ADC_DR       (*(volatile uint32_t*)0x4001204C)









#define GPIOB_MODER   (*(volatile uint32_t*)0x40020400)
#define GPIOB_OTYPER  (*(volatile uint32_t*)0x40020404)
#define GPIOB_PUPDR   (*(volatile uint32_t*)0x4002040C)
#define GPIOB_AFRH    (*(volatile uint32_t*)0x40020424)





#define I2C1_CR1      (*(volatile uint32_t*)0x40005400)
#define I2C1_CR2      (*(volatile uint32_t*)0x40005404)
#define I2C1_DR       (*(volatile uint32_t*)0x40005410)
#define I2C1_SR1      (*(volatile uint32_t*)0x40005414)
#define I2C1_SR2      (*(volatile uint32_t*)0x40005418)
#define I2C1_CCR      (*(volatile uint32_t*)0x4000541C)
#define I2C1_TRISE    (*(volatile uint32_t*)0x40005420)

#define OLED_ADDR 0x3C

int main(void)
{
    uint16_t value;

    /* Enable GPIOA Clock */
    RCC_AHB1ENR |= (1 << 0);

    /* Enable ADC1 Clock */
    RCC_APB2ENR |= (1 << 8);

    /* PA0 -> Analog Mode */
    GPIOA_MODER &= ~(3 << 0);
    GPIOA_MODER |=  (3 << 0);

    /* PA5 -> Output Mode (Onboard LED) */
    GPIOA_MODER &= ~(3 << 10);
    GPIOA_MODER |=  (1 << 10);

    /* ADC Channel 0 = PA0 */
    ADC_SQR3 = 0;

    /* Sample Time for Channel 0 */
    ADC_SMPR2 |= (7 << 0);

    /* Enable ADC */
    ADC_CR2 |= (1 << 0);

    while(1)
    {
        /* Start Conversion */
        ADC_CR2 |= (1 << 30);

        /* Wait for End Of Conversion */
        while(!(ADC_SR & (1 << 1)));

        /* Read ADC Result */
        value = ADC_DR;
        __asm("NOP");

        /* LED Control */
        if(value > 2000)
        {
            GPIOA_ODR |= (1 << 5);
        }
        else
        {
            GPIOA_ODR &= ~(1 << 5);
        }
    }
}
