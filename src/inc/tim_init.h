/*
 *  tim3_pwm_init.h
 *  Header file for tim3_pwm_init.c
 *  Created on: Jun 12, 2026
 *      Author: nate
 */

#ifndef TIM_INIT_H_
#define TIM_INIT_H_

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "semphr.h"

void pulse_task(void *pvParameters);
void pulse_led(void);
void tim3_init(void);
void tim2_init(void);
void TIM2_IRQHandler(void);
void tim4_init(void);
void TIM4_IRQHandler(void);

extern volatile uint8_t data_ready;
extern volatile uint8_t pulse_ready;

#endif
