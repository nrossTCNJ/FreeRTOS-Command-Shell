/*
 *  tim_init.c
 *  Initializes timers for use
 *  Created on: Jun 12, 2026
 *      Author: nate
 */

#include "tim_init.h"

#define TICKS_PER_SECOND 65522
#define PULSE_INTERVAL_SEC 0.025

extern osSemaphoreId_t data_ready_sem;
extern osSemaphoreId_t pulse_ready_sem;
uint8_t led_state;
volatile uint8_t pwm_val;
volatile uint8_t pulse_state;


// initializes blue pulse LED for dim command to change brightness levels
void tim3_init(void)
{
	// peripheral clock enables
	RCC->AHB1ENR |= 1;					// GPIOA
	RCC->APB1ENR |= (1 << 1); 			// TIM3

	// configure GPIOA as alternate function
	GPIOA->MODER &= ~(0b11 << 12);
	GPIOA->MODER |= (0b10 << 12);		// AF mode

	// set to push-pull
	GPIOA->OTYPER &= ~(1 << 6);

	// set to no pull-up pull-down
	GPIOA->PUPDR &= ~(0b11 << 12);

	// configure AF02 for PA6
	GPIOA->AFR[0] &= ~(0xF << 24);
	GPIOA->AFR[0] |= (0x2 << 24);

	// configure timer base
	TIM3->PSC &= ~(0xFFFF);
	TIM3->PSC |= (0b0000000101000111);	// set PSC to 327
	TIM3->ARR &= ~(0xFFFF);
	TIM3->ARR |= (0xFF);				// set ARR to 255

	// configure PWM channel
	TIM3->CCMR1 &= ~(0b111 << 4);
	TIM3->CCMR1 |= (0b110 << 4);		// PWM Mode 1
	TIM3->CCMR1 |= (1 << 3);			// enable preload register
	TIM3->CCR1 &= ~(0xFFFF);
	TIM3->CCR1 |= (0x80);				// CCR1 = 128, ~50% duty cycle

	// enable output channel
	TIM3->CCER &= ~(0b11);
	TIM3->CCER |= (0b01);				// enable channel active high

	// enable auto-reload preload
	TIM3->CR1 |= (1 << 7);

	// force update for PSC/ARR/preload values
	TIM3->EGR |= 0b1;					// set UG (update generation)

	// start timer
	TIM3->CR1 |= 0b1;					// set CEN (counter enable)
}

// timer for MPU 6500 sensor data streaming
void tim2_init(void)
{
	RCC->APB1ENR |= (1 << 0); 			// TIM2 peripheral clock
	TIM2->CR1 = 0;						// reset timer configuration
	TIM2->PSC = 1281; 					// PSC = 1281
	TIM2->ARR = (2* TICKS_PER_SECOND); 	// period = seconds * 65522
	TIM2->EGR |= 1; 					// Force update event
	TIM2->SR &= ~0b1;					// clear UIF early to avoid missing future update events
	TIM2->DIER |= 0b1; 					// update interrupt enable
	TIM2->CR1 |= (1 << 7) | 1; 			// enable ARR preload (ARPE) and start timer (CEN)
	NVIC->ISER[0] |= (1 << 28);			// enable TIM2 interrupt in the NVIC
}

void TIM2_IRQHandler(void)
{
	TIM2->SR &= ~0b1;	// clear UIF early to avoid missing future update events
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(data_ready_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

// timer for blue LED pulse command
void tim4_init(void)
{
	RCC->APB1ENR |= (1 << 2);		// TIM 4 peripheral clock
	TIM4->CR1 = 0;
	TIM4->PSC = 1281;
	TIM4->ARR = (PULSE_INTERVAL_SEC * TICKS_PER_SECOND);
	TIM4->EGR |= 1;
	TIM4->SR &= ~0b1;
	TIM4->DIER |= 0b1;
	TIM4->CR1 |= (1 << 7) | 1;
	NVIC->ISER[0] |= (1 << 30);
}
void TIM4_IRQHandler(void)
{
	TIM4->SR &= ~0b1;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(pulse_ready_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// task for FreeRTOS to access pulsing
void pulse_task(void *pvParameters)
{
	while(1)
	{
		osSemaphoreAcquire(pulse_ready_sem, osWaitForever);
		pulse_led();
	}
}

// state machine for LED pulsing
void pulse_led(void)
{
	if(pulse_state != 0)
	{
		// pulse based on state
		if(pulse_state == 1)
			pwm_val++;
		else if(pulse_state == 2)
			pwm_val--;

		// change state
		if(pwm_val == 0)
			pulse_state = 1;
		else if(pwm_val == 128)	 // only goes half to be quicker
			pulse_state = 2;

		TIM3->CCR1 = pwm_val;
	}
}
