#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

static char printf_buffer[1023];
void UART_Init(void);
char UART_InChar(void);
void UART_OutChar(char data);
void UART_OutWord(uint16_t data);
void UART_printf(const char* array);
void Status_Check(char* array, int status);

#endif
