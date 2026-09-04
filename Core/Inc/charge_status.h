#ifndef __CHARGE_STATUS_H
#define __CHARGE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Reads CHARGE_STATUS_SIG (PB12), the buffered/pulled-up STAT output
 * of the MCP73831 charge-management IC, and drives the charge-status
 * RGB LED (D3) accordingly:
 *   - STAT low  -> charging (or constant input voltage present) -> green, pulsing
 *   - STAT high -> battery fully charged                        -> green, solid
 */
typedef enum {
    CHG_STATE_CHARGING = 0,   /* STAT low  */
    CHG_STATE_FULL,           /* STAT high */
} charge_status_t;

void ChargeStatus_Init(void);

/* Call every main-loop tick (a few ms) -- not just periodically --
 * so the pulsing-green animation stays smooth while charging. */
charge_status_t ChargeStatus_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* __CHARGE_STATUS_H */
