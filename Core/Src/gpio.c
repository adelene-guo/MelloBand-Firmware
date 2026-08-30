#include "gpio.h"

void GPIO_Init(void)
{
    GPIO_InitTypeDef gi = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* MOTOR_EN (PA0) - shared hardware enable for U5/U6/U7. Start low
     * (drivers held in shutdown) until DRV2605_PowerUp() (called from
     * Haptic_NextPattern on the first button press) brings them up. */
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
    gi.Pin = MOTOR_EN_Pin;
    gi.Mode = GPIO_MODE_OUTPUT_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MOTOR_EN_GPIO_Port, &gi);

    /* Charge-status RGB LED (D3) is initialized by RGB_ChargeLED_Init()
     * in rgb_led.c (R is plain GPIO, G is TIM12 PWM, B is analog/idle)
     * -- not configured here, to avoid two files fighting over the
     * same pins. */

    /* Heartbeat/status LED (D5) on PB4 (MCU_STATUS_SIG) */
    HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_RESET);
    gi.Pin = STATUS_LED_Pin;
    gi.Mode = GPIO_MODE_OUTPUT_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STATUS_LED_GPIO_Port, &gi);

    /* CHARGE_STATUS_SIG input (PB12) - open-drain STAT from MCP73831,
     * externally pulled up by R9, no internal pull needed. */
    gi.Pin = CHG_STAT_Pin;
    gi.Mode = GPIO_MODE_INPUT;
    gi.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CHG_STAT_GPIO_Port, &gi);

    /* MODE_TOGGLE button input (PC13), plain polled input -- read
     * directly by Button_ConsumePressEvent() in button.c, no
     * interrupt/NVIC involved. */
    gi.Pin = BUTTON_Pin;
    gi.Mode = GPIO_MODE_INPUT;
    gi.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BUTTON_GPIO_Port, &gi);

    /* MOTOR_PWM_A/B/C (PA6/PA7/PB0): these physically connect to the
     * DRV2605 IN/TRIG pins, but this design drives motor amplitude
     * over I2C (Real-Time-Playback mode) instead, per haptic.c -- see
     * tim.h. Park them as analog so they're not left as
     * undriven/floating digital inputs. */
    gi.Pin = MOTOR_PWM_A_Pin;
    gi.Mode = GPIO_MODE_ANALOG;
    gi.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(MOTOR_PWM_A_GPIO_Port, &gi);   /* PA6 */

    gi.Pin = MOTOR_PWM_B_Pin;
    HAL_GPIO_Init(MOTOR_PWM_B_GPIO_Port, &gi);   /* PA7 */

    gi.Pin = MOTOR_PWM_C_Pin;
    HAL_GPIO_Init(MOTOR_PWM_C_GPIO_Port, &gi);   /* PB0 */

    /* Note: MODE_IND_R/G/B (PA8/PA9/PA10) get their timer alternate
     * function from TIM1_RGB_Init() in tim.c, CHARGE_STATUS_G
     * (PB14) from TIM12_ChargeLED_Init(), and MOTOR_SCL/SDA
     * (PB8/PB9) from I2C1_Init() in i2c.c -- not configured here, for
     * the same one-owner-per-pin reason as the charge LED above. */
}
