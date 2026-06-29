/*
 *  mfrc522.h
 *  Header file for mfrc522.c
 *  Created on: Jun 18, 2026
 *      Author: nate
 */

#ifndef MFRC522_H_
#define MFRC522_H_

#include "stm32f4xx.h"
#include "spi_init.h"
#include "main.h"

#define VERSION_REG 0x37
#define TX_CTRL_REG 0x14
#define COMMAND_REG 0x01
#define COM_IRQ_REG 0x04
#define FIFO_LEVEL_REG 0x0A
#define FIFO_DATA_REG 0x09
#define BITFRAME_REG 0x0D

uint8_t mfrc522_init(void);
void mfrc522_task(void *pvParameters);
void handle_mfrc522(void);
void check_uid(uint8_t *uid);
void configure_leds(void);
uint8_t mfrc522_read_uid(uint8_t *uid);
uint8_t mfrc522_detect_card(uint8_t *atqa);
uint8_t mfrc522_read_reg(uint8_t reg, uint8_t *result);
uint8_t mfrc522_write_reg(uint8_t reg, uint8_t value);


#endif /* MFRC522_H_ */
