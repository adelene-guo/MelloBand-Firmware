#ifndef __DRV2605_H
#define __DRV2605_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* DRV2605L fixed 7-bit I2C address (no ADDR pins on this part). All
 * three drivers on this board share this address on one bus -- see
 * i2c.h. That turns out to be harmless for this application: every
 * pattern drives all active motors with the *same* amplitude curve at
 * the same time, so a single broadcast I2C write already does the
 * right thing for all three chips at once. */
#define DRV2605_I2C_ADDR7           0x5A
#define DRV2605_I2C_ADDR_W8         (DRV2605_I2C_ADDR7 << 1)

/* Register map (subset used here) */
#define DRV2605_REG_STATUS          0x00
#define DRV2605_REG_MODE            0x01
#define DRV2605_REG_RTP             0x02
#define DRV2605_REG_LIBRARY         0x03
#define DRV2605_REG_WAVESEQ1        0x04
#define DRV2605_REG_WAVESEQ2        0x05
#define DRV2605_REG_WAVESEQ3        0x06
#define DRV2605_REG_WAVESEQ4        0x07
#define DRV2605_REG_GO              0x0C
#define DRV2605_REG_FEEDBACK        0x1A
#define DRV2605_REG_CONTROL3        0x1D

#define DRV2605_MODE_INTERNAL_TRIG  0x00   /* MODE[2:0]=0: waveform-library playback via GO bit */
#define DRV2605_MODE_PWM_ANALOG     0x03   /* MODE[2:0]=3: amplitude driven by PWM on IN/TRIG pin -- used by the current minimal bring-up test */
#define DRV2605_MODE_RTP            0x05   /* MODE[2:0]=5: amplitude driven live by RTP register */

#define DRV2605_LIBRARY_ROM_A       0x01

/* ROM library "A" (ERM) effect IDs used here */
#define DRV2605_EFFECT_STRONG_CLICK_100   1

/* Bring all three drivers out of hardware shutdown (shared EN line),
 * broadcast-configure ERM feedback, and leave them parked in
 * Real-Time-Playback mode with amplitude 0 (silent) ready for
 * Haptic_Update() to drive via DRV2605_SetRealtimeAmplitude(). */
HAL_StatusTypeDef DRV2605_PowerUp(void);

/* Bring all three drivers out of hardware shutdown (shared EN line),
 * broadcast-configure ERM feedback, and select PWM/analog-input mode
 * with PWM selected -- IN/TRIG (PA6/PA7/PB0, see tim.c TIM3_Motor_Init)
 * now directly sets each motor's amplitude. Use this instead of
 * DRV2605_PowerUp() when driving motors via TIM3 PWM rather than the
 * RTP register. */
HAL_StatusTypeDef DRV2605_EnterPWMMode(void);

/* Drops the shared EN line low, hardware-shutting-down all three
 * drivers. Call at the end of a session. */
void DRV2605_PowerDown(void);

/* RTP mode: sets the live amplitude (0-255, 0=silent, 255=full
 * strength). This is the register that "ramp gain register in 200ms
 * steps" and "continuous waveform playback at fixed gain" refer to --
 * it's a direct amplitude knob, updated as fast or as slow as the
 * caller likes. */
HAL_StatusTypeDef DRV2605_SetRealtimeAmplitude(uint8_t amplitude);

/* Plays a two-click "double-tap" confirmation using the DRV2605's
 * built-in ROM waveform library (two Strong-Click effects separated
 * by a short scripted wait, triggered by a single GO). Blocks
 * (briefly, with a timeout) until playback finishes. Leaves the part
 * in internal-trigger mode; call DRV2605_PowerDown() right after. */
HAL_StatusTypeDef DRV2605_PlayDoubleTap(void);

/* Low level helpers */
HAL_StatusTypeDef DRV2605_WriteReg(uint8_t reg, uint8_t value);
HAL_StatusTypeDef DRV2605_ReadReg(uint8_t reg, uint8_t *value);

#ifdef __cplusplus
}
#endif

#endif /* __DRV2605_H */
