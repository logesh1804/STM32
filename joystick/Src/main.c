
#include<stdint.h>
#include<stdio.h>
#define RCC_AHB1ENR (*(volatile uint32_t*)0x40023830)
#define RCC_APB2ENR (*(volatile uint32_t*)0x40023844)
#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)
#define ADC_SQR3  (*(volatile uint32_t*)0x40012034)
#define ADC_CR2 (*(volatile uint32_t*)0x40012008)
#define ADC_DR (*(volatile uint32_t*)0x4001204C)
#define ADC_SR (*(volatile uint32_t*)0x40012000)
#define ADC_SMPR2 (*(volatile uint32_t*)0x40012010)
#define GPIOA_ODR (*(volatile uint32_t*)0x40020014)
#define RCC_APB1ENR  (*(volatile uint32_t*)0x40023840)
#define GPIOA_AFRL (*(volatile uint32_t*)0x40020020)




#define GPIOB_MODER   (*(volatile uint32_t*)0x40020400)
#define GPIOB_OTYPER  (*(volatile uint32_t*)0x40020404)
#define GPIOB_PUPDR   (*(volatile uint32_t*)0x4002040C)
#define GPIOB_AFRL   (*(volatile uint32_t*)0x40020420)

#define I2C1_CR1      (*(volatile uint32_t*)0x40005400)
#define I2C1_CR2      (*(volatile uint32_t*)0x40005404)
#define I2C1_DR       (*(volatile uint32_t*)0x40005410)
#define I2C1_SR1      (*(volatile uint32_t*)0x40005414)
#define I2C1_SR2      (*(volatile uint32_t*)0x40005418)
#define I2C1_CCR      (*(volatile uint32_t*)0x4000541C)
#define I2C1_TRISE    (*(volatile uint32_t*)0x40005420)
#define OLED_ADDR 0x3C

volatile uint16_t joystick_value = 0;
void ADC_Init(void)
{
    RCC_AHB1ENR |= (1<<0);     // GPIOA Clock
    RCC_APB2ENR |= (1<<8);     // ADC1 Clock

    /* PA0 Analog Mode */
    GPIOA_MODER &= ~(3<<0);
    GPIOA_MODER |=  (3<<0);

    ADC_SQR3 = 0;              // Channel 0

    ADC_CR2 |= (1<<0);         // ADC ON

    ADC_SMPR2 &= ~(7<<0);
    ADC_SMPR2 |=  (7<<0);      // 480 cycles
}

uint16_t Joystick_Read(void)
{
    ADC_CR2 |= (1<<30);        // Start Conversion

    while(!(ADC_SR & (1<<1))); // EOC

    return (uint16_t)ADC_DR;
}
void i2cint(void)
{
    RCC_AHB1ENR |= (1<<1);
    RCC_APB1ENR |= (1<<21);

    GPIOB_MODER &= ~(3<<12);
    GPIOB_MODER |=  (2<<12);

    GPIOB_MODER &= ~(3<<14);
    GPIOB_MODER |=  (2<<14);

    GPIOB_AFRL &= ~(15<<24);
    GPIOB_AFRL |=  (4<<24);

    GPIOB_AFRL &= ~(15<<28);
    GPIOB_AFRL |=  (4<<28);

    GPIOB_OTYPER |= (1<<6);
    GPIOB_OTYPER |= (1<<7);

    GPIOB_PUPDR &= ~(3<<12);
    GPIOB_PUPDR &= ~(3<<14);

    GPIOB_PUPDR |= (1<<12);
    GPIOB_PUPDR |= (1<<14);

    I2C1_CR1 &= ~(1<<0);

    I2C1_CR2   = 16;
    I2C1_CCR   = 80;
    I2C1_TRISE = 17;

    I2C1_CR1 |= (1<<0);
}

void i2cst()
{
	I2C1_CR1 |= (1<<8);

	while(!(I2C1_SR1 & (1<<0)));


}
void i2caddrs(uint8_t address)
{
	I2C1_DR = (address<<1);

	while(!(I2C1_SR1 & (1<<1)));




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
void oled_set_cursor(uint8_t page, uint8_t column)
{
    oled_cmd(0xB0 + page);

    oled_cmd(column & 0x0F);

    oled_cmd(0x10 | ((column >> 4) & 0x0F));
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

void OLED_ShowValue(uint16_t value)
{
    char buffer[10];

    sprintf(buffer,"%u",value);

    oled_set_cursor(2,0);

    oled_data_start();

    for(int i=0;i<128;i++)
    {
        i2c1_write(0x00);
    }

    oled_data_stop();

    oled_set_cursor(2,0);

    oled_string(buffer);
}
void oled_init(void)
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


void JoystickTask(void)
{
    while(1)
    {
        joystick_value = Joystick_Read();

        OLED_ShowValue(joystick_value);

        //TaskDelay(100);
    }
}

int main(void)
{
    ADC_Init();

    i2cint();

    oled_init();

    oled_clear();

    while(1)
    {
        joystick_value = Joystick_Read();

        OLED_ShowValue(joystick_value);

        for(volatile int i=0;i<100000;i++);
    }
}
