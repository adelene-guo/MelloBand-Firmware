#include "debug.h"
#include "main.h"

#define SWO_BAUD     2000000U
#define USART2_BAUD  115200U

static void swo_init(void)
{
    DBGMCU->CR |= DBGMCU_CR_TRACE_IOEN;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    TPI->ACPR = (SystemCoreClock / SWO_BAUD) - 1U;
    TPI->SPPR = 2U; /* asynchronous NRZ */
    TPI->FFCR = 0x100U;

    ITM->LAR = 0xC5ACCE55U;
    ITM->TCR = ITM_TCR_ITMENA_Msk | ITM_TCR_SYNCENA_Msk | ITM_TCR_SWOENA_Msk;
    ITM->TER = 0x1U; /* stimulus port 0 */
}

static void usart2_tx_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    /* PA2 = USART2_TX, AF7. Unused on this board; optional USB-UART. */
    GPIO_InitTypeDef gi = {0};
    gi.Pin = GPIO_PIN_2;
    gi.Mode = GPIO_MODE_AF_PP;
    gi.Pull = GPIO_PULLUP;
    gi.Speed = GPIO_SPEED_FREQ_HIGH;
    gi.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &gi);

    /* APB1 = 50 MHz. OVER8=0 -> BRR = fck / baud. */
    USART2->CR1 = 0;
    USART2->BRR = 50000000U / USART2_BAUD;
    USART2->CR2 = 0;
    USART2->CR3 = 0;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void DebugConsole_Init(void)
{
    swo_init();
    usart2_tx_init();
}

int __io_putchar(int ch)
{
    ITM_SendChar((uint32_t)ch);

    if (ch == '\n') {
        while ((USART2->SR & USART_SR_TXE) == 0U) { }
        USART2->DR = (uint16_t)'\r';
    }
    while ((USART2->SR & USART_SR_TXE) == 0U) { }
    USART2->DR = (uint16_t)(ch & 0xFF);
    return ch;
}
