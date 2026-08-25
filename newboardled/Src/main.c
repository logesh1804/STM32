#include<stdint.h>
#include<stdio.h>
#define RCC_AHB1ENR (*(volatile uint32_t*)0x40023830)
#define GPIOB_MODER (*(volatile uint32_t*)0x40020400)
#define GPIOB_ODR (*(volatile uint32_t*)0x40020014)

static void delay(){
	for(int i=0;i<300000;i++);
}
int main(void){
	RCC_AHB1ENR |=(1<<0);
	GPIOB_MODER &=~(3<<26);
	GPIOB_MODER |=(1<<26);

while(1){
	GPIOB_ODR |=(1<<13);
	delay();
	GPIOB_ODR &=~(1<<13);
	delay();

}
}
