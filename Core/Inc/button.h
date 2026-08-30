#ifndef __BUTTON_H
#define __BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

/* MODE_TOGGLE button, sensed through the analog input-conditioning
 * front end (clamp + RC filter + unity buffer + 0.6x attenuator) and
 * landing on PC13. Idle sits around 0.6*VCC, pressed pulls to ~0V, so
 * we treat PC13 as an active-low digital input and debounce in
 * software (PC13 has no usable analog range here, and the front-end
 * RC filter already does most of the anti-bounce work for contact
 * noise; the software debounce below guards against the remaining
 * multi-triggering during a human press/release). */
void Button_Init(void);

void Button_EXTI_Callback(void);

/* Debounced level-change detector. Call from the main loop so a press
 * still registers if EXTI misses the analog-conditioned edge. */
void Button_Poll(void);

bool Button_ConsumePressEvent(void);

GPIO_PinState Button_ReadRaw(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H */
