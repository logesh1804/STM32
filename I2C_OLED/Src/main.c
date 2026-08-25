#include<stdint.h>

/* RCC */

#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)
#define RCC_APB1ENR   (*(volatile uint32_t*)0x40023840)

/* GPIOB */

#define GPIOB_MODER   (*(volatile uint32_t*)0x40020400)
#define GPIOB_OTYPER  (*(volatile uint32_t*)0x40020404)
#define GPIOB_PUPDR   (*(volatile uint32_t*)0x4002040C)
#define GPIOB_AFRH    (*(volatile uint32_t*)0x40020424)

/* I2C1 */

#define I2C1_CR1      (*(volatile uint32_t*)0x40005400)
#define I2C1_CR2      (*(volatile uint32_t*)0x40005404)
#define I2C1_DR       (*(volatile uint32_t*)0x40005410)
#define I2C1_SR1      (*(volatile uint32_t*)0x40005414)
#define I2C1_SR2      (*(volatile uint32_t*)0x40005418)
#define I2C1_CCR      (*(volatile uint32_t*)0x4000541C)
#define I2C1_TRISE    (*(volatile uint32_t*)0x40005420)

#define OLED_ADDR 0x3C

/* FONT */

const uint8_t font[][5] = {

{0x7E,0x11,0x11,0x11,0x7E}, // A
{0x7F,0x49,0x49,0x49,0x36}, // B
{0x3E,0x41,0x41,0x41,0x22}, // C
{0x7F,0x41,0x41,0x22,0x1C}, // D
{0x7F,0x49,0x49,0x49,0x41}, // E
{0x7F,0x09,0x09,0x09,0x01}, // F
{0x3E,0x41,0x49,0x49,0x7A}, // G
{0x7F,0x08,0x08,0x08,0x7F}, // H
{0x00,0x41,0x7F,0x41,0x00}, // I
{0x20,0x40,0x41,0x3F,0x01}, // J
{0x7F,0x08,0x14,0x22,0x41}, // K
{0x7F,0x40,0x40,0x40,0x40}, // L
{0x7F,0x02,0x04,0x02,0x7F}, // M
{0x7F,0x04,0x08,0x10,0x7F}, // N
{0x3E,0x41,0x41,0x41,0x3E}, // O
{0x7F,0x09,0x09,0x09,0x06}, // P
{0x3E,0x41,0x51,0x21,0x5E}, // Q
{0x7F,0x09,0x19,0x29,0x46}, // R
{0x46,0x49,0x49,0x49,0x31}, // S
{0x01,0x01,0x7F,0x01,0x01}, // T
{0x3F,0x40,0x40,0x40,0x3F}, // U
{0x1F,0x20,0x40,0x20,0x1F}, // V
{0x7F,0x20,0x18,0x20,0x7F}, // W
{0x63,0x14,0x08,0x14,0x63}, // X
{0x03,0x04,0x78,0x04,0x03}, // Y
{0x61,0x51,0x49,0x45,0x43}  // Z
};

void delay(void)
{
	for(int i=0;i<50000;i++);
}

/* GPIO INIT */

void gpio_init(void)
{
	RCC_AHB1ENR |= (1<<1);

	GPIOB_MODER &= ~(3<<16);
	GPIOB_MODER &= ~(3<<18);

	GPIOB_MODER |= (2<<16);	GPIOB_PUPDR &= ~(3<<16);

	GPIOB_MODER |= (2<<18);

	GPIOB_OTYPER |= (1<<8);
	GPIOB_OTYPER |= (1<<9);

	GPIOB_PUPDR &= ~(3<<18);

	GPIOB_PUPDR |= (1<<16);
	GPIOB_PUPDR |= (1<<18);

	GPIOB_AFRH &= ~(0xFF);

	GPIOB_AFRH |= (4<<0);
	GPIOB_AFRH |= (4<<4);
}

/* I2C INIT */

void i2c1_init(void)
{
	RCC_APB1ENR |= (1<<21);

	I2C1_CR1 &= ~(1<<0);

	I2C1_CR2 = 16;

	I2C1_CCR = 80;

	I2C1_TRISE = 17;

	I2C1_CR1 |= (1<<0);
}

/* START */

void i2c1_start(void)
{
	I2C1_CR1 |= (1<<8);

	while(!(I2C1_SR1 & (1<<0)));
}

/* ADDRESS */

void i2c1_address(uint8_t address)
{
	I2C1_DR = (address << 1);

	while(!(I2C1_SR1 & (1<<1)));

	volatile uint32_t temp;

	temp = I2C1_SR1;
	temp = I2C1_SR2;
}

/* WRITE */

void i2c1_write(uint8_t data)
{
	while(!(I2C1_SR1 & (1<<7)));

	I2C1_DR = data;

	while(!(I2C1_SR1 & (1<<2)));
}

/* STOP */

void i2c1_stop(void)
{
	I2C1_CR1 |= (1<<9);
}

/* OLED COMMAND */

void oled_cmd(uint8_t cmd)
{
	i2c1_start();

	i2c1_address(OLED_ADDR);

	i2c1_write(0x00);

	i2c1_write(cmd);

	i2c1_stop();
}

/* OLED DATA */

void oled_data(uint8_t data)
{
	i2c1_start();

	i2c1_address(OLED_ADDR);

	i2c1_write(0x40);

	i2c1_write(data);

	i2c1_stop();
}

/* OLED INIT */

void oled_init(void)
{
	oled_cmd(0xAE);
	oled_cmd(0x20);
	oled_cmd(0x10);
	oled_cmd(0xB0);
	oled_cmd(0xC8);
	oled_cmd(0x00);
	oled_cmd(0x10);
	oled_cmd(0x40);
	oled_cmd(0x81);
	oled_cmd(0xFF);
	oled_cmd(0xA1);
	oled_cmd(0xA6);
	oled_cmd(0xA8);
	oled_cmd(0x3F);
	oled_cmd(0xA4);
	oled_cmd(0xD3);
	oled_cmd(0x00);
	oled_cmd(0xD5);
	oled_cmd(0xF0);
	oled_cmd(0xD9);
	oled_cmd(0x22);
	oled_cmd(0xDA);
	oled_cmd(0x12);
	oled_cmd(0xDB);
	oled_cmd(0x20);
	oled_cmd(0x8D);
	oled_cmd(0x14);
	oled_cmd(0xAF);
}

/* CLEAR */

void oled_clear(void)
{
	for(int i=0;i<1024;i++)
	{
		oled_data(0x00);
	}
}

/* CHARACTER */

void oled_char(char ch)
{
	int i;

	ch = ch - 'A';

	for(i=0;i<15;i++)
	{
		oled_data(font[(int)ch][i]);
	}

	oled_data(0x00);
}

/* STRING */

void oled_string(char *str)
{
	while(*str)
	{
		if(*str >= 'A' && *str <= 'Z')
		{
			oled_char(*str);
		}

		str++;
	}
}

/* MAIN */

int main(void)
{
	gpio_init();

	i2c1_init();

	delay();

	oled_init();

	oled_clear();

	oled_string("Dineshkumar");

	while(1);
}
