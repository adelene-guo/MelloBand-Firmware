#include "tim.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim12;

/* TIM1, TIM3 (APB1, x2 since APB1 prescaler != 1), and TIM12 (also
 * APB1) all run from a 100 MHz timer clock in this project's clock
 * tree -- see SystemClock_Config() in main.c (HSE 25 MHz -> PLL ->
 * 100 MHz SYSCLK, APB1 /2, APB2 /1). */

void TIM1_RGB_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();

    GPIO_InitTypeDef gi = {0};
    gi.Pin = MODE_LED_R_Pin | MODE_LED_G_Pin | MODE_LED_B_Pin;
    gi.Mode = GPIO_MODE_AF_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    gi.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &gi);

    /* 100 MHz / 100 / 256 ~= 3.9 kHz PWM, 8-bit (0-255) duty steps. */
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 99;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 255;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }

    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 0;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    oc.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    oc.OCIdleState = TIM_OCIDLESTATE_RESET;
    oc.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    if (HAL_TIM_PWM_ConfigChannel(&htim1, &oc, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &oc, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &oc, TIM_CHANNEL_3) != HAL_OK) Error_Handler();

    /* TIM1 is an advanced-control timer: PWM outputs stay disabled
     * until the main output enable is set. */
    __HAL_TIM_MOE_ENABLE(&htim1);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

void TIM_SetModeLED(uint8_t r, uint8_t g, uint8_t b)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, r);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, g);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, b);
}

void TIM3_Motor_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    GPIO_InitTypeDef gi = {0};
    gi.Mode = GPIO_MODE_AF_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_MEDIUM;
    gi.Alternate = GPIO_AF2_TIM3;

    gi.Pin = MOTOR_PWM_A_Pin | MOTOR_PWM_B_Pin;
    HAL_GPIO_Init(GPIOA, &gi);

    gi.Pin = MOTOR_PWM_C_Pin;
    HAL_GPIO_Init(GPIOB, &gi);

    /* 100 MHz / 50 / 100 = 20 kHz PWM, duty steps 0-100 map 1:1 to
     * percent -- comfortably inside the DRV2605 PWM-input frequency
     * range. */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 49;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 99;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
        Error_Handler();
    }

    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 0;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_3) != HAL_OK) Error_Handler();

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);   /* MOTOR_PWM_A */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);   /* MOTOR_PWM_B */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);   /* MOTOR_PWM_C */
}

void TIM_SetMotorDuty(uint8_t motor_index, uint8_t duty_pct)
{
    if (duty_pct > 100) duty_pct = 100;

    uint32_t channel;
    switch (motor_index) {
        case 0: channel = TIM_CHANNEL_1; break;   /* MOTOR_PWM_A */
        case 1: channel = TIM_CHANNEL_2; break;   /* MOTOR_PWM_B */
        case 2: channel = TIM_CHANNEL_3; break;   /* MOTOR_PWM_C */
        default: return;
    }
    __HAL_TIM_SET_COMPARE(&htim3, channel, duty_pct);
}

void TIM12_ChargeLED_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM12_CLK_ENABLE();

    GPIO_InitTypeDef gi = {0};
    gi.Pin = CHG_LED_G_Pin;
    gi.Mode = GPIO_MODE_AF_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    gi.Alternate = GPIO_AF9_TIM12;
    HAL_GPIO_Init(GPIOB, &gi);

    /* Same 3.9 kHz / 8-bit setup as TIM1, just for one channel. */
    htim12.Instance = TIM12;
    htim12.Init.Prescaler = 99;
    htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim12.Init.Period = 255;
    htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim12) != HAL_OK) {
        Error_Handler();
    }

    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 0;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim12, &oc, TIM_CHANNEL_1) != HAL_OK) Error_Handler();

    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);
}

void TIM_SetChargeGreenDuty(uint8_t duty)
{
    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, duty);
}
