/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#define RANGE_TEST
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
repeaterSettings_t *flashSettings = (repeaterSettings_t*) 0x0800FC00;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

char string[32];

SX127X_t myRadio1;
SX127X_t myRadio2;
repeaterSettings_t settings;
char error[40];
flag_t flag;
const uint32_t version = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void saveSettings() {
	uint16_t i = 0;
	uint16_t const settingsSize = (sizeof(repeaterSettings_t) + 3) / 4;
	FLASH_EraseInitTypeDef eraseInit;
	uint32_t pageError;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.PageAddress = (uint32_t) flashSettings;
	eraseInit.NbPages = 1;
	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&eraseInit, &pageError);
	for (i = 0; i < settingsSize; i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				((uint32_t) flashSettings) + 4 * i,
				*(((uint32_t*) &settings) + i));
	HAL_FLASH_Lock();
}

void settingsInitiate() {
	myRadio1.sf = settings.sf1;
	myRadio1.bw = settings.bw1;
	myRadio1.cr = settings.cr1;
	myRadio1.frequency = (uint32_t) (settings.realFrequency1 / 61.035f);
	myRadio1.preamble = settings.preamble1;
	myRadio1.power = settings.power1;
	myRadio1.syncWord = settings.sw1;

	myRadio2.sf = settings.sf2;
	myRadio2.bw = settings.bw2;
	myRadio2.cr = settings.cr2;
	myRadio2.frequency = (uint32_t) (settings.realFrequency2 / 61.035f);
	myRadio2.preamble = settings.preamble2;
	myRadio2.power = settings.power2;
	myRadio2.syncWord = settings.sw2;

}

uint8_t tryLoadSettings() {
	if (flashSettings->realFrequency1 >= MIN_FREQUENCY
			&& flashSettings->realFrequency1 <= MAX_FREQUENCY)
		if (flashSettings->sf1 > 6 && flashSettings->sf1 < 13)
			if (flashSettings->bw1 < 10)
				if (flashSettings->cr1 > 0 && flashSettings->cr1 < 5)
					if (flashSettings->preamble1 > 1
							&& flashSettings->preamble1 < 65535)
						if (flashSettings->sw1 != 0x34)
							if (flashSettings->power1 < 21
									|| flashSettings->power1 > 9)
								if (flashSettings->realFrequency2
										>= MIN_FREQUENCY
										&& flashSettings->realFrequency2
												<= MAX_FREQUENCY)
									if (flashSettings->sf2 > 6
											&& flashSettings->sf2 < 13)
										if (flashSettings->bw2 < 10)
											if (flashSettings->cr2 > 0
													&& flashSettings->cr2 < 5)
												if (flashSettings->preamble2 > 1
														&& flashSettings->preamble2
																< 65535)
													if (flashSettings->sw2
															!= 0x34)
														if (flashSettings->power2
																< 21
																|| flashSettings->power2
																		> 9) {
															settings =
																	*flashSettings;
															settingsInitiate();
															return 1;
														}
	return 0;
}

void useDefaultSettings() {
	settings.realFrequency1 = DEF_FREQUENCY;
	settings.sf1 = 12;
	settings.bw1 = 7;
	settings.cr1 = 4;
	settings.power1 = 20;
	settings.preamble1 = 5;
	settings.sw1 = 1;

	settings.realFrequency2 = DEF_FREQUENCY2;
	settings.sf2 = 10;
	settings.bw2 = 7;
	settings.cr2 = 4;
	settings.power2 = 20;
	settings.preamble2 = 5;
	settings.sw2 = 1;

	settingsInitiate();
}

void RadioInit() {
	SX127X_dio_t nss1;
	SX127X_dio_t nss2;
	SX127X_dio_t reset1;
	SX127X_dio_t reset2;

	nss1.pin = NSS1_Pin;
	nss1.port = NSS1_GPIO_Port;
	reset1.pin = RESET1_Pin;
	reset1.port = RESET1_GPIO_Port;
	nss2.pin = NSS2_Pin;
	nss2.port = NSS2_GPIO_Port;
	reset2.pin = RESET2_Pin;
	reset2.port = RESET2_GPIO_Port;

	SX127X_PortConfig(&myRadio1, reset1, nss1, &hspi1);
	SX127X_PortConfig(&myRadio2, reset2, nss2, &hspi1);

	myRadio1.alwaysRX = true;
	myRadio1.implicitHeader = true;
	myRadio1.crcEnable = true;
	myRadio1.len = 3;

	myRadio2.alwaysRX = true;
	myRadio2.implicitHeader = true;
	myRadio2.crcEnable = true;
	myRadio2.len = 3;

	if (tryLoadSettings() == 0)
		useDefaultSettings();
#ifdef RANGE_TEST
	myRadio2.frequency = myRadio1.frequency;
	myRadio2.sf = myRadio1.sf;
	myRadio2.cr = myRadio1.cr;
	myRadio2.bw = myRadio1.bw;
	myRadio2.preamble = myRadio1.preamble;
	myRadio2.syncWord = myRadio1.syncWord;
	myRadio2.power = myRadio1.power;

#endif
	SX127X_init(&myRadio1);
	SX127X_init(&myRadio2);

	SX127X_config(&myRadio1);
	SX127X_config(&myRadio2);
}

void LedRoutine() {
	uint8_t blinker = (HAL_GetTick() % 50) > 25;

	if (myRadio1.status == TX)
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	else if (myRadio1.signalDetected)
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, blinker);
	else
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

	if (myRadio2.status == TX)
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	else if (myRadio2.signalDetected)
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, blinker);
	else
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	readByte();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	RadioInit();

	uartInit();
#ifdef RANGE_TEST

	myRadio1.txBuf[0] = 255;
	myRadio1.txBuf[1] = 255;
	myRadio1.txBuf[2] = 1;
	myRadio2.txBuf[0] = 255;
	myRadio2.txBuf[1] = 255;
	myRadio2.txBuf[2] = 2;
#endif
	while (1) {
		HAL_IWDG_Refresh(&hiwdg);
#ifdef RANGE_TEST

		static uint32_t lastSend = 0;
		static bool first = true;
		if (HAL_GetTick() - lastSend > 1500) {
			lastSend = HAL_GetTick();
			if (first)
				SX127X_transmitAsync(&myRadio1, 3);
			else
				SX127X_transmitAsync(&myRadio2, 3);
			first = !first;
		}
#endif
		SX127X_Routine(&myRadio1);
		SX127X_Routine(&myRadio2);

		if (flag.saveSettings) {
			flag.saveSettings = 0;
			saveSettings();
		}

		if (flag.uartRx) {
			flag.uartRx = 0;

			uartReceiveHandler();

		}
#ifndef RANGE_TEST
		if (myRadio1.readBytes > 0) {
			if (myRadio1.badCrc) {
				sprintf(string, "1 to 2: Bad CRC\n");
				HAL_UART_Transmit(&huart1, (uint8_t*) string, strlen(string), 100);
				myRadio1.readBytes = 0;
				continue;
			}
			memcpy(myRadio2.txBuf, myRadio1.rxBuf, myRadio1.readBytes);
			sprintf(string, "1 to 2:  %02x %02x %02x\n", myRadio1.rxBuf[0],
					myRadio1.rxBuf[1], myRadio1.rxBuf[2]);
			HAL_UART_Transmit(&huart1, (uint8_t*) string, strlen(string), 100);
			SX127X_transmitAsync(&myRadio2, myRadio1.readBytes);
			myRadio1.readBytes = 0;
		}

		if (myRadio2.readBytes > 0) {
			if (myRadio2.badCrc) {
				sprintf(string, "2 to 1: Bad CRC\n");
				HAL_UART_Transmit(&huart1, (uint8_t*) string, strlen(string), 100);
				myRadio2.readBytes = 0;
				continue;
			}
			memcpy(myRadio1.txBuf, myRadio2.rxBuf, myRadio2.readBytes);
			sprintf(string, "2 to 1: %02x %02x %02x\n", myRadio2.rxBuf[0],
					myRadio2.rxBuf[1], myRadio2.rxBuf[2]);
			HAL_UART_Transmit(&huart1, (uint8_t*) string, strlen(string), 100);
			SX127X_transmitAsync(&myRadio1, myRadio2.readBytes);
			myRadio2.readBytes = 0;
		}
#endif
		LedRoutine();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_16;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED2_Pin|NSS2_Pin|RESET2_Pin|RESET1_Pin
                          |NSS1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED2_Pin NSS2_Pin RESET2_Pin RESET1_Pin
                           NSS1_Pin */
  GPIO_InitStruct.Pin = LED2_Pin|NSS2_Pin|RESET2_Pin|RESET1_Pin
                          |NSS1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
