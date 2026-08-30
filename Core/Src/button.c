#include "button.h"
#include "main.h"
#define DEBOUNCE_MS   180U

static volatile uint32_t s_last_edge_tick = 0;
static volatile bool s_press_pending = false;

static GPIO_PinState s_stable_level;
static GPIO_PinState s_candidate_level;
static uint32_t s_candidate_since;

void Button_Init(void)
{
    s_last_edge_tick = 0;
    s_press_pending = false;
    s_stable_level = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);
    s_candidate_level = s_stable_level;
    s_candidate_since = HAL_GetTick();
}

void Button_EXTI_Callback(void)
{
    uint32_t now = HAL_GetTick();
    if ((now - s_last_edge_tick) >= DEBOUNCE_MS) {
        s_press_pending = true;
    }
    s_last_edge_tick = now;
}

void Button_Poll(void)
{
    GPIO_PinState raw = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);
    uint32_t now = HAL_GetTick();

    if (raw != s_candidate_level) {
        s_candidate_level = raw;
        s_candidate_since = now;
        return;
    }

    if ((now - s_candidate_since) < DEBOUNCE_MS) {
        return;
    }

    if (raw != s_stable_level) {
        s_stable_level = raw;
        s_press_pending = true;
    }
}

/* Atomically consume a pending button press event; returns true if a
 * press was available and clears the pending flag. */
bool Button_ConsumePressEvent(void)
{
    if (s_press_pending) {
        s_press_pending = false;
        return true;
    }
    return false;
}

GPIO_PinState Button_ReadRaw(void)
{
    return HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);
}
