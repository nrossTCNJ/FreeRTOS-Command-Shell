/*
 *  spi_init.h
 *  Header file for spi_init.c
 *  Created on: Jun 17, 2026
 *      Author: nate
 */

#ifndef SPI_INIT_H_
#define SPI_INIT_H_

#define SPI_OK 0
#define SPI_ERROR 1

#include "stm32f4xx.h"

void spi1_init(void);
uint8_t spi1_transfer(uint8_t tx_data, uint8_t *rx_data);
uint8_t spi1_receive(uint8_t *rx_data);
void cs_high(void);
void cs_low(void);

#endif /* SPI_INIT_H_ */
