#ifndef __TIM_H
#define __TIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern TIM_HandleTypeDef htim1;    /* MODE_IND RGB LED, PA8/PA9/PA10 */
extern TIM_HandleTypeDef htim3;    /* Motor A/B/C PWM, PA6/PA7/PB0 (bring-up test only) */
extern TIM_HandleTypeDef htim12;   /* Charge-status green LED pulse, PB14 (unused in bring-up test) */

/* TIM1: ~3.9 kHz PWM, 8-bit duty (0-255) -> smooth RGB brightness mixing
 * for the mode-indicator LED (D2). */
void TIM1_RGB_Init(void);
void TIM_SetModeLED(uint8_t r, uint8_t g, uint8_t b);

/* TIM3: 20 kHz PWM, duty 0-100 (%) on each channel -> DRV2605 IN/TRIG
 * pins, for use while the DRV2605 is in PWM/analog-input mode. Note:
 * the full breathing-pattern design (haptic.c) instead drives motor
 * amplitude over I2C via the DRV2605 real-time-playback register, not
 * PWM -- this timer is only wired up for the current minimal
 * PWM bring-up test in main.c. */
void TIM3_Motor_Init(void);
void TIM_SetMotorDuty(uint8_t motor_index /*0=A,1=B,2=C*/, uint8_t duty_pct);

/* TIM12 CH1 (PB14 / CHARGE_STATUS_G): single-channel PWM used only to
 * make the charge LED's green channel dimmable, so it can "pulsate"
 * while charging. Not used by the current minimal bring-up test
 * (charge LED is just plain GPIO there) -- kept for when
 * charge_status.c / rgb_led.c are reintroduced. */
void TIM12_ChargeLED_Init(void);
void TIM_SetChargeGreenDuty(uint8_t duty /* 0-255 */);

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H */
