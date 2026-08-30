#ifndef __HAPTIC_H
#define __HAPTIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Breathing patterns. Durations are per the specified breath timing;
 * a hold duration of 0 means that phase is skipped entirely (used by
 * Coherent Breath, which has no holds). Amplitude behavior per phase
 * is fixed by spec (see haptic.c) and NOT per-pattern:
 *   inhale : ramp 20% -> 85%, 200ms steps, over the inhale duration
 *   hold   : ramp 70% -> 10% -> off, 200ms steps, over the hold duration
 *   exhale : steady ~30% amplitude pulsed at 1 Hz (500ms on/500ms off)
 * for the whole exhale duration
 */
typedef struct {
    const char *name;
    uint8_t  led_r, led_g, led_b;   /* mode indicator LED color, 0-255 */
    uint16_t inhale_ms;
    uint16_t hold1_ms;              /* after inhale */
    uint16_t exhale_ms;
    uint16_t hold2_ms;              /* after exhale */
} haptic_pattern_t;

#define HAPTIC_NUM_PATTERNS   3

/* Number of full breathe-cycles in one session before the end-of-
 * session double-tap + power-down. Not specified in the brief --
 * chosen as a reasonable default; change freely. */
#define SESSION_CYCLES        5

void Haptic_Init(void);

/* Starts (or restarts, if one is already running) a session on the
 * next pattern in the list. Powers the DRV2605s up if they weren't
 * already running. */
void Haptic_NextPattern(void);

/* Call frequently from the main loop (every ~5ms is plenty). Advances
 * the breathing phase state machine, updates the DRV2605 real-time
 * amplitude over I2C, and -- on the last cycle of a session -- plays
 * the end-of-session double-tap and powers the motors down. */
void Haptic_Update(void);

uint8_t Haptic_CurrentPatternIndex(void);
bool    Haptic_SessionActive(void);

#ifdef __cplusplus
}
#endif

#endif /* __HAPTIC_H */
