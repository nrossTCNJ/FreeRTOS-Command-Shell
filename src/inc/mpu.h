/*
 *  mpu.h
 *  Header for mpu.c file
 *  Created on: Jun 5, 2026
 *      Author: nate
 */

#ifndef MPU_H
#define MPU_H

#include "stm32f4xx.h"
#include "i2c.h"

typedef struct {
	int32_t accel_x, accel_y, accel_z;
	int32_t gyro_x, gyro_y, gyro_z;
	int32_t temp_c, temp_f;
} MPU_Data;

void mpu_init(void);
void mpu_task(void *pvParameters);
void stream_mpu(void);
void print_mpu_data();
uint8_t mpu_read(int16_t* data);
void mpu_convert(int16_t* raw, MPU_Data* out);

#endif
