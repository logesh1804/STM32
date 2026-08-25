#include <stdint.h>

#define RCC_AHB1ENR (*(volatile uint32_t*)0x40023830)
#define RCC_APB1ENR (*(volatile uint32_t*)0x40023840)

#define GPIOA_MODER (*(volatile uint32_t*)0x40020000)
#define GPIOA_AFRL  (*(volatile uint32_t*)0x40020020)

#define GPIOB_MODER    (*(volatile uint32_t*)0x40020400)
#define GPIOB_ODR      (*(volatile uint32_t*)0x40020414)

#define TIM2_CR1    (*(volatile uint32_t*)0x40000000)
#define TIM2_EGR    (*(volatile uint32_t*)0x40000014)
#define TIM2_CCMR1  (*(volatile uint32_t*)0x40000018)
#define TIM2_CCER   (*(volatile uint32_t*)0x40000020)
#define TIM2_PSC    (*(volatile uint32_t*)0x40000028)
#define TIM2_ARR    (*(volatile uint32_t*)0x4000002C)
#define TIM2_CCR1   (*(volatile uint32_t*)0x40000034)


void delay(void){

	for(int i=0;i<1000000;i++){

	}
}
int main(void)
{
	//CLOCK
     RCC_AHB1ENR |=(1<<0);
     RCC_AHB1ENR |=(1<<1);
     RCC_APB1ENR |=(1<<0);
     //MODE
     GPIOA_MODER &=~(3<<10);
     GPIOA_MODER  |=(2<<10);
     //ALTERNATE FUNCTION
     GPIOA_AFRL &= ~(0xF<<20);
     GPIOA_AFRL |= (1<<20);
     //PB0 PB1 Output
     GPIOB_MODER &= ~(3<<0);
     GPIOB_MODER |=  (1<<0);

     GPIOB_MODER &= ~(3<<2);
     GPIOB_MODER |=  (1<<2);

     // Forward Direction
     GPIOB_ODR |=  (1<<0);   // IN1 HIGH
     GPIOB_ODR &= ~(1<<1);   // IN2 LOW
     //TIMER CONFIGURATION
     TIM2_PSC = 99;		//CLK DIVIDE
     TIM2_ARR = 999;	//PWM PERIOD
     TIM2_EGR = 1;		//Update generation

     TIM2_CCMR1 &= ~(7<<4);	//CLEAR ENABLE
     TIM2_CCMR1 |= (6<<4);	//PWM MODE ON CHANNEL 1
     TIM2_CCMR1 |= (1<<3);	//ENABLE PRELOAD
     TIM2_CCER |= (1<<0);	//ENABLE CH1
     TIM2_CR1 |= (1<<0);

     while(1){

    	    TIM2_CCR1 = ;      // 0% off

     }
}
