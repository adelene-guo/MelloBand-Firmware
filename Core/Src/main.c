/**
 * main.c -- BREATHING-PATTERN HAPTICS (RTP mode) + BUTTON + MODE LED
 *
 * Switches from the flat PWM buzz test to the actual breathing-
 * pattern engine in haptic.c: motor amplitude is now driven entirely
 * over I2C via the DRV2605 real-time-playback (RTP) register, not
 * PWM -- a different control path from the one already validated by
 * the standalone motor test. TIM3/PWM is intentionally NOT
 * initialized here; the DRV2605 IN/TRIG pins (PA6/PA7/PB0) stay
 * parked as analog (see gpio.c) since RTP mode doesn't use them.
 *
 * Each MODE_TOGGLE button press starts (or advances to) the next
 * breathing pattern:
 *   Box Breath (green)  -> Deep Calm (blue) -> Coherent Breath (purple) -> ...
 * Each session runs SESSION_CYCLES breathing cycles (see haptic.h),
 * then plays a two-click end-of-session confirmation and powers the
 * motors down automatically -- the next press starts a fresh session
 * on the next pattern.
 *
 * NOTE: this RTP path hasn't been physically confirmed yet the way
 * the PWM path was -- if motors don't respond as expected here, that
 * narrows the problem to RTP mode specifically (FEEDBACK_CONTROL /
 * MODE register setup in DRV2605_PowerUp(), or the RTP register
 * writes in DRV2605_SetRealtimeAmplitude()), not to I2C wiring in
 * general, since the same bus already works for PWM-mode
 * configuration writes.
 */

#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "i2c.h"
#include "drv2605.h"
#include "haptic.h"
#include "button.h"
#include "charge_status.h"

static void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    GPIO_Init();
    TIM1_RGB_Init();   /* mode LED only -- no TIM3/PWM in this build */
    I2C1_Init();
    Button_Init();
    ChargeStatus_Init();

    Haptic_Init();      /* motors stay powered down until the first button press */

    while (1) {
        if (Button_ConsumePressEvent()) {
            Haptic_NextPattern();   /* Box Breath -> Deep Calm -> Coherent Breath -> ... */
        }
        Haptic_Update();
        ChargeStatus_Update();
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
