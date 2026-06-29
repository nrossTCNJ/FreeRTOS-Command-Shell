/*
 *  process_command.h
 *  Header file for process_command.c
 *  Created on: Jun 16, 2026
 *      Author: nate
 */

#ifndef PROCESS_COMMAND_H_
#define PROCESS_COMMAND_H_

#include <stdint.h>
#include "stm32f4xx.h"
#include "cmsis_os2.h"
#include "main.h"
#include "i2c.h"
#include "mpu.h"
#include "tim_init.h"

void process_command(uint8_t* cmd);

void cmd_led_on(void);
void cmd_led_off(void);
void cmd_status(void);
void cmd_whoami(void);
void cmd_stream(void);
void cmd_stop(void);
void cmd_dim(int brightness);
void cmd_pulse(void);
void cmd_stop_pulse(void);
void cmd_auth_leds_off(void);
void cmd_unknown(void);

#endif /* PROCESS_COMMAND_H_ */
