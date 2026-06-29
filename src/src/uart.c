/*
 *  uart.c
 *  Handles uart functions
 *  Created on: Jun 28, 2026
 *      Author: nate
 */

#include "uart.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern UART_HandleTypeDef huart2;
extern osMutexId_t uart_mutex;

volatile uint8_t head;
volatile uint8_t tail;
uint8_t receive[1];
uint8_t buffer[BUFFER_SIZE];


// UART task for FreeRTOS to use command shell
void uart_task(void *pvParameters)
{
    uint8_t cmd_buff[CMD_SIZE];
    uint8_t cmd_index = 0;
    HAL_UART_Receive_IT(&huart2, receive, 1);
    while(1)
    {
        handle_uart_cmd(cmd_buff, &cmd_index);
        osDelay(1);
    }
}

// handles every keystroke of uart shell
void handle_uart_cmd(uint8_t *cmd_buff, uint8_t *cmd_index)
{
	if(!ring_empty())
	{
		uint8_t byte = ring_read();

		osMutexAcquire(uart_mutex, osWaitForever);

		// New line
		if(byte == '\r' || byte == '\n')
		{
			cmd_buff[*cmd_index] = '\0';
			process_command(cmd_buff);
			*cmd_index = 0;
			memset(cmd_buff, 0, CMD_SIZE);
		}
		// Backspace
		else if(byte == '\b' || byte == 0x7F)
		{
			if(*cmd_index != 0)
			{
				(*cmd_index)--;
				cmd_buff[*cmd_index] = 0;
				uint8_t bs_seq[] = "\b \b";
				HAL_UART_Transmit(&huart2, bs_seq, 3, HAL_MAX_DELAY);
			}
		}
		// Normal character, buffer isn't full
		else if(*cmd_index < CMD_SIZE - 1)
		{
			cmd_buff[*cmd_index] = byte;
			(*cmd_index)++;
			HAL_UART_Transmit(&huart2, &byte, 1, HAL_MAX_DELAY);
		}

		osMutexRelease(uart_mutex);
	}
}

// writes to ring buffer
void ring_write(uint8_t byte)
{
	// checks if buffer is not full
	if (head != (tail - 1 + BUFFER_SIZE) % BUFFER_SIZE)
	{
		// writes to head of ring buffer
		buffer[head] = byte;
		head = (head + 1) % BUFFER_SIZE; // wrap-around
	}
}

// reads data from ring buffer
uint8_t ring_read(void)
{
	// checks if empty buffer
	if (head == tail)
	{
		return 0;
	}
	// reads current byte (tail) and advances
	else
	{
		uint8_t byte = buffer[tail];
		tail = (tail + 1) % BUFFER_SIZE; // wrap-around
		return byte;
	}
}

// checks status of ring buffer
uint8_t ring_empty(void)
{
	// returns true if ring is empty
	if (head == tail)
		return 1;
	else
		return 0;
}

// re-arms byte for receiving
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// checks for USART 2
	if(huart->Instance == USART2)
	{
		ring_write(receive[0]);
		HAL_UART_Receive_IT(huart, receive, 1);
	}
}
