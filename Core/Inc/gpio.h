#ifndef __GPIO_H
#define __GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Configures every non-timer, non-I2C GPIO used on this board:
 * MOTOR_EN, both status/indicator LED banks (as plain outputs -- the
 * RGB PWM-capable pins are configured separately in tim.c), the
 * charge-status input, and the button input/EXTI. */
void GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H */
