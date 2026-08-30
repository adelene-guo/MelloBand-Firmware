/**
 * main.c -- MINIMAL BRING-UP TEST
 *
 * Does exactly three things:
 *   1. Turns the mode-indicator RGB LED (D2) solid purple.
 *   2. Turns the charge-status RGB LED (D3) solid green (always -- no
 *      STAT reading yet).
 *   3. Drives all three DRV2605 motors via PWM at 50% duty for 3
 *      seconds, then stops.
 *
 * This intentionally bypasses gpio.c / haptic.c / charge_status.c /
 * button.c / rgb_led.c -- they're untouched in the project and will
 * be wired back in once this basic hardware bring-up is confirmed
 * working. TIM1 (mode LED), TIM3 (motor PWM), and I2C1 are still
 * reused from tim.c / i2c.c since those are just peripheral init and
 * not tied to the removed pattern/session logic.
 *
 * Note: PWM alone does not make the DRV2605 vibrate -- it powers up
 * in standby by default and ignores IN/TRIG until told over I2C to
 * enter PWM/analog-input mode. That's the one I2C write below.
 */

#include "main.h"
#include "tim.h"
#include "i2c.h"
#include "drv2605.h"

static void SystemClock_Config(void);
static void GPIO_BringUp_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    GPIO_BringUp_Init();
    TIM1_RGB_Init();
    TIM3_Motor_Init();
    I2C1_Init();

    /* Charge LED: solid green, always. */
    HAL_GPIO_WritePin(CHG_LED_R_GPIO_Port, CHG_LED_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CHG_LED_G_GPIO_Port, CHG_LED_G_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CHG_LED_B_GPIO_Port, CHG_LED_B_Pin, GPIO_PIN_RESET);

    /* Mode LED: solid purple. */
    TIM_SetModeLED(160, 0, 255);

    /* Power up the DRV2605s (shared EN line) and put them in
     * PWM/analog-input mode so they respond to the TIM3 PWM below.
     * All three chips share one I2C address on one bus, so this
     * single write configures all three at once. */
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(2); /* >250us required before first I2C txn after EN */
    DRV2605_WriteReg(DRV2605_REG_MODE, DRV2605_MODE_PWM_ANALOG);

    /* Motors on at 50% duty for 3 seconds. */
    TIM_SetMotorDuty(0, 50);
    TIM_SetMotorDuty(1, 50);
    TIM_SetMotorDuty(2, 50);
    HAL_Delay(3000);

    /* Motors off, drivers back into shutdown. */
    TIM_SetMotorDuty(0, 0);
    TIM_SetMotorDuty(1, 0);
    TIM_SetMotorDuty(2, 0);
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);

    while (1) { }
}

/* Only what this bring-up test needs: MOTOR_EN and the charge LED's
 * three plain-GPIO pins. (PA6/PA7/PB0 and PA8/PA9/PA10 get their AF
 * config from TIM3_Motor_Init()/TIM1_RGB_Init() above, and PB8/PB9
 * from I2C1_Init().) */
static void GPIO_BringUp_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gi = {0};
    gi.Mode = GPIO_MODE_OUTPUT_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
    gi.Pin = MOTOR_EN_Pin;
    HAL_GPIO_Init(MOTOR_EN_GPIO_Port, &gi);

    HAL_GPIO_WritePin(CHG_LED_R_GPIO_Port, CHG_LED_R_Pin, GPIO_PIN_RESET);
    gi.Pin = CHG_LED_R_Pin;
    HAL_GPIO_Init(CHG_LED_R_GPIO_Port, &gi);

    HAL_GPIO_WritePin(CHG_LED_G_GPIO_Port, CHG_LED_G_Pin, GPIO_PIN_RESET);
    gi.Pin = CHG_LED_G_Pin;
    HAL_GPIO_Init(CHG_LED_G_GPIO_Port, &gi);

    HAL_GPIO_WritePin(CHG_LED_B_GPIO_Port, CHG_LED_B_Pin, GPIO_PIN_RESET);
    gi.Pin = CHG_LED_B_Pin;
    HAL_GPIO_Init(CHG_LED_B_GPIO_Port, &gi);
}

/**
 * HSE (X1A/X1B, 25 MHz) -> PLL -> 100 MHz SYSCLK.
 * PLLM=25, PLLN=200, PLLP=2  =>  VCO = 25MHz/25*200 = 200MHz, SYSCLK = 200/2 = 100MHz
 * AHB /1 = 100 MHz, APB1 /2 = 50 MHz (timers x2 = 100 MHz), APB2 /1 = 100 MHz (timers = 100 MHz)
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM = 25;
    osc.PLL.PLLN = 200;
    osc.PLL.PLLP = RCC_PLLP_DIV2;
    osc.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }

    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif