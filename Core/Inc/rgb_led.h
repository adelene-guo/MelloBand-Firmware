/*
 * rgb_led.h
 *
 *  Created on: Sep 4, 2026
 *      Author: guoad
 */

#ifndef INC_RGB_LED_H_
#define INC_RGB_LED_H_

#include "rgb_led.h"
#include "tim.h"
#include <math.h>
#include <stdbool.h>


void RGB_ChargeLED_Init(void);
void RGB_ChargeLED_Red(bool on);
void RGB_ChargeLED_GreenSolid(void);
void RGB_ChargeLED_GreenOff(void);
void RGB_ChargeLED_GreenPulseUpdate(void);

#endif /* INC_RGB_LED_H_ */
