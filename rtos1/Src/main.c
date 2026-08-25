#include<stdint.h>
#define RCC_AHB1ENR (*(volatile uint32_t*)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR (*(volatile uint32_t*)0x40020014)
#define SYST_CSR (*(volatile uint32_t*)0xE000E010)
#define SYST_RVR (*(volatile uint32_t*)0XE000E014)
#define SYST_CVR (*(volatile uint32_t*)0xE000E018)
volatile uint32_t g_tick =0;
void delay(){
	for(int i=0;i<20000;i++);
}
void led_task(){
	static uint32_t end=0;
	if(g_tick-end>=1000){
		GPIOA_ODR|=(1<<5);
		delay();
		GPIOA_ODR &=~(1<<5);
		delay();
		end=g_tick;
	}
}

void SysTick_Handler(){
	g_tick++;
}
int main(){
	RCC_AHB1ENR|=(1<<0);
	GPIOA_MODER &= ~(3<<10);
	GPIOA_MODER |=(1<<10);
	SYST_CSR=1600;
	SYST_RVR =0;
	SYST_CVR=0x07;
	while(1){
		led_task();
	}
}
