#ifndef __CHARGE_STATUS_H
#define __CHARGE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Charge status enum used by ChargeStatus_Update/Init.
 * CHG_STATUS_NO_BATTERY : STAT reads high -> no battery present (show RED)
 * CHG_STATUS_BATTERY_PRESENT : STAT reads low -> battery present (show GREEN)
 * CHG_STATUS_UNKNOWN : initial/uninitialized state. */
typedef enum {
    CHG_STATUS_UNKNOWN = 0,
    CHG_STATUS_NO_BATTERY,
    CHG_STATUS_BATTERY_PRESENT,
} charge_status_t;

/* Initialize charge-status handling and set the LED to the current
 * STAT state. Call once at startup. */
void ChargeStatus_Init(void);

/* Read STAT and update the charge-status RGB LED. Returns the current
 * charge status. Call periodically (a few times per second is fine).
 */
charge_status_t ChargeStatus_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* __CHARGE_STATUS_H */
