#ifndef __I2C_H
#define __I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "i2c.h"
#include "stm32f4xx_hal_i2c.h"

extern I2C_HandleTypeDef hi2c1;

/* I2C1 on PB8 (SCL) / PB9 (SDA), 400 kHz fast mode. This single bus is
 * physically wired to all three DRV2605L drivers (U5/U6/U7) in
 * parallel, and DRV2605L has NO address-select pins -- every device
 * answers to the same fixed 7-bit address (0x5A). There is therefore
 * no way to address one driver individually over I2C on this board;
 * every I2C transaction is effectively a broadcast to all three. See
 * drv2605.c for how the firmware works around this (I2C only used for
 * one-time identical configuration; per-motor control happens via the
 * independent PWM lines instead). */
void I2C1_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H */
