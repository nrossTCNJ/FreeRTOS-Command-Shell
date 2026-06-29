/*
 *  uart.h
 *  Header file for uart.c
 *  Created on: Jun 28, 2026
 *      Author: nate
 */

#ifndef UART_H_
#define UART_H_

#include "stm32f4xx.h"
#include "cmsis_os2.h"
#include "main.h"
#include "process_command.h"

#define BUFFER_SIZE 64
#define CMD_SIZE 32

void uart_task(void *pvParameters);
void handle_uart_cmd(uint8_t *cmd_buff, uint8_t *cmd_index);
void ring_write(uint8_t byte);
uint8_t ring_read(void);
uint8_t ring_empty(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif /* UART_H_ */
