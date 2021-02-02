#include "main.h"
extern repeaterSettings_t settings;
extern UART_HandleTypeDef huart1;
extern uint8_t uartIn;
extern char error[40];
extern flag_t flag;
extern uint32_t version;
uint8_t uartRx[32];
uint8_t uartIn;
uint8_t uartPos;
uint8_t len;


void uartInit()
{
	HAL_UART_Receive_IT(&huart1, &uartIn, 1);
}

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
	case UART_READ:
		if (uartRx[1]=='1')
			sendConfig(1);
		if (uartRx[1]=='2')
			sendConfig(2);
		break;
	case UART_FREQUENCY   :
		tmp = DecToInt(ptr, l);
		if(uartRx[1]=='1')
		settings.realFrequency1 = tmp;
		if(uartRx[1]=='2')
		settings.realFrequency2 = tmp;
		break;

	case UART_SF          :
		tmp=DecToInt(ptr, l);
		if(uartRx[1]=='1')
		settings.sf1 = tmp;
		if(uartRx[1]=='2')
		settings.sf2 = tmp;
		break;

	case UART_BW          :
		tmp=DecToInt(ptr, l);
		if(uartRx[1]=='1')
		settings.bw1=tmp;
		if(uartRx[1]=='2')
		settings.bw2=tmp;
		break;

	case UART_SYNCWORD    :

		tmp=HexToInt(ptr, l);
		if(uartRx[1]=='1')
		settings.sw1 = tmp;
		if(uartRx[1]=='2')
		settings.sw2 = tmp;
		break;

	case UART_PREAMBLE:
		tmp = DecToInt(ptr, l);
		if(uartRx[1]=='1')
		settings.preamble1 = tmp;
		if(uartRx[1]=='2')
		settings.preamble2 = tmp;
		break;

	case UART_CR:
		tmp = DecToInt(ptr, l);
		if(uartRx[1]=='1')
		settings.cr1=tmp;
		if(uartRx[1]=='2')
		settings.cr2=tmp;
		break;

	case UART_POWER:
		tmp = DecToInt(ptr, l);
		if(uartRx[1]=='1')
		settings.power1=tmp;
		if(uartRx[1]=='2')
		settings.power2=tmp;
		break;

	case UART_SAVE:
		flag.saveSettings=1;
		break;


	case UART_CALL:

		sprintf(tempString,"<rNv%lu>",version);
		HAL_UART_Transmit(&huart1, (uint8_t*)tempString, strlen(tempString), 100);
		break;

	default:
		HAL_UART_Transmit(&huart1, (uint8_t*)"Error", 5, 100);
		Error_Handler();
		break;

	}

}

void sendConfig(uint8_t num)
{
	char str[32];
	if (num==1)
	{
	sprintf(str,"<1%ld>",settings.realFrequency1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<2%u>",settings.sf1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<3%u>",settings.bw1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<4%X>",settings.sw1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<5%u>",settings.power1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<8%u>",settings.preamble1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<9%u>",settings.cr1);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	}
	if (num==2)
	{
	sprintf(str,"<1%ld>",settings.realFrequency2);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<2%u>",settings.sf2);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<3%u>",settings.bw2);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<4%X>",settings.sw2);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<5%u>",settings.power2);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<8%u>",settings.preamble2);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	sprintf(str,"<9%u>",settings.cr2);
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
	}
}



