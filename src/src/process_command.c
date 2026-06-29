/*
 *  process_command.c
 *  Processes all of the user commands in one clean file
 *  Created on: Jun 16, 2026
 *      Author: nate
 */

#include "process_command.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern UART_HandleTypeDef huart2;
extern uint8_t led_state;
extern uint8_t streaming;
extern volatile uint8_t pwm_val;
extern volatile uint8_t pulse_state;

extern void print_mpu_data(void);

void process_command(uint8_t* cmd)
{
	uint8_t newline[] = "\r\n";
	HAL_UART_Transmit(&huart2, newline, 2, HAL_MAX_DELAY);
	int brightness;

	if(strcmp((char*)cmd, "led on") == 0) 				// turns led on
		cmd_led_on();

	else if(strcmp((char*)cmd, "led off") == 0) 		// turns led off
		cmd_led_off();

	else if(strcmp((char*)cmd, "status") == 0) 			// fetches led status
		cmd_status();

	else if(strcmp((char*)cmd, "whoami") == 0)			// returns MPU's identity
		cmd_whoami();

	else if(strcmp((char*)cmd, "read") == 0)			// reads accel, temp, and gyro data from MPU
		print_mpu_data();

	else if(strcmp((char*)cmd, "stream") == 0)			// constantly streams MPU sensor data
		cmd_stream();

	else if(strcmp((char*)cmd, "stop") == 0)			// stops stream of MPU sensor data
		cmd_stop();

	else if(sscanf(cmd, "dim %d", &brightness) == 1)	// change attached LED brightness
		cmd_dim(brightness);

	else if(strcmp((char*)cmd, "pulse") == 0)			// sets LED state to pulse
		cmd_pulse();

	else if (strcmp((char*)cmd, "stop_pulse") == 0)		// sets LED state to normal
		cmd_stop_pulse();

	else if (strcmp((char*)cmd, "auth_leds_off") == 0)
		cmd_auth_leds_off();
	else cmd_unknown();									// unknown command entered
}

void cmd_led_on(void)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	led_state = 1;
	uint8_t msg[] = "> LED ON\r\n";
	HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
}
void cmd_led_off(void)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	led_state = 0;
	uint8_t msg[] = "> LED OFF\r\n";
	HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
}
void cmd_status(void)
{
	if(led_state)
	{
		uint8_t msg[] = "> LED is ON\r\n";
		HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
	}
	else
	{
		uint8_t msg[] = "> LED is OFF\r\n";
		HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
	}
}
void cmd_whoami(void)
{
	uint8_t result = 0;
	char msg[32];
	uint8_t err = i2c_read(0x68, 0x75, &result);
	if(err != I2C_OK)
	{
		snprintf(msg, sizeof(msg), "> Error\r\n");
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
	else
	{
		snprintf(msg, sizeof(msg), "> WHO_AM_I: 0x%02X\r\n", result);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
}
void cmd_stream(void)
{
	streaming = 1;
	char msg[64];
	snprintf(msg, sizeof(msg), "> Streaming MPU Data\r\n");
	HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
}
void cmd_stop(void)
{
	streaming = 0;
	char msg[64];
	snprintf(msg, sizeof(msg), "> Stopping Data Stream\r\n");
	HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
}
void cmd_dim(int brightness)
{
	if(brightness >= 0 && brightness <= 100)
	{
		uint8_t val = brightness * 255 / 100;
		TIM3->CCR1 = val;
		pwm_val = val;

		char msg[64];
		snprintf(msg, sizeof(msg), "> Changed brightness to %d%%\r\n", brightness);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
	else
	{
		char msg[64];
		snprintf(msg, sizeof(msg), "> Invalid number, chose 0-100\r\n");
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
}
void cmd_pulse(void)
{
	pwm_val = 0;
	TIM3->CCR1 = 0;
	pulse_state = 1;
	char msg[64];
	snprintf(msg, sizeof(msg), "> Pulsing LED\r\n");
	HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
}
void cmd_stop_pulse(void)
{
	pulse_state = 0;
	char msg[64];
	snprintf(msg, sizeof(msg), "> Stopping LED Pulse\r\n");
	HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
}
void cmd_auth_leds_off(void)
{
	GPIOC->ODR &= ~(1 << 2);	// turn red led off
	GPIOC->ODR &= ~(1 << 3);	// turn green led off
	char msg[64];
	snprintf(msg, sizeof(msg), "> Authorization LEDs off\r\n");
	HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
}
void cmd_unknown(void)
{
	uint8_t msg[] = "> unknown command\r\n";
	HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
}

