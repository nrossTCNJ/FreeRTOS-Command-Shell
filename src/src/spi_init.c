/*
 *  spi_init.c
 *  Initializes SPI communication
 *  Created on: Jun 17, 2026
 *      Author: nate
 */

#include "spi_init.h"

/* PB3: SCK
 * PB4: MISO
 * PB5: MOSI
 * PB6: CS
 * PC7: RST
*/

void spi1_init(void)
{
	// configure clocks
	RCC->AHB1ENR |= (1 << 1);   		// GPIOB clock enable
	RCC->AHB1ENR |= (1 << 2);			// GPIOC clock enable
	RCC->APB2ENR |= (1 << 12);			// SPI1 clock enable

	// configure PB3, PB4, PB5
	GPIOB->MODER &= ~(0b111111 << 6);	// clear PB3,PB4,PB5
	GPIOB->MODER |= (0b101010 << 6);	// AF mode for PB3,PB4,PB5
	GPIOB->AFR[0] &= ~(0xFFF << 12);	// clear AF selections
	GPIOB->AFR[0] |= (0x555 << 12);		// set all AF05
	GPIOB->OTYPER &= ~(0b111 << 3);		// push-pull output type
	GPIOB->PUPDR &= ~(0b111111 << 6);	// no pull-up, pull-down
	GPIOB->OSPEEDR |= (0b111111 << 6);	// high speed

	// configure PB6, PC7
	GPIOB->MODER &= ~(0b11 << 12);		// clear PB6
	GPIOC->MODER &= ~(0b11 << 14);		// clear PC7
	GPIOB->MODER |= (0b01 << 12);		// output mode for PB6
	GPIOC->MODER |= (0b01 << 14);		// output mode for PC7
	GPIOB->OTYPER &= ~(1 << 6);			// push-pull output type
	GPIOC->OTYPER &= ~(1 << 7);			// push-pull output type
	GPIOB->PUPDR &= ~(0b11 << 12);		// no pull-up, pull-down
	GPIOC->PUPDR &= ~(0b11 << 14);		// no pull-up, pull-down
	GPIOB->BSRR = (1 << 6);				// idle CS high
	GPIOC->BSRR = (1 << 7);				// idle RST high

	// configure master mode
	SPI1->CR1 = 0;						// clear configuration
	SPI1->CR1 |= (1 << 2);				// set master configuration
	SPI1->CR1 &= ~(0b111 << 3);
	SPI1->CR1 |= (0b011 << 3);			// baud rate = 84MHz/16 = 5.25 MHz
	SPI1->CR1 &= ~(1 << 1);				// CPOL = 0, clock idle low
	SPI1->CR1 &= ~(1 << 0);				// CPHA = 0, sample on first edge
	SPI1->CR1 &= ~(1 << 7);				// MSB transmitted first
	SPI1->CR1 &= ~(1 << 11);			// 8-bit data frame format
	SPI1->CR1 |= (1 << 9);				// software slave management
	SPI1->CR1 |= (1 << 8);				// internal NSS high (SSI)
	SPI1->CR1 |= (1 << 6);				// enable SPI
}

uint8_t spi1_transfer(uint8_t tx_data, uint8_t *rx_data)
{
	uint32_t timeout = 10000;
	while(!(SPI1->SR & (1 << 1))) 				// wait for TXE to be set
	{
		if(timeout-- == 0)
			return SPI_ERROR;
	}
	*((volatile uint8_t*)&SPI1->DR) = tx_data; 	// write byte to data register

	timeout = 10000;
	while(!(SPI1->SR & (1 << 0)))				// wait for RXNE to be set
	{
		if(timeout-- == 0)
			return SPI_ERROR;
	}

	*rx_data = *((volatile uint8_t*)&SPI1->DR);	// read received byte

	return SPI_OK;
}

uint8_t spi1_receive(uint8_t *rx_data)
{
	return spi1_transfer(0xFF, rx_data);		// send dummy byte to receive real byte
}

void cs_high(void)
{
	GPIOB->BSRR = (1 << 6);
}

void cs_low(void)
{
	GPIOB->BSRR = (1 << 22);
}

