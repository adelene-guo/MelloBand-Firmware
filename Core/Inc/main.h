/**
 * main.h
 *
 * Board: STM32F413RGT6 haptic "breathing" wearable/handheld.
 *
 * Pin mapping below was reconstructed from the schematic NETLIST
 * (PIU10xx pin references), which is authoritative over the row
 * labels printed on the sheet -- a couple of those (BOOT1/TDO,
 * MCU_STATUS_SIG) are ambiguous in the OCR'd text and were
 * cross-checked against the net list and STM32F413 default AF map
 * (e.g. PB3=JTDO, PB8/PB9=I2C1 SCL/SDA, PA6/PA7/PB0=TIM3_CH1/2/3,
 * PA8/PA9/PA10=TIM1_CH1/2/3) for consistency.
 */
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* ---------------------------------------------------------------------
 * Pin map (from MCU-io.SchDoc / Interface.SchDoc / Power.SchDoc)
 * ------------------------------------------------------------------- */

/* Haptic motor driver enable (shared EN line for U5/U6/U7, all DRV2605L) */
#define MOTOR_EN_GPIO_Port         GPIOA
#define MOTOR_EN_Pin                GPIO_PIN_0

/* Charge-status RGB LED (D3), driven straight by MCU GPIO through 100R */
#define CHG_LED_R_GPIO_Port         GPIOA
#define CHG_LED_R_Pin               GPIO_PIN_5
#define CHG_LED_G_GPIO_Port         GPIOB
#define CHG_LED_G_Pin               GPIO_PIN_14
#define CHG_LED_B_GPIO_Port         GPIOB
#define CHG_LED_B_Pin               GPIO_PIN_15

/* Motor PWM (-> DRV2605 IN/TRIG pins, PWM/analog-input mode), TIM3 CH1-3 */
#define MOTOR_PWM_A_GPIO_Port       GPIOA
#define MOTOR_PWM_A_Pin             GPIO_PIN_6   /* TIM3_CH1, AF2 */
#define MOTOR_PWM_B_GPIO_Port       GPIOA
#define MOTOR_PWM_B_Pin             GPIO_PIN_7   /* TIM3_CH2, AF2 */
#define MOTOR_PWM_C_GPIO_Port       GPIOB
#define MOTOR_PWM_C_Pin             GPIO_PIN_0   /* TIM3_CH3, AF2 */

/* Mode-indicator RGB LED (D2), TIM1 CH1-3 for smooth color mixing */
#define MODE_LED_R_GPIO_Port        GPIOA
#define MODE_LED_R_Pin              GPIO_PIN_8   /* TIM1_CH1, AF1 */
#define MODE_LED_G_GPIO_Port        GPIOA
#define MODE_LED_G_Pin              GPIO_PIN_9   /* TIM1_CH2, AF1 */
#define MODE_LED_B_GPIO_Port        GPIOA
#define MODE_LED_B_Pin              GPIO_PIN_10  /* TIM1_CH3, AF1 */

/* Heartbeat / status LED (D5), driven from MCU_STATUS_SIG */
#define STATUS_LED_GPIO_Port        GPIOB
#define STATUS_LED_Pin              GPIO_PIN_4

/* I2C1 bus shared by all three DRV2605L drivers (same fixed 0x5A addr) */
#define MOTOR_I2C_SCL_GPIO_Port     GPIOB
#define MOTOR_I2C_SCL_Pin           GPIO_PIN_8   /* I2C1_SCL, AF4 */
#define MOTOR_I2C_SDA_GPIO_Port     GPIOB
#define MOTOR_I2C_SDA_Pin           GPIO_PIN_9   /* I2C1_SDA, AF4 */

/* Charge-status input from MCP73831 STAT pin (open-drain, R9 pull-up) */
#define CHG_STAT_GPIO_Port          GPIOB
#define CHG_STAT_Pin                GPIO_PIN_12

/* Mode-toggle button, conditioned/attenuated by the analog front end,
 * lands on PC13 (EXTI13). Idle ~0.6*VCC, pressed ~0V -> treat as
 * active-low digital input. */
#define BUTTON_GPIO_Port            GPIOC
#define BUTTON_Pin                  GPIO_PIN_13
#define BUTTON_EXTI_IRQn            EXTI15_10_IRQn

/* ---------------------------------------------------------------------
 * Misc
 * ------------------------------------------------------------------- */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
