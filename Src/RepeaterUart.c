#include "main.h"
extern repeaterSettings_t settings;
extern UART_HandleTypeDef huart1;
extern SX1276_t myRadio1;
extern SX1276_t myRadio2;
extern uint8_t uartIn;
extern char error[40];
extern flag_t flag;
extern uint32_t version;
uint8_t uartRx[32];
uint8_t uartIn;
uint8_t uartPos;
uint8_t len;


void readByte(void)
{
	if (uartIn=='<')
	uartPos=0;
	else if (uartIn=='>')
	{
		len=uartPos;
		flag.uartRx=1;
	}
	else
	uartRx[uartPos++]=uartIn;
	HAL_UART_Receive_IT(&huart1, &uartIn, 1);
}

void uartReceiveHandler()
{

	uint8_t l=len-2;
	uint8_t * ptr = uartRx+2;
	uint32_t tmp;
	char tempString[20];

	switch (uartRx[0])
	{
	case UART_FREQUENCY   :
		tmp = DecToInt(ptr, l);
		if(uartRx[0]==1)
		settings.realFrequency1 = tmp;
		if(uartRx[0]==2)
		settings.realFrequency2 = tmp;
		break;

	case UART_SF          :
		tmp=DecToInt(ptr, l);
		if(uartRx[0]==1)
		settings.sf1 = tmp;
		if(uartRx[0]==2)
		settings.sf2 = tmp;
		break;

	case UART_BW          :
		tmp=DecToInt(ptr, l);
		if(uartRx[0]==1)
		settings.bw1=tmp;
		if(uartRx[0]==2)
		settings.bw2=tmp;
		break;

	case UART_SYNCWORD    :

		tmp=HexToInt(ptr, l);
		if(uartRx[0]==1)
		settings.sw1 = tmp;
		if(uartRx[0]==2)
		settings.sw2 = tmp;
		break;

	case UART_PREAMBLE:
		tmp = DecToInt(ptr, l);
		if(uartRx[0]==1)
		settings.preamble1 = tmp;
		if(uartRx[0]==2)
		settings.preamble2 = tmp;
		break;

	case UART_CR:
		tmp = DecToInt(ptr, l);
		if(uartRx[0]==1)
		settings.cr1=tmp;
		if(uartRx[0]==2)
		settings.cr2=tmp;
		break;

	case UART_POWER:
		tmp = DecToInt(ptr, l);
		if(uartRx[0]==1)
		settings.power1=tmp;
		if(uartRx[0]==2)
		settings.power2=tmp;
		break;

	case UART_USELED:
		if(uartRx[0]==1)
		settings.useLed1=DecToInt(ptr, l);
		if(uartRx[0]==2)
		settings.useLed2=DecToInt(ptr, l);
		break;

	case UART_SAVE:
		flag.saveSettings=1;
		break;


	case UART_CALL:
		sprintf(tempString,"<ANv%lu>",version);
		HAL_UART_Transmit(&huart1, (uint8_t*)tempString, strlen(tempString), 100);
		break;

	default:
		HAL_UART_Transmit(&huart1, (uint8_t*)"Error", 5, 100);
		Error_Handler();
		break;

	}

}

void sendConfig(void)
{
	char str[32];
	sprintf(str,"<11%ld>",settings.realFrequency1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<12%u>",settings.sf1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<13%u>",settings.bw1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<14%X>",settings.sw1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<15%u>",settings.power1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<18%u>",settings.preamble1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<19%u>",settings.cr1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<1L%u>",settings.useLed1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<11%ld>",settings.realFrequency1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<22%u>",settings.sf1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<23%u>",settings.bw1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<24%X>",settings.sw1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<25%u>",settings.power1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<28%u>",settings.preamble1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<29%u>",settings.cr1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<2L%u>",settings.useLed1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
}



