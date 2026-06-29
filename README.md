# FreeRTOS Command Shell — STM32F411RE


A bare-metal embedded systems project built on the STM32F411RE NUCLEO board that integrates multiple hardware peripherals under a FreeRTOS preemptive scheduler. This system utilizes a UART command shell, streams IMU sensor data, controls PWM-driven LED brightness, and performs RFID access control. All of this runs concurrently as independent RTOS tasks.

This is the fifth and final project in a series of embedded systems projects, with each one building on the previous. The previous four projects include: a UART command shell in PuTTY, an I2C IMU dashboard, a PWM/timer controller, and an SPI RFID reader.

---

## Hardware

- **MCU:** STM32F411RE (NUCLEO-F411RE)
- **IMU:** MPU-6500/9250 (I2C1, PB8/PB9)
- **RFID Reader:** MFRC522 clone (SPI1, PB3/PB4/PB5)
- **PWM LED:** Connected to TIM3 CH1 (PA6)
- **Auth LEDs:** PC2 (red), PC3 (green)
- **IDE:** STM32CubeIDE
- **RTOS:** FreeRTOS via CMSIS_V2

---

## System Architecture

The system runs four concurrent FreeRTOS tasks, each responsible for a distinct hardware subsystem:

| Task | Priority | Wake Condition | Responsibility |
|------|----------|---------|----------------|
| `uart_task` | High | Ring buffer | UART command shell, command dispatch |
| `mfrc522_task` | Above Normal | 100ms poll | RFID card detection, UID-based auth |
| `mpu_task` | Normal | Binary semaphore (TIM2 ISR) | IMU data read and stream |
| `pulse_task` | Low | Binary semaphore (TIM4 ISR) | PWM LED pulse animation |

### Shared Resource Protection

One mutex protect shared hardware:

- **`uart_mutex` (recursive):** Guards all `HAL_UART_Transmit` calls across tasks. Recursive because `uart_task` holds the mutex when dispatching commands that themselves transmit (e.g. `read` → `print_mpu_data`).

### ISR-to-Task Signaling

TIM2 and TIM4 use binary semaphores to signal tasks rather than polling volatile flags:

```c
void TIM2_IRQHandler(void)
{
    TIM2->SR &= ~0b1;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(data_ready_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

This ensures tasks block with zero CPU consumption between events, and `portYIELD_FROM_ISR` triggers an immediate context switch if a higher-priority task was unblocked.

---

## Peripheral Drivers

All drivers are written at the register level with minimal HAL usage.

### I2C (MPU-6500/9250)
- Bare-metal I2C1 init (PB8/PB9, AF4, open-drain, 100kHz)
- Single-byte read, single-byte write, burst read (14 bytes for full accel/gyro/temp)
- Fixed-point arithmetic for sensor conversion (avoids `snprintf` float hang under FreeRTOS/newlib-nano)

### SPI (MFRC522)
- Bare-metal SPI1 init (PB3/PB4/PB5, AF5, push-pull, CPOL=0 CPHA=0, 5.25MHz)
- Register read/write with MFRC522 address byte formatting (bit 7 = R/W, bits [6:1] = address)
- Full card detection sequence: REQA -> ATQA -> anticollision -> UID read with BCC checksum verification
- Clone chip handling (version register returns `0x18` instead of `0x91`/`0x92`)

### PWM (TIM3)
- TIM3 CH1 on PA6, AF2, 8-bit resolution (ARR=255), ~1kHz frequency
- Pulse animation driven by TIM4 interrupt at 25ms intervals

---

## UART Command Shell

The shell uses an interrupt-driven ring buffer for non-blocking receive. `HAL_UART_RxCpltCallback` writes each byte to the ring buffer and re-arms the interrupt. `uart_task` drains the buffer and dispatches complete commands.

### Supported Commands

| Command | Description |
|---------|-------------|
| `led on` / `led off` | Toggle onboard LED (PA5) |
| `status` | Report onboard LED state |
| `whoami` | Read MPU WHO_AM_I register |
| `read` | Single IMU data snapshot |
| `stream` / `stop` | Start/stop continuous IMU streaming |
| `dim <0-100>` | Set blue LED brightness by percentage |
| `pulse` / `stop_pulse` | Start/stop PWM pulse animation |
| `auth_leds_off` | Turn off RFID auth LEDs |

---

## RFID Access Control

Cards are identified by their 4-byte UID read via the ISO 14443 anticollision sequence. A known UID is stored at startup and compared using `memcmp`. A matching card turns on the green LED; any other card turns on the red LED.

The detection state machine prevents duplicate scans with debouncing:

```c
if(mfrc_state == 0 && mfrc522_read_uid(uid) == SPI_OK)
{
    mfrc_state = 1;
    last_detected_tick = HAL_GetTick();
    check_uid(uid);
}
else if(mfrc_state == 1 && HAL_GetTick() - last_detected_tick > 2000)
{
    mfrc_state = 0;
}
```

---

## Key Engineering Challenges

**FreeRTOS interrupt priority configuration:** All ISRs calling FreeRTOS `FromISR` APIs (TIM2, TIM4, USART2) must be set to a numerically higher priority than `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` (5). Default priority of 0 causes FreeRTOS to mask these interrupts during critical sections. Fixed by explicitly calling `HAL_NVIC_SetPriority` after peripheral init.

**Recursive mutex for UART:** `uart_task` holds `uart_mutex` while dispatching commands. The `read` command calls `print_mpu_data` which also acquires `uart_mutex` — a deadlock without a recursive mutex. Solved by creating `uart_mutex` with `osMutexRecursive` flag.

**Float formatting under FreeRTOS:** `snprintf` with `%.2f` hangs under FreeRTOS with newlib-nano due to stack consumption from the float math library. Replaced with fixed-point integer arithmetic using divide and modulo.

**Task stack sizing:** Each FreeRTOS task has a private fixed stack. Deep HAL call chains (uart → process_command → print_mpu_data → HAL_UART_Transmit) consumed more stack than default allocations. Stack overflow detection enabled via `configCHECK_FOR_STACK_OVERFLOW = 2` with `vApplicationStackOverflowHook`.

**Card UID read debouncing:** Software initally read multiple card scans every time a card was held to the MFRC522. Debouncing was introduced with `HAL_GetTick()` such that a scan is only read when enough time has passed from the last read.

---

## File Structure

```
src/src/
├── main.c              # Init, task creation, scheduler start
├── uart.c              # Ring buffer, command shell, uart_task
├── mpu.c               # MPU-6500 driver, mpu_task
├── mfrc522.c           # MFRC522 SPI driver, mfrc522_task
├── spi_init.c          # Bare-metal SPI1 init, transfer functions
├── i2c.c               # Bare-metal I2C1 init, read/write/burst
├── tim_init.c          # TIM2/TIM3/TIM4 init, ISR handlers, pulse_task
└── process_command.c   # Command dispatch and handlers
```

---

## Prior Projects in This Series

| Project | Focus |
|---------|-------|
| [UART Shell](https://github.com/nrossTCNJ/uart-shell) | Interrupt-driven RX, ring buffer, command dispatcher |
| [I2C Sensor Dashboard](https://github.com/nrossTCNJ/uart-shell-I2C-sensors) | Bare-metal I2C1, MPU-6500 burst read, non-blocking streaming |
| [PWM Timer Control](https://github.com/nrossTCNJ/uart-shell-timers) | TIM3 PWM, TIM4 interrupt-driven pulse state machine |
| [SPI RFID Reader](https://github.com/nrossTCNJ/uart-shell-SPI) | MFRC522 driver, UID-based access control, clone chip handling |
