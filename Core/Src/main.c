/**
 * main.c -- MOTOR PWM + BUTTON-DRIVEN MODE LED
 *
 * Consolidates the validated DRV2605 PWM-mode motor path with
 * button-driven mode LED color cycling:
 *
 *   Boot:
 *     - DRV2605_EnterPWMMode() configures all three drivers. On
 *       failure, mode LED goes solid RED and the program halts there
 *       -- same pass/fail signal as the standalone motor test.
 *     - On success, mode LED starts Teal, motors idle.
 *
 *   Each MODE_TOGGLE button press:
 *     - Mode LED cycles Teal -> Blue -> Purple -> Teal -> ...
 *     - All three motors buzz at 100% duty for ~300ms as haptic
 *       confirmation of the press, then go back to idle.
 *
 * Still intentionally bypasses haptic.c (RTP-register breathing
 * patterns) -- that's a separate, not-yet-validated control path;
 * this file only uses the PWM-mode path that's now confirmed working.
 * charge_status.c / rgb_led.c are likewise not wired in here.
 */

#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "i2c.h"
#include "drv2605.h"
#include "button.h"

static void SystemClock_Config(void);
static void BuzzMotors(uint16_t duration_ms);

/* Teal -> Blue -> Purple, cycled by button press. */
typedef struct { uint8_t r, g, b; } rgb_t;
static const rgb_t s_mode_colors[3] = {
    { 0, 180, 180 },   /* Teal   */
    { 0,   0, 255 },   /* Blue   */
    { 160,  0, 255 },  /* Purple */
};
#define NUM_MODE_COLORS  (sizeof(s_mode_colors) / sizeof(s_mode_colors[0]))
static uint8_t s_mode_color_idx = 0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    GPIO_Init();
    TIM1_RGB_Init();     /* mode LED */
    TIM3_Motor_Init();   /* motor PWM on PA6/PA7/PB0 */
    I2C1_Init();
    Button_Init();

    if (DRV2605_EnterPWMMode() != HAL_OK) {
        TIM_SetModeLED(255, 0, 0);   /* red: I2C failed, stop here for good */
        while (1) { }
    }

    /* Mode LED: start on the first color in the cycle (Teal). Motors
     * stay idle until the first button press. */
    s_mode_color_idx = 0;
    TIM_SetModeLED(s_mode_colors[0].r, s_mode_colors[0].g, s_mode_colors[0].b);

    while (1) {
        if (Button_ConsumePressEvent()) {
            s_mode_color_idx = (uint8_t)((s_mode_color_idx + 1) % NUM_MODE_COLORS);
            const rgb_t *c = &s_mode_colors[s_mode_color_idx];
            TIM_SetModeLED(c->r, c->g, c->b);

            BuzzMotors(300);   /* haptic confirmation of the press */
        }
        HAL_Delay(5);
    }
}

/* All three motors on at 100% duty for duration_ms, then off. Blocks
 * for the duration -- fine for a short confirmation buzz, but note
 * button presses aren't polled again until this returns. */
static void BuzzMotors(uint16_t duration_ms)
{
    TIM_SetMotorDuty(0, 100);
    TIM_SetMotorDuty(1, 100);
    TIM_SetMotorDuty(2, 100);
    HAL_Delay(duration_ms);

    TIM_SetMotorDuty(0, 0);
    TIM_SetMotorDuty(1, 0);
    TIM_SetMotorDuty(2, 0);
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
