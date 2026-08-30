#include "i2c.h"
#include "main.h"

I2C_HandleTypeDef hi2c1;

/* Initialize I2C1 peripheral and associated GPIO pins. */
void I2C1_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitTypeDef gi = {0};
    gi.Pin = MOTOR_I2C_SCL_Pin | MOTOR_I2C_SDA_Pin;
    gi.Mode = GPIO_MODE_AF_OD;      /* I2C requires open-drain */
    gi.Pull = GPIO_PULLUP;          /* internal pull as backup; add
                                        external 2.2-4.7k pull-ups on
                                        the bus if not already present */
    gi.Speed = GPIO_SPEED_FREQ_HIGH;
    gi.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gi);

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;             /* fast mode */
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}
