#include "haptic.h"
#include "tim.h"
#include "drv2605.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Box Breath: 4s in / 4s hold / 4s out / 4s hold, green
 * Deep Calm:  4s in / 1s hold / 6s out / 1s hold, blue
 * Coherent:   5s in / 0 hold  / 5s out / 0 hold,  purple
 *
 * Per-phase motor behavior:
 *   inhale : continuous rise, 20% -> 85%, across the whole inhale duration
 *   exhale : continuous fade, 85% -> 0%, across the whole exhale duration
 *   hold   : one short buzz per second for the whole hold duration
 *            (a 4s hold = 4 distinct buzzes, a 1s hold = 1 buzz)
 */
static const haptic_pattern_t s_patterns[HAPTIC_NUM_PATTERNS] = {
    /* name               R    G    B   inhale hold1 exhale hold2 */
    { "Box Breath",        255, 0,   255,  4000, 4000,  4000, 4000 },
    { "Deep Calm",         255,   255, 0,  4000, 1000,  6000, 1000 },
    { "Coherent Breath", 95,   255, 0,  5000,    0,  5000,    0 },
};

/* Inhale/exhale ramp update interval. Small enough that the amplitude
 * change is imperceptibly smooth (a 4s inhale becomes ~160 steps
 * instead of 20), not a fixed "waveform" -- these are ordinary RTP
 * writes, just frequent ones. */
#define RAMP_STEP_MS       25U

/* Hold-phase buzz shape: a short pulse at the start of each 1-second
 * slot, silent for the rest of that second. */
#define HOLD_BUZZ_ON_MS    150U
#define HOLD_BUZZ_AMP_PCT  70U

typedef enum {
    PHASE_IDLE = 0,
    PHASE_INHALE,
    PHASE_HOLD1,
    PHASE_EXHALE,
    PHASE_HOLD2,
    PHASE_CYCLE_DONE,   /* internal marker only, never held as current phase */
} phase_t;

static uint8_t  s_pattern_idx   = 0;
static phase_t  s_phase         = PHASE_IDLE;
static uint32_t s_phase_start   = 0;
static uint8_t  s_cycle_count   = 0; /* counts until SESSION_CYCLES, then a double buzz */
static bool     s_session_active = false;

static uint16_t s_last_step_index = 0xFFFF;  /* sentinel: forces a write on phase entry (inhale/exhale) */
static bool     s_last_pulse_on   = false;   /* tracks buzz on/off state (hold) */

/* Convert 0..100 percentage to 0..255 amplitude for the DRV2605 RTP register. */
static inline uint8_t pct_to_amp(uint8_t pct)
{
    if (pct > 100) pct = 100;
    return (uint8_t)(((uint16_t)pct * 255U) / 100U);
}

/* End the current haptic session: silence motors, play confirmation,
 * power the drivers down, and clear session state. */
static void end_session(void)
{
    DRV2605_SetRealtimeAmplitude(0);
    DRV2605_PlayDoubleTap();      /* end-of-session double-tap confirmation */
    DRV2605_PowerDown();

    TIM_SetModeLED(0, 0, 0);
    s_session_active = false;
    s_phase = PHASE_IDLE;
}

/* Enter a new phase and reset per-phase transient state. */
static void enter_phase(phase_t phase)
{
    s_phase = phase;
    s_phase_start = HAL_GetTick();
    s_last_step_index = 0xFFFF;
    s_last_pulse_on = false;
}

/* Called when `completed` phase's duration has fully elapsed; figures
 * out (and enters) the next phase, skipping any zero-length hold, and
 * rolls over into a new breathing cycle or ends the session once
 * SESSION_CYCLES have completed. */
static void advance_from(phase_t completed)
{
    const haptic_pattern_t *p = &s_patterns[s_pattern_idx];
    phase_t next;

    switch (completed) {
        case PHASE_INHALE:
            next = (p->hold1_ms > 0) ? PHASE_HOLD1 : PHASE_EXHALE;
            break;
        case PHASE_HOLD1:
            next = PHASE_EXHALE;
            break;
        case PHASE_EXHALE:
            next = (p->hold2_ms > 0) ? PHASE_HOLD2 : PHASE_CYCLE_DONE;
            break;
        case PHASE_HOLD2:
        default:
            next = PHASE_CYCLE_DONE;
            break;
    }

    if (next == PHASE_CYCLE_DONE) {
        s_cycle_count++;
        if (s_cycle_count >= SESSION_CYCLES) {
            end_session();
            return;
        }
        next = PHASE_INHALE;
    }

    enter_phase(next);
}

/* Initialize the haptic subsystem; choose starting pattern so the first
 * user press selects pattern 0. */
void Haptic_Init(void)
{
    /* Set up so the first button press lands on pattern 0 (Box Breath). */
    s_pattern_idx = HAPTIC_NUM_PATTERNS - 1;
    s_phase = PHASE_IDLE;
    s_session_active = false;
    s_cycle_count = 0;
    TIM_SetModeLED(0, 0, 0);
}

/* Start (or restart) a breathing session on the next pattern. Powers
 * the DRV2605s if needed and initializes the breathing state machine. */
void Haptic_NextPattern(void)
{
    s_pattern_idx = (uint8_t)((s_pattern_idx + 1) % HAPTIC_NUM_PATTERNS);
    const haptic_pattern_t *p = &s_patterns[s_pattern_idx];

    if (!s_session_active) {
        DRV2605_PowerUp();
    } else {
        DRV2605_SetRealtimeAmplitude(0);   /* silence whatever was mid-phase */
    }

    s_cycle_count = 0;
    s_session_active = true;
    TIM_SetModeLED(p->led_r, p->led_g, p->led_b);
    enter_phase(PHASE_INHALE);
}

/* Run the breathing session state machine; update motor amplitude and
 * LED mode. Call frequently from the main loop (every few milliseconds).
 */
void Haptic_Update(void)
{
    if (!s_session_active || s_phase == PHASE_IDLE) {
        return;
    }

    const haptic_pattern_t *p = &s_patterns[s_pattern_idx];
    uint32_t elapsed = HAL_GetTick() - s_phase_start;

    switch (s_phase) {

    case PHASE_INHALE: {
        /* Smooth eased rise 20% -> 85% across the whole inhale
         * duration, updated every RAMP_STEP_MS for a continuous feel
         * (not a stepped/staircase ramp). */
        uint16_t dur = p->inhale_ms;
        uint16_t total_steps = (dur / RAMP_STEP_MS);
        if (total_steps == 0) total_steps = 1;
        uint16_t step = (uint16_t)(elapsed / RAMP_STEP_MS);
        if (step > total_steps) step = total_steps;

        if (step != s_last_step_index) {
            s_last_step_index = step;
            float frac = (float)step / (float)total_steps;               /* 0..1 */
            float eased = 0.5f * (1.0f - cosf((float)M_PI * frac));       /* slow-start/slow-end 0..1 */
            uint8_t pct = (uint8_t)(20.0f + eased * (85.0f - 20.0f) + 0.5f);
            DRV2605_SetRealtimeAmplitude(pct_to_amp(pct));
        }

        if (elapsed >= dur + 1000) {
        	advance_from(PHASE_INHALE);
        } else if (elapsed >= dur) {
        	DRV2605_SetRealtimeAmplitude(0);
        }
        break;
    }

    case PHASE_EXHALE: {
        /* Continuous fade 85% -> 0%, updated in 200ms steps across
         * the whole exhale duration. */
        uint16_t dur = p->exhale_ms;
        uint16_t total_steps = (dur / 200U);
        if (total_steps == 0) total_steps = 1;
        uint16_t step = (uint16_t)(elapsed / 200U);
        if (step > total_steps) step = total_steps;

        if (step != s_last_step_index) {
            s_last_step_index = step;
            uint8_t pct = (uint8_t)(85U - ((uint32_t)85U * step) / total_steps);
            DRV2605_SetRealtimeAmplitude(pct_to_amp(pct));
        }

        if (elapsed >= dur) {
            DRV2605_SetRealtimeAmplitude(0);
            advance_from(PHASE_EXHALE);
        }
        break;
    }

    case PHASE_HOLD1:
    case PHASE_HOLD2: {
        /* One short buzz per second for the whole hold duration:
         * on for HOLD_BUZZ_ON_MS at the start of each 1-second slot,
         * silent for the rest of that second. */
        uint16_t dur = (s_phase == PHASE_HOLD1) ? p->hold1_ms : p->hold2_ms;
        uint32_t ms_into_second = elapsed % 1000U;
        bool buzz_on = (ms_into_second < HOLD_BUZZ_ON_MS);

        if (buzz_on != s_last_pulse_on) {
            s_last_pulse_on = buzz_on;
            DRV2605_SetRealtimeAmplitude(buzz_on ? pct_to_amp(HOLD_BUZZ_AMP_PCT) : 0);
        }

        if (elapsed >= dur) {
            DRV2605_SetRealtimeAmplitude(0);
            advance_from(s_phase);
        }
        break;
    }

    default:
        break;
    }
}

uint8_t Haptic_CurrentPatternIndex(void)
{
    return s_pattern_idx;
}

bool Haptic_SessionActive(void)
{
    return s_session_active;
}
