#include<stdint.h>
#define RCC_AHB1ENR (*(volatile uint32_t*)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR (*(volatile uint32_t*)0x40020014)
#define GPIOB_MODER (*(volatile uint32_t*)0x40020400)

#define GPIOB_ODR (*(volatile uint32_t*)0x40020414)
#define SYST_CSR (*(volatile uint32_t*)0xE000E010)
#define SYST_RVR (*(volatile uint32_t*)0xE000E014)
#define SYST_CVR (*(volatile uint32_t*)0xE000E018)
volatile uint32_t g_tick=0;
uint32_t end1=0;
uint32_t end2=0;

void delay(){
	for (int i=0;i<20000;i++);
}
void task1(){

	if((g_tick-end1) >=1000){
	GPIOA_ODR ^=(1<<5);
	end1=g_tick;
}}
void task2(){


if((g_tick-end2) >=1500){
	GPIOB_ODR ^=(1<<0);
	end2=g_tick;
}
}

void SysTick_Handler(void)
{
    g_tick++;
}
void SysTick_Init(void)
{
    SYST_RVR = 16000 - 1;

    SYST_CVR = 0;

    SYST_CSR = (1<<0)|(1<<1)|(1<<2);
}
int main(){

RCC_AHB1ENR |= (1<<0);
RCC_AHB1ENR |= (1<<1);
GPIOA_MODER &= ~(3<<(5*2));
GPIOA_MODER |=  (1<<(5*2));
GPIOB_MODER &= ~(3<<(0*2));
GPIOB_MODER |=  (1<<(0*2));
SysTick_Init();

while(1){
	task1();
	task2();
}

}
