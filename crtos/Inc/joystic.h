


#ifndef JOYSTIC_H_
#define JOYSTIC_H_
#include<stdint.h>
#include <stdio.h>


#define RCC_APB2ENR (*(volatile uint32_t*)0x40023844)
#define RCC_AHB1ENR (*(volatile uint32_t*)0x40023830)
#define ADC_SQR3  (*(volatile uint32_t*)0x40012034)
#define ADC_CR2 (*(volatile uint32_t*)0x40012008)
#define ADC_DR (*(volatile uint32_t*)0x4001204C)
#define ADC_SR (*(volatile uint32_t*)0x40012000)
#define ADC_SMPR2 (*(volatile uint32_t*)0x40012010)
#define RCC_APB1ENR  (*(volatile uint32_t*)0x40023840)
#define GPIOA_AFRL (*(volatile uint32_t*)0x40020020)

#define USART_SR (*(volatile uint32_t*)0x40004400)
#define USART_DR (*(volatile uint32_t*)0x40004404)
#define USART_BRR (*(volatile uint32_t*)0x40004408)
#define USART_CR1 (*(volatile uint32_t*)0x4000440C)



#define GPIOB_MODER   (*(volatile uint32_t*)0x40020400)
#define GPIOB_OTYPER  (*(volatile uint32_t*)0x40020404)
#define GPIOB_PUPDR   (*(volatile uint32_t*)0x4002040C)
#define GPIOB_AFRL   (*(volatile uint32_t*)0x40020420)
#define GPIOA_MODER (*(volatile uint32_t*)0x40020000)
#define I2C1_CR1      (*(volatile uint32_t*)0x40005400)
#define I2C1_CR2      (*(volatile uint32_t*)0x40005404)
#define I2C1_DR       (*(volatile uint32_t*)0x40005410)
#define I2C1_SR1      (*(volatile uint32_t*)0x40005414)
#define I2C1_SR2      (*(volatile uint32_t*)0x40005418)
#define I2C1_CCR      (*(volatile uint32_t*)0x4000541C)
#define I2C1_TRISE    (*(volatile uint32_t*)0x40005420)
#define OLED_ADDR 0x3C












extern volatile uint16_t value=0;
//char data='b';
//char data1='f'
void oled_set_cursor(uint8_t page,uint8_t column);
void joystic();
void i2cint(){
RCC_AHB1ENR |=(1<<1);
RCC_APB1ENR |=(1<<21);
GPIOB_MODER &=~(3<<12);
GPIOB_MODER |=(2<<12);
GPIOB_MODER &=~(3<<14);
GPIOB_MODER |=(2<<14);
GPIOB_AFRL &=~(15<<24);
GPIOB_AFRL |=(4<<24);
GPIOB_AFRL &=~(15<<28);
GPIOB_AFRL|=(4<<28);
GPIOB_OTYPER |= (1<<6);
GPIOB_OTYPER |= (1<<7);
GPIOB_PUPDR &= ~(3<<12);
GPIOB_PUPDR &= ~(3<<14);
GPIOB_PUPDR |=(1<<12);
GPIOB_PUPDR |=(1<<14);
I2C1_CR1 &= ~(1<<0);
I2C1_CR2 = 16;
I2C1_CCR = 80;
I2C1_TRISE = 17;
I2C1_CR1 |= (1<<0);
}
void i2cst()
{
	I2C1_CR1 |= (1<<8);

	while(!(I2C1_SR1 & (1<<0)));

	while(!(USART_SR & (1<<7)));

}
void i2caddrs(uint8_t address)
{
	I2C1_DR = (address<<1);

	while(!(I2C1_SR1 & (1<<1)));

	while(!(USART_SR & (1<<7)));


	volatile uint32_t temp;

	temp = I2C1_SR1;
	temp = I2C1_SR2;
}


void i2c1_write(uint8_t data)
{
	while(!(I2C1_SR1 & (1<<7)));

	I2C1_DR = data;
	while(!(I2C1_SR1 &(1<<2)));
}
void i2c1_stop(void)
{
	I2C1_CR1 |= (1<<9);
}



void oled_cmd(uint8_t cmd)
{
	i2cst();

	i2caddrs(OLED_ADDR);

	i2c1_write(0x00);

	i2c1_write(cmd);

	i2c1_stop();
}




void oled_data(uint8_t data)
{
	i2cst();

	i2caddrs(OLED_ADDR);

	i2c1_write(0x40);
	i2c1_write(data);
	i2c1_stop();

}
void oled_data_start(void)
{
    i2cst();

    i2caddrs(OLED_ADDR);

    i2c1_write(0x40);
}

void oled_data_stop(void)
{
	i2c1_stop();
}


void oled_clear(void)
{
    for(int page=0; page<8; page++)
    {
        oled_set_cursor(page,0);

        oled_data_start();

        for(int i=0;i<128;i++)
        {
            i2c1_write(0x00);
        }

        oled_data_stop();
    }
}

void oled_init()
{

	    oled_cmd(0xAE);

	    oled_cmd(0xD5);
	    oled_cmd(0x80);

	    oled_cmd(0xA8);
	    oled_cmd(0x3F);

	    oled_cmd(0xD3);
	    oled_cmd(0x00);

	    oled_cmd(0x40);

	    oled_cmd(0x8D);
	    oled_cmd(0x14);

	    oled_cmd(0x20);
	    oled_cmd(0x10);

	    oled_cmd(0xA1);

	    oled_cmd(0xC8);

	    oled_cmd(0xDA);
	    oled_cmd(0x12);

	    oled_cmd(0x81);
	    oled_cmd(0xCF);

	    oled_cmd(0xD9);
	    oled_cmd(0xF1);

	    oled_cmd(0xDB);
	    oled_cmd(0x40);

	    oled_cmd(0xA4);

	    oled_cmd(0xA6);

	    oled_cmd(0xAF);
	}

const uint8_t font5x7[][5] =
{
    {0x3E,0x51,0x49,0x45,0x3E},
    {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30},
    {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},
    {0x06,0x49,0x49,0x29,0x1E}
};

void oled_digit(char c)
{
    uint8_t index = c - '0';

    for(int i=0;i<5;i++)
    {
        oled_data(font5x7[index][i]);
    }

    oled_data(0x00);
}


void oled_string(char *str)
{
    while(*str)
    {
        oled_digit(*str);
        str++;
    }
}



void oled_set_cursor(uint8_t page, uint8_t column)
{
    oled_cmd(0xB0 + page);

    oled_cmd(column & 0x0F);

    oled_cmd(0x10 | ((column >> 4) & 0x0F));
}










int  joystick(void){


	RCC_AHB1ENR &= ~(1<<0);
	RCC_AHB1ENR |= (1<<0);
	RCC_APB2ENR &= ~(1<<8);
	RCC_APB2ENR |= (1<<8);
	GPIOA_MODER &=~(3<<0);
	GPIOA_MODER |=(3<<0);
	GPIOA_MODER &= ~(3<<10);
	GPIOA_MODER |=  (1<<10);
	ADC_SQR3 =0;
	ADC_CR2 &= ~(1<<0);
	ADC_CR2 |= (1<<0);
	ADC_SMPR2 &= ~(7<<0);
	ADC_SMPR2 |= (7<<0);





	RCC_APB1ENR |=(1<<17);
	GPIOA_MODER &=~(3<<4);
	GPIOA_MODER &=~(3<<6);
	GPIOA_MODER |=(2<<4);
	GPIOA_MODER	|=(2<<6);
	GPIOA_AFRL &= ~(15 << 8);
	GPIOA_AFRL |=  (7 << 8);
	GPIOA_AFRL &= ~(15 << 12);
	GPIOA_AFRL |=(7 << 12);
    USART_CR1  &= ~(1<<13);
    USART_CR1 &=~(3<<2);
	USART_CR1  |=(1<<13);
	USART_CR1 |=(3<<2);
	USART_CR1 &=~(1<<15);
	USART_BRR=0x0683;
	//i2cint();
	//joystic();
	//i2cst();
	//i2caddrs(OLED_ADDR);
	//i2c1_write(value);
	//i2c1_stop();


	i2cint();

	oled_init();
	oled_clear();

	/* Entire display follows RAM */
	oled_cmd(0xA4);

	/* Page 0 */
	oled_cmd(0xB0);

	/* Column 0 */
	oled_cmd(0x00);
	oled_cmd(0x10);

	/* Start continuous data transfer */
	oled_data_start();

	for(int i=0;i<20;i++)
	{
	    i2c1_write(0xFF);
	}

	oled_data_stop();
	while(1) {joystic();
	for(int i=0;i<10000;i++);
	}



	while(1);
}

	void joystic(){



	{ 	    ADC_CR2 |= (1<<30);


	    while(!(ADC_SR & (1<<1)));

	    value = ADC_DR;

	    char buffer [20];
	    sprintf(buffer,"%u",value);


	    oled_set_cursor(2,0);

	        oled_data_start();

	        for(int i=0;i<128;i++)
	        {
	            i2c1_write(0x00);
	        }

	        oled_data_stop();



	    oled_set_cursor(2,0);
	   // oled_string("123456");
	    oled_string(buffer);
	    for(volatile int i=0;i<1000;i++);
	    for(int i=0; buffer[i]!='\0';i++)
	    {
	    	while(!(USART_SR&(1<<7)));
	    USART_DR=buffer[i];

	    }




	  }



	}


#endif /* JOYSTIC_H_ */
