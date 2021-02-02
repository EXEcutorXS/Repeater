#ifndef UART_PROT
#define UART_PROT

#include "main.h"

//NODE COMMANDS
#define UART_FREQUENCY '1'
#define UART_SF '2'
#define UART_BW '3'
#define UART_SYNCWORD '4'
#define UART_POWER '5'
#define UART_PREAMBLE '8'
#define UART_CR '9'
#define UART_POWERALARM 'P'
#define UART_NODENUM 'n'
#define UART_CALL 'C'
#define UART_ACKNOWLEDGE 'A'
#define UART_READ 'R'
#define UART_SAVE 'S'
#define UART_WORKING_INTERVAL 'i'
#define UART_WARNING_DELAY 'q'
#define UART_SUPER_WARNING_DELAY 'w'
#define UART_WORKING_INTERVAL 'i'
#define UART_USELED	'L'

void readByte(void);
void uartInit();
void uartReceiveHandler();
void sendConfig(void);
void configNode(uint8_t nodeNum);
#endif
