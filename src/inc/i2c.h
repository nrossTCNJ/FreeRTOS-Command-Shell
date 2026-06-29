/*
 *  i2c.h
 *  Header for i2c.c file
 *  Created on: Jun 4, 2026
 *      Author: nate
 */

#ifndef I2C_H
#define I2C_H

#define I2C_OK 0
#define I2C_ERROR 1

#include "stm32f4xx.h"

void i2c1_init(void);
uint8_t i2c_read(uint8_t device_address, uint8_t regular_address, uint8_t* result);
uint8_t i2c_write(uint8_t device_address, uint8_t regular_address, uint8_t byte_to_write);
uint8_t i2c_read_burst(uint8_t device_address, uint8_t regular_address, uint8_t* buffer, uint8_t length);

#endif
