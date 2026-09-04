#include "rgb_led.h"
#include "tim.h"
#include <math.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PULSE_PERIOD_MS   1500U   /* gentle ~1.5s breathing period for "charging" */

static uint32_t s_pulse_start_tick = 0;

void RGB_ChargeLED_Init(void)
{
    GPIO_InitTypeDef gi = {0};

    /* Red: plain push-pull output. Cathode is on the GPIO side, so
     * SET (HIGH) is off, RESET (LOW) is on -- start SET (off). */
    HAL_GPIO_WritePin(CHG_LED_R_GPIO_Port, CHG_LED_R_Pin, GPIO_PIN_SET);
    gi.Pin = CHG_LED_R_Pin;
    gi.Mode = GPIO_MODE_OUTPUT_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CHG_LED_R_GPIO_Port, &gi);

    /* Blue: unused for charge status, park as analog to save power /
     * avoid a floating pin. */
    gi.Pin = CHG_LED_B_Pin;
    gi.Mode = GPIO_MODE_ANALOG;
    gi.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CHG_LED_B_GPIO_Port, &gi);

    /* Green: PWM-capable via TIM12 CH1 (polarity already accounts
     * for the cathode-on-GPIO wiring -- see tim.c). */
    TIM12_ChargeLED_Init();
    TIM_SetChargeGreenDuty(0);

    s_pulse_start_tick = HAL_GetTick();
}

void RGB_ChargeLED_Red(bool on)
{
    /* Cathode on the GPIO side: RESET (LOW) sinks current = on. */
    HAL_GPIO_WritePin(CHG_LED_R_GPIO_Port, CHG_LED_R_Pin,
                       on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void RGB_ChargeLED_GreenSolid(void)
{
    TIM_SetChargeGreenDuty(255);
}

void RGB_ChargeLED_GreenOff(void)
{
    TIM_SetChargeGreenDuty(0);
}

void RGB_ChargeLED_GreenPulseUpdate(void)
{
    uint32_t elapsed = HAL_GetTick() - s_pulse_start_tick;
    float phase_frac = (float)(elapsed % PULSE_PERIOD_MS) / (float)PULSE_PERIOD_MS;
    float envelope = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * phase_frac)); /* 0..1 */
    TIM_SetChargeGreenDuty((uint8_t)(envelope * 255.0f + 0.5f));
}
