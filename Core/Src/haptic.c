#include "haptic.h"
#include "tim.h"
#include "drv2605.h"

/* Box Breath: 4s in / 4s hold / 4s out / 4s hold, green
 * Deep Calm:  4s in / 1s hold / 6s out / 1s hold, blue
 * Coherent:   5s in / 0 hold  / 5s out / 0 hold,  purple
 */
static const haptic_pattern_t s_patterns[HAPTIC_NUM_PATTERNS] = {
    /* name               R    G    B   inhale hold1 exhale hold2 */
    { "Box Breath",        0, 255,   0,  4000, 4000,  4000, 4000 },
    { "Deep Calm",         0,   0, 255,  4000, 1000,  6000, 1000 },
    { "Coherent Breath", 160,   0, 255,  5000,    0,  5000,    0 },
};

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
static uint8_t  s_cycle_count   = 0; /* counts until 5 sessions, then a double buzz*/
static bool     s_session_active = false;

static uint16_t s_last_step_index = 0xFFFF;  /* sentinel: forces a write on phase entry */
static bool     s_last_pulse_on   = false;

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
        /* Smooth rise 20% -> 85%, updated in 200ms steps. */
        uint16_t dur = p->inhale_ms;
        uint16_t total_steps = (dur / 200U);
        if (total_steps == 0) total_steps = 1;
        uint16_t step = (uint16_t)(elapsed / 200U);
        if (step > total_steps) step = total_steps;

        if (step != s_last_step_index) {
            s_last_step_index = step;
            uint8_t pct = (uint8_t)(20U + ((uint32_t)(85U - 20U) * step) / total_steps);
            DRV2605_SetRealtimeAmplitude(pct_to_amp(pct));
        }

        if (elapsed >= dur) {
            advance_from(PHASE_INHALE);
        }
        break;
    }

    case PHASE_HOLD1:
    case PHASE_HOLD2: {
        /* Smooth fade 70% -> 10%, updated in 200ms steps, explicit
         * off at the end of the hold. */
        uint16_t dur = (s_phase == PHASE_HOLD1) ? p->hold1_ms : p->hold2_ms;
        uint16_t total_steps = (dur / 200U);
        if (total_steps == 0) total_steps = 1;
        uint16_t step = (uint16_t)(elapsed / 200U);
        if (step > total_steps) step = total_steps;

        if (step != s_last_step_index) {
            s_last_step_index = step;
            uint8_t pct = (uint8_t)(70U - ((uint32_t)(70U - 10U) * step) / total_steps);
            DRV2605_SetRealtimeAmplitude(pct_to_amp(pct));
        }

        if (elapsed >= dur) {
            DRV2605_SetRealtimeAmplitude(0);   /* ...-> off */
            advance_from(s_phase);
        }
        break;
    }

    case PHASE_EXHALE: {
        /* Steady ~30% amplitude, pulsed at 1 Hz (500ms on / 500ms
         * off), held constant (not ramped) for the whole exhale. */
        uint16_t dur = p->exhale_ms;
        bool on = (elapsed % 1000U) < 500U;

        if (on != s_last_pulse_on) {
            s_last_pulse_on = on;
            DRV2605_SetRealtimeAmplitude(on ? pct_to_amp(30) : 0);
        }

        if (elapsed >= dur) {
            DRV2605_SetRealtimeAmplitude(0);
            advance_from(PHASE_EXHALE);
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
