#include "button.h"

static bool s_last_pressed = false;

void Button_Init(void)
{
    s_last_pressed = (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_SET);
}

bool Button_ConsumePressEvent(void)
{
    bool pressed = (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_SET);
    bool event = pressed && !s_last_pressed;   /* low->high transition */
    s_last_pressed = pressed;
    return event;
}
