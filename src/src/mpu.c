/*
 *  mpu.c
 *  MPU 6500 sensor initialization
 *  Created on: Jun 5, 2026
 *      Author: nate
 */

#include "mpu.h"
#include "main.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define MPU_ADDR 0x68
extern UART_HandleTypeDef huart2;
extern osMutexId_t uart_mutex;
extern osSemaphoreId_t data_ready_sem;

uint8_t streaming;

void mpu_init(void)
{
	streaming = 0;

	// wake up MPU from sleep
	i2c_write(MPU_ADDR, 0x6B, 0x00);

	// configure sensor registers
	i2c_write(MPU_ADDR, 0x1B, 0x00);	// gyro
	i2c_write(MPU_ADDR, 0x1C, 0x00);	// accel
}

// task for FreeRTOS to use MPU 6500
void mpu_task(void *pvParameters)
{
    while(1)
    {
        osSemaphoreAcquire(data_ready_sem, osWaitForever);
        stream_mpu();
    }
}

// checks for streaming flag
void stream_mpu(void)
{
	if(streaming)
	{
		print_mpu_data();
		HAL_UART_Transmit(&huart2, "\r\n", 2, HAL_MAX_DELAY);
	}
}

// prints all sensor data in standard units
void print_mpu_data()
{
	int16_t data[7];
	char msg[64];
	uint8_t err = mpu_read(data);

	osMutexAcquire(uart_mutex, osWaitForever); // wait for UART availability

	if(err != I2C_OK)
	{
		snprintf(msg, sizeof(msg), "> Error\r\n");
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
	else
	{
		MPU_Data mpu_data;
		mpu_convert(data, &mpu_data);

		snprintf(msg, sizeof(msg), "> Accel X: %ld.%02ld\r\n",
				mpu_data.accel_x / 100,
				mpu_data.accel_x < 0 ? -(mpu_data.accel_x % 100) : mpu_data.accel_x % 100);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Accel Y: %ld.%02ld\r\n",
				mpu_data.accel_y / 100,
				mpu_data.accel_y < 0 ? -(mpu_data.accel_y % 100) : mpu_data.accel_y % 100);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Accel Z: %ld.%02ld\r\n",
				mpu_data.accel_z / 100,
				mpu_data.accel_z < 0 ? -(mpu_data.accel_z % 100) : mpu_data.accel_z % 100);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Temp (C): %ld.%02ld\r\n",
				mpu_data.temp_c / 100,
				mpu_data.temp_c < 0 ? -(mpu_data.temp_c % 100) : mpu_data.temp_c % 100);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Temp (F): %ld.%02ld\r\n",
				mpu_data.temp_f / 100,
				mpu_data.temp_f < 0 ? -(mpu_data.temp_f % 100) : mpu_data.temp_f % 100);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Gyro X: %ld.%02ld\r\n",
				mpu_data.gyro_x / 100,
				mpu_data.gyro_x < 0 ? -(mpu_data.gyro_x % 100) : mpu_data.gyro_x % 100);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Gyro Y: %ld.%02ld\r\n",
				mpu_data.gyro_y / 100,
				mpu_data.gyro_y < 0 ? -(mpu_data.gyro_y % 100) : mpu_data.gyro_y % 100);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Gyro Z: %ld.%02ld\r\n",
				mpu_data.gyro_z / 100,
				mpu_data.gyro_z < 0 ? -(mpu_data.gyro_z % 100) : mpu_data.gyro_z % 100);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}

	osMutexRelease(uart_mutex);
}

// reads device sensor data
uint8_t mpu_read(int16_t* data)
{
	// reads all 14 bytes for gyro, temp, and accel data
	uint8_t buffer[14];
	if (i2c_read_burst(MPU_ADDR, 0x3B, buffer, 14) == 0)
	{
		for(int i = 0; i < 14; i+=2)
		{
			data[i/2] = buffer[i+1] | buffer[i] << 8;
		}

		return I2C_OK;
	}
	else
		return I2C_ERROR;
}

// converts raw data into physical units
void mpu_convert(int16_t* raw, MPU_Data* out)
{
	// converts MPU data into physical units
	out->accel_x = (int32_t)(raw[0]*100) / 16384;
	out->accel_y = (int32_t)(raw[1]*100) / 16384;
	out->accel_z = (int32_t)(raw[2]*100) / 16384;
	out->temp_c = (int32_t)(raw[3]*100 / 333) + 2100;
	out->temp_f = (out->temp_c * 9 / 5) + 3200;
	out->gyro_x = (int32_t)raw[4] / 131;
	out->gyro_y = (int32_t)(raw[5]*100) / 131;
	out->gyro_z = (int32_t)(raw[6]*100) / 131;
}
