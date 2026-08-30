#include "rgb_led.h"
#include "tim.h"
#include <math.h>
#include "main.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PULSE_PERIOD_MS   1500U   /* gentle ~1.5s breathing period for "charging" */

static uint32_t s_pulse_start_tick = 0;

/* Initialize GPIOs and timers used by the charge-status RGB LED. */
void RGB_ChargeLED_Init(void)
{
    GPIO_InitTypeDef gi = {0};

    /* Red: plain push-pull output. */
    HAL_GPIO_WritePin(CHG_LED_R_GPIO_Port, CHG_LED_R_Pin, GPIO_PIN_RESET);
    gi.Pin = CHG_LED_R_Pin;
    gi.Mode = GPIO_MODE_OUTPUT_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CHG_LED_R_GPIO_Port, &gi);

    /* Blue: unused for charge status, park as analog to save power */
    gi.Pin = CHG_LED_B_Pin;
    gi.Mode = GPIO_MODE_ANALOG;
    gi.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CHG_LED_B_GPIO_Port, &gi);

    /* Green: PWM-capable via TIM12 CH1. */
    TIM12_ChargeLED_Init();
    TIM_SetChargeGreenDuty(0);

    s_pulse_start_tick = HAL_GetTick();
}

/* Turn the red LED channel on or off. */
void RGB_ChargeLED_Red(bool on)
{
    HAL_GPIO_WritePin(CHG_LED_R_GPIO_Port, CHG_LED_R_Pin,
                       on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* Set green channel fully on (solid). */
void RGB_ChargeLED_GreenSolid(void)
{
    TIM_SetChargeGreenDuty(255);
}

/* Turn the green channel fully off. */
void RGB_ChargeLED_GreenOff(void)
{
    TIM_SetChargeGreenDuty(0);
}

/* Update the green PWM duty to create a smooth breathing/pulse effect.
 * Call frequently (e.g., every main-loop tick) while in charging state. */
void RGB_ChargeLED_GreenPulseUpdate(void)
{
    uint32_t elapsed = HAL_GetTick() - s_pulse_start_tick;
    float phase_frac = (float)(elapsed % PULSE_PERIOD_MS) / (float)PULSE_PERIOD_MS;
    float envelope = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * phase_frac)); /* 0..1 */
    TIM_SetChargeGreenDuty((uint8_t)(envelope * 255.0f + 0.5f));
}
