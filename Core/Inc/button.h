#ifndef __BUTTON_H
#define __BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

/* MODE_TOGGLE button, plain polling -- no EXTI, no interrupt, no
 * debounce timer. Treats a HIGH read on BUTTON_Pin (PC13) as
 * "pressed". Call Button_Init() once at boot, then call
 * Button_ConsumePressEvent() every main-loop iteration. */

void Button_Init(void);

/* Reads the pin right now and compares it to the last poll. Returns
 * true exactly once per low->high transition (i.e. once per press,
 * as long as you call this often enough that you don't miss a
 * transition -- there's no debounce, so a mechanically noisy switch
 * may register more than one event per physical press). */
bool Button_ConsumePressEvent(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H */
