#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/* SWO/ITM (CubeIDE SWV console) plus USART2 TX on PA2 at 115200 8N1
 * (USB-UART adapter on PA2 if you are not using SWV). */
void DebugConsole_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_H */
