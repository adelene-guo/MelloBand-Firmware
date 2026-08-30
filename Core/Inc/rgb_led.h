#ifndef __RGB_LED_H
#define __RGB_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

/* Charge-status RGB LED (D3):
 *   - Red   (PA5)  : plain GPIO on/off -- "no battery"
 *   - Green (PB14) : PWM-dimmable (TIM12 CH1) -- solid for "battery
 *                     OK", breathing/pulsating for "charging"
 *   - Blue  (PB15) : unused by the current charge-status scheme, left
 *                     off (configured as analog in gpio.c)
 */
void RGB_ChargeLED_Init(void);

void RGB_ChargeLED_Red(bool on);
void RGB_ChargeLED_GreenSolid(void);
void RGB_ChargeLED_GreenOff(void);

/* Called periodically (e.g. every main-loop tick) while in the
 * charging state to animate a gentle ~1.5s-period green pulse. */
void RGB_ChargeLED_GreenPulseUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* __RGB_LED_H */
