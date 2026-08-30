#include "gpio.h"

/* Initialize board GPIOs used by the application (motors, LEDs,
 * charge-status input, and button EXTI). Timer/I2C alternate
 * configurations are performed in tim.c and i2c.c respectively. */
void GPIO_Init(void)
{
    GPIO_InitTypeDef gi = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* MOTOR_EN (PA0) - shared hardware enable for U5/U6/U7. Start low
     * (drivers held in shutdown) until DRV2605_InitAll() is ready to
     * bring them up, so we never chatter I2C at a powered-down chip. */
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
    gi.Pin = MOTOR_EN_Pin;
    gi.Mode = GPIO_MODE_OUTPUT_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MOTOR_EN_GPIO_Port, &gi);

    /* Charge-status RGB LED (D3) is common-anode: pin HIGH = channel off.
     * Drive all high so D3 stays dark during motor/button bring-up. */
    HAL_GPIO_WritePin(CHG_LED_R_GPIO_Port, CHG_LED_R_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CHG_LED_G_GPIO_Port, CHG_LED_G_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CHG_LED_B_GPIO_Port, CHG_LED_B_Pin, GPIO_PIN_SET);

    gi.Pin = CHG_LED_R_Pin;
    gi.Mode = GPIO_MODE_OUTPUT_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CHG_LED_R_GPIO_Port, &gi);

    gi.Pin = CHG_LED_G_Pin;
    HAL_GPIO_Init(CHG_LED_G_GPIO_Port, &gi);

    gi.Pin = CHG_LED_B_Pin;
    HAL_GPIO_Init(CHG_LED_B_GPIO_Port, &gi);

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

    /* MODE_TOGGLE (PC13) through the analog front end. Idle is ~0.6*VCC
     * (in the CMOS undefined band), so fire EXTI on either edge and
     * also poll in software -- see button.c. */
    gi.Pin = BUTTON_Pin;
    gi.Mode = GPIO_MODE_IT_RISING_FALLING;
    gi.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_GPIO_Port, &gi);

    HAL_NVIC_SetPriority(BUTTON_EXTI_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(BUTTON_EXTI_IRQn);

    /* Note: MOTOR_PWM_A/B/C (PA6/PA7/PB0) and MODE_IND_R/G/B
     * (PA8/PA9/PA10) are configured for their timer alternate
     * functions in tim.c, and MOTOR_SCL/SDA (PB8/PB9) for I2C1 in
     * i2c.c. */
}
