/**
 * main.c -- MINIMAL MOTOR-PWM-ONLY TEST (looping)
 *
 * Isolates just the DRV2605 PWM path: no button, no mode LED
 * cycling, no charge LED. The mode LED is only used here as a
 * pass/fail indicator so you can tell "I2C never ACKed" apart from
 * "I2C worked but the motors don't move":
 *
 *   RED     (solid, stays)     -- DRV2605_EnterPWMMode() failed (I2C
 *                                  error). Check wiring/pull-ups
 *                                  before looking anywhere else -- if
 *                                  this is red, the PWM step below
 *                                  never even runs, ever.
 *   GREEN   (2s, repeating)    -- motors driven at 100% PWM duty.
 *   OFF     (1s, repeating)    -- motors off, in between buzzes.
 *
 * This now loops forever (green 2s / off 1s) instead of running once,
 * specifically so you can't miss the buzz window by looking at the
 * board a few seconds late. If you watch it cycle green/off
 * repeatedly and still never feel/hear anything, I2C configuration is
 * confirmed working and the problem is downstream:
 *   - Check DRV2605 OUT+/OUT- actually reach the motor terminals.
 *   - Check MOTOR_EN (PA0) really reaches all three drivers' EN pins
 *     (continuity), not just one.
 *   - Check the motors themselves aren't dead/disconnected.
 *   - Bumped to 100% duty here (from 50%) in case duty was below a
 *     motor's minimum start voltage -- if it buzzes now but not at
 *     50%, that's your answer.
 */

#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "i2c.h"
#include "drv2605.h"

static void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    GPIO_Init();
    TIM1_RGB_Init();     /* mode LED, used here only as a pass/fail indicator */
    TIM3_Motor_Init();   /* motor PWM on PA6/PA7/PB0 */
    I2C1_Init();

    if (DRV2605_EnterPWMMode() != HAL_OK) {
        TIM_SetModeLED(255, 0, 0);   /* red: I2C failed, stop here for good */
        while (1) { }
    }

    while (1) {
        TIM_SetModeLED(0, 255, 0);   /* green: buzzing */
        TIM_SetMotorDuty(0, 100);
        TIM_SetMotorDuty(1, 100);
        TIM_SetMotorDuty(2, 100);
        HAL_Delay(2000);

        TIM_SetModeLED(0, 0, 0);     /* off: paused between buzzes */
        TIM_SetMotorDuty(0, 0);
        TIM_SetMotorDuty(1, 0);
        TIM_SetMotorDuty(2, 0);
        HAL_Delay(1000);
    }
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
