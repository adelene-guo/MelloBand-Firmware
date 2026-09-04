#include "charge_status.h"
#include "rgb_led.h"

void ChargeStatus_Init(void)
{
    RGB_ChargeLED_Init();
    RGB_ChargeLED_Red(false);       /* red channel unused in this scheme */
    RGB_ChargeLED_GreenSolid();     /* default to "full" look until first read */
}

charge_status_t ChargeStatus_Update(void)
{
    /* STAT (open-drain, pulled up by R9): LOW = charging / constant
     * input voltage present, HIGH = fully charged. */
    GPIO_PinState stat = HAL_GPIO_ReadPin(CHG_STAT_GPIO_Port, CHG_STAT_Pin);

    if (stat == GPIO_PIN_RESET) {
        RGB_ChargeLED_GreenPulseUpdate();   /* call every tick for a smooth pulse */
        return CHG_STATE_CHARGING;
    } else {
        RGB_ChargeLED_GreenSolid();
        return CHG_STATE_FULL;
    }
}
