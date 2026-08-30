#include "charge_status.h"
#include "rgb_led.h"
#include "main.h"

/* Tracks last-observed charge status to avoid redundant LED operations. */
static charge_status_t s_last_state = CHG_STATUS_UNKNOWN;

/*
 * Initialize charge-status handling and set the LED to the current STAT
 * reading (red if no battery, green if battery present).
 */
void ChargeStatus_Init(void)
{
    GPIO_PinState stat = HAL_GPIO_ReadPin(CHG_STAT_GPIO_Port, CHG_STAT_Pin);

    s_last_state = (stat == GPIO_PIN_SET) ? CHG_STATUS_NO_BATTERY
                                          : CHG_STATUS_BATTERY_PRESENT;

    if (s_last_state == CHG_STATUS_NO_BATTERY) {
        RGB_ChargeLED_Red(true);
        RGB_ChargeLED_GreenOff();
    } else {
        RGB_ChargeLED_Red(false);
        RGB_ChargeLED_GreenSolid();
    }
}

/*
 * Read the MCP73831 STAT pin and update the charge-status LED.
 * Returns the current charge_status_t.
 *
 * STAT mapping used here (per request):
 *   - STAT high (GPIO_PIN_SET): no battery present -> RED
 *   - STAT low  (GPIO_PIN_RESET): battery present / CV -> GREEN
 */
charge_status_t ChargeStatus_Update(void)
{
    GPIO_PinState stat = HAL_GPIO_ReadPin(CHG_STAT_GPIO_Port, CHG_STAT_Pin);

    charge_status_t state = (stat == GPIO_PIN_SET) ? CHG_STATUS_NO_BATTERY
                                                  : CHG_STATUS_BATTERY_PRESENT;

    if (state != s_last_state) {
        s_last_state = state;
        if (state == CHG_STATUS_NO_BATTERY) {
            RGB_ChargeLED_Red(true);
            RGB_ChargeLED_GreenOff();
        } else {
            RGB_ChargeLED_Red(false);
            RGB_ChargeLED_GreenSolid();
        }
    }

    return s_last_state;
}
