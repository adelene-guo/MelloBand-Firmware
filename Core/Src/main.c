/**
 * main.c -- BRING-UP TEST + MODE LED COLOR CYCLE
 *
 * On boot:
 *   1. Charge-status RGB LED (D3) goes solid green (always -- no STAT
 *      reading yet).
 *   2. Mode-indicator RGB LED (D2) starts Teal.
 *   3. All three DRV2605 motors run via PWM at 50% duty for 3 seconds,
 *      then stop.
 *
 * After that, each MODE_TOGGLE button press cycles the mode LED:
 *   Teal -> Blue -> Purple -> Teal -> ...
 *
 * Still intentionally bypasses haptic.c / charge_status.c / rgb_led.c
 * -- those stay untouched in the project for when the full
 * breathing-pattern/session logic comes back. gpio.c and button.c ARE
 * now in use (button.c plain-polls PC13, no interrupt).
 *
 * Note: PWM alone does not make the DRV2605 vibrate -- it powers up
 * in standby by default and ignores IN/TRIG until told over I2C to
 * enter PWM/analog-input mode. That's the one I2C write below.
 */

#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "i2c.h"
#include "drv2605.h"
#include "button.h"

static void SystemClock_Config(void);

/* Teal -> Blue -> Purple, cycled by button press. */
typedef struct { uint8_t r, g, b; } rgb_t;
static const rgb_t s_mode_colors[3] = {
    { 180, 0, 0 },   /* Teal   */
    { 255,   255, 0 },   /* Blue   */
    { 180,  255, 0 },  /* Purple */
};
#define NUM_MODE_COLORS  (sizeof(s_mode_colors) / sizeof(s_mode_colors[0]))
static uint8_t s_mode_color_idx = 0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    TIM1_RGB_Init();
    TIM3_Motor_Init();
    I2C1_Init();
    Button_Init();

    /* Charge LED: solid green, always. GPIO_Init() deliberately does
     * NOT configure these pins (that's normally RGB_ChargeLED_Init()'s
     * job in rgb_led.c, which this bring-up test bypasses) -- so we
     * have to put them in output mode ourselves before writing to
     * them, or the writes below are silently no-ops. */
    {
        GPIO_InitTypeDef gi = {0};
        gi.Mode = GPIO_MODE_OUTPUT_PP;
        gi.Pull = GPIO_NOPULL;
        gi.Speed = GPIO_SPEED_FREQ_LOW;

        gi.Pin = CHG_LED_R_Pin;
        HAL_GPIO_Init(CHG_LED_R_GPIO_Port, &gi);

        gi.Pin = CHG_LED_G_Pin;
        HAL_GPIO_Init(CHG_LED_G_GPIO_Port, &gi);

        gi.Pin = CHG_LED_B_Pin;
        HAL_GPIO_Init(CHG_LED_B_GPIO_Port, &gi);
    }
    HAL_GPIO_WritePin(CHG_LED_R_GPIO_Port, CHG_LED_R_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CHG_LED_G_GPIO_Port, CHG_LED_G_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CHG_LED_B_GPIO_Port, CHG_LED_B_Pin, GPIO_PIN_SET);

    /* Mode LED: start on the first color in the cycle (Teal). */
    s_mode_color_idx = 0;
    TIM_SetModeLED(s_mode_colors[0].r, s_mode_colors[0].g, s_mode_colors[0].b);

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

    while (1) {
        if (Button_ConsumePressEvent()) {
            s_mode_color_idx = (uint8_t)((s_mode_color_idx + 1) % NUM_MODE_COLORS);
            const rgb_t *c = &s_mode_colors[s_mode_color_idx];
            TIM_SetModeLED(c->r, c->g, c->b);
        }
        HAL_Delay(5);
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
