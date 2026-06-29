/*
 *  i2c.c
 *  Initializes i2c protocol
 *  Created on: Jun 4, 2026
 *      Author: nate
 */

#include "i2c.h"

void i2c1_init(void) {
	// clock enables
	RCC->AHB1ENR |= (1 << 1);   // GPIOB
	RCC->APB1ENR |= (1 << 21);  // I2C1

	// set PB8 and PB9 to alternate function mode (10)
	GPIOB->MODER &= ~(0b1111 << 16);  // clear
	GPIOB->MODER |= (0b1010 << 16);   // set

	// set PB8 and PB9 to open-drain
	GPIOB->OTYPER |= (0b11 << 8);

	// pull-up pull-down mode
	GPIOB->PUPDR &= ~(0b1111 << 16); // clear
	GPIOB->PUPDR |= (0b0000 << 16);   // set (to zero)

	// set alternate function register
	GPIOB->AFR[1] &= ~(0xFF); // clear
	GPIOB->AFR[1] |= (0x44);  // set to AF4

	// sets peripheral clock frequency
	I2C1->CR2 &= ~(0b111111); // clear
	I2C1->CR2 |= (0b101010);  // 42 MHz

	// set the SCL clock
	I2C1->CCR &= ~(0b111111111111); // clear
	I2C1->CCR |= (0b000011010010);  // set to 100kHz

	// set the rise time
	I2C1->TRISE &= ~(0b111111); // clear
	I2C1->TRISE |= (0b101011);  // set to 43ns

	// enable peripheral
	I2C1->CR1 |= 0b1;

	// enable ACK
	I2C1->CR1 |= (1 << 10);
}

// function to read any register from an I2C device
uint8_t i2c_read(uint8_t device_address, uint8_t regular_address, uint8_t* result)
{
	// create START condition
	I2C1->CR1 |= (1 << 8);
	uint32_t timeout = 10000;
	// checks SB if start condition created
	while(!(I2C1->SR1 & (1 << 0))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// set device register to write mode
	I2C1->DR = device_address << 1 | 0;
	timeout = 10000;
	// checks ADDR if address successfully sent and ACKed
	while(!(I2C1->SR1 & (1 << 1))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}
	// clear ADDR
	(void)I2C1->SR1;
	(void)I2C1->SR2;

	// set register address to read from
	I2C1->DR = regular_address;
	timeout = 10000;
	// check BTF (byte transfer finished)
	while(!(I2C1->SR1 & (1 << 2))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// create another START condition
	I2C1->CR1 |= (1 << 8);
	timeout = 10000;
	while(!(I2C1->SR1 & (1 << 0))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// set device register to read mode
	I2C1->DR = device_address << 1 | 1;
	timeout = 10000;
	while(!(I2C1->SR1 & (1 << 1))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}
	// disable ACK because only reading one byte
	I2C1->CR1 &= ~(1 << 10);
	(void)I2C1->SR1;
	(void)I2C1->SR2;

	// generate STOP condition
	I2C1->CR1 |= (1 << 9);
	timeout = 10000;
	// check RXNE for data ready to be read
	while(!(I2C1->SR1 & (1 << 6))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}
	// read data register
	*result = I2C1->DR;

	// enable ACK
	I2C1->CR1 |= (1 << 10);

	return I2C_OK;
}

uint8_t i2c_write(uint8_t device_address, uint8_t regular_address, uint8_t byte_to_write)
{
	// create START condition
	I2C1->CR1 |= (1 << 8);
	uint32_t timeout = 10000;
	while(!(I2C1->SR1 & (1 << 0))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// set device register to write mode
	I2C1->DR = device_address << 1 | 0;
	timeout = 10000;
	// checks ADDR if address successfully sent and ACKed
	while(!(I2C1->SR1 & (1 << 1))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}
	// clear ADDR
	(void)I2C1->SR1;
	(void)I2C1->SR2;

	// set register address to write
	I2C1->DR = regular_address;
	timeout = 10000;
	// check BTF (byte transfer finished)
	while(!(I2C1->SR1 & (1 << 2))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// set byte to write
	I2C1->DR = byte_to_write;
	timeout = 10000;
	// check BTF (byte transfer finished)
	while(!(I2C1->SR1 & (1 << 2))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// generate STOP condition
	I2C1->CR1 |= (1 << 9);
	return I2C_OK;
}

uint8_t i2c_read_burst(uint8_t device_address, uint8_t regular_address, uint8_t* buffer, uint8_t length)
{
	// create START condition
	I2C1->CR1 |= (1 << 8);
	uint32_t timeout = 10000;
	// checks SB if start condition created
	while(!(I2C1->SR1 & (1 << 0))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// set device register to write mode
	I2C1->DR = device_address << 1 | 0;
	timeout = 10000;
	// checks ADDR if address successfully sent and ACKed
	while(!(I2C1->SR1 & (1 << 1))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}
	// clear ADDR
	(void)I2C1->SR1;
	(void)I2C1->SR2;

	// set register address to read from
	I2C1->DR = regular_address;
	timeout = 10000;
	// check BTF (byte transfer finished)
	while(!(I2C1->SR1 & (1 << 2))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// create another START condition
	I2C1->CR1 |= (1 << 8);
	timeout = 10000;
	while(!(I2C1->SR1 & (1 << 0))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// set device register to read mode
	I2C1->DR = device_address << 1 | 1;
	timeout = 10000;
	while(!(I2C1->SR1 & (1 << 1))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}
	// clear ADDR
	(void)I2C1->SR1;
	(void)I2C1->SR2;

	// read each byte
	for(int i = 0; i < length-1; i++)
	{
		timeout = 10000;
		// check RXNE for data ready to be read
		while(!(I2C1->SR1 & (1 << 6))) {
			if(--timeout == 0)
				return I2C_ERROR;
		}
		// read data register
		buffer[i] = I2C1->DR;
	}

	// disable ACK for last byte
	I2C1->CR1 &= ~(1 << 10);
	(void)I2C1->SR1;
	(void)I2C1->SR2;

	// generate STOP condition
	I2C1->CR1 |= (1 << 9);
	timeout = 10000;
	// check RXNE for data ready to be read
	while(!(I2C1->SR1 & (1 << 6))) {
		if(--timeout == 0)
			return I2C_ERROR;
	}

	// read last byte
	buffer[length-1] = I2C1->DR;

	// enable ACK
	I2C1->CR1 |= (1 << 10);
	return I2C_OK;
}

