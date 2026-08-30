#include "drv2605.h"
#include "i2c.h"
#include "main.h"

#define DRV2605_I2C_TIMEOUT_MS   50

/* Write a single register to the DRV2605 over I2C. */
HAL_StatusTypeDef DRV2605_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { reg, value };
    return HAL_I2C_Master_Transmit(&hi2c1, DRV2605_I2C_ADDR_W8, buf, 2,
                                    DRV2605_I2C_TIMEOUT_MS);
}

/* Read a single register from the DRV2605 over I2C. */
HAL_StatusTypeDef DRV2605_ReadReg(uint8_t reg, uint8_t *value)
{
    /* All three DRV2605Ls share one address/bus and (after
     * DRV2605_PowerUp) identical register state, so they answer
     * identically -- this is safe to use as an "is the bus alive"
     * check or to poll GO/STATUS. */
    HAL_StatusTypeDef st = HAL_I2C_Master_Transmit(&hi2c1, DRV2605_I2C_ADDR_W8,
                                                     &reg, 1, DRV2605_I2C_TIMEOUT_MS);
    if (st != HAL_OK) return st;
    return HAL_I2C_Master_Receive(&hi2c1, DRV2605_I2C_ADDR_W8, value, 1,
                                   DRV2605_I2C_TIMEOUT_MS);
}

/* Power up and configure the DRV2605 devices for real-time playback.
 * Returns HAL_OK on success or an I2C error status. */
HAL_StatusTypeDef DRV2605_PowerUp(void)
{
    HAL_StatusTypeDef st;

    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(2);   /* >250us required before first I2C txn after EN */

    /* FEEDBACK_CONTROL: N_ERM_LRA=0 -> ERM mode. */
    st = DRV2605_WriteReg(DRV2605_REG_FEEDBACK, 0x36);
    if (st != HAL_OK) return st;

    /* Park in Real-Time-Playback mode, silent, ready for
     * DRV2605_SetRealtimeAmplitude() from the breathing engine. */
    st = DRV2605_WriteReg(DRV2605_REG_RTP, 0);
    if (st != HAL_OK) return st;

    st = DRV2605_WriteReg(DRV2605_REG_MODE, DRV2605_MODE_RTP);
    if (st != HAL_OK) return st;

    return HAL_OK;
}

HAL_StatusTypeDef DRV2605_EnterPWMMode(void)
{
    HAL_StatusTypeDef st;

    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(2);

    st = DRV2605_WriteReg(DRV2605_REG_FEEDBACK, 0x36);
    if (st != HAL_OK) return st;

    st = DRV2605_WriteReg(DRV2605_REG_CONTROL3, DRV2605_CONTROL3_N_PWM_ANALOG);
    if (st != HAL_OK) return st;

    return DRV2605_WriteReg(DRV2605_REG_MODE, DRV2605_MODE_PWM_ANALOG);
}

/* Hardware power-down: drive shared enable low. */
void DRV2605_PowerDown(void)
{
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
}

/* Set the DRV2605 real-time-playback amplitude (0..255). */
HAL_StatusTypeDef DRV2605_SetRealtimeAmplitude(uint8_t amplitude)
{
    return DRV2605_WriteReg(DRV2605_REG_RTP, amplitude);
}

/* Play a two-click confirmation using internal waveform library;
 * block until playback completes (with timeout). */
HAL_StatusTypeDef DRV2605_PlayDoubleTap(void)
{
    HAL_StatusTypeDef st;

    st = DRV2605_WriteReg(DRV2605_REG_LIBRARY, DRV2605_LIBRARY_ROM_A);
    if (st != HAL_OK) return st;

    st = DRV2605_WriteReg(DRV2605_REG_MODE, DRV2605_MODE_INTERNAL_TRIG);
    if (st != HAL_OK) return st;

    /* Two-slot sequence: click, ~50ms scripted wait, click, stop.
     * WAVESEQ wait entries use bit7 set + lower 7 bits = wait time in
     * 10ms units (0x85 = 5 * 10ms = 50ms). */
    st = DRV2605_WriteReg(DRV2605_REG_WAVESEQ1, DRV2605_EFFECT_STRONG_CLICK_100);
    if (st != HAL_OK) return st;
    st = DRV2605_WriteReg(DRV2605_REG_WAVESEQ2, 0x85);
    if (st != HAL_OK) return st;
    st = DRV2605_WriteReg(DRV2605_REG_WAVESEQ3, DRV2605_EFFECT_STRONG_CLICK_100);
    if (st != HAL_OK) return st;
    st = DRV2605_WriteReg(DRV2605_REG_WAVESEQ4, 0); /* terminate */
    if (st != HAL_OK) return st;

    st = DRV2605_WriteReg(DRV2605_REG_GO, 0x01);
    if (st != HAL_OK) return st;

    /* GO auto-clears when the sequence finishes; poll with a
     * generous timeout (click + 50ms wait + click is well under
     * 300ms in practice). */
    uint8_t go = 1;
    for (int i = 0; i < 50 && go != 0; i++) {
        HAL_Delay(10);
        if (DRV2605_ReadReg(DRV2605_REG_GO, &go) != HAL_OK) break;
    }

    return HAL_OK;
}
