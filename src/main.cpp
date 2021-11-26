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

/*
 * I2C1
 * -----
 * PA15 -> I2C1_SCL
 * PB7 -> I2C1_SDA
 *
 * ADC1
 * -----
 * PA0 -> ADC_In1
 *
 * TIM3_Channel3
 * -----------
 * PB0 -> PWM Output
 *
 * TIM2 -> ADC
 *
 *UART
 *-------
 *
 *
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <cstdint>
#include <FFT.h>
#include <Adc1.h>
#include <VoltageSignal.h>
#include <UartController.h>
#include <SettingsController.h>
#include <SettingsStructs.h>
#include <LedController.h>
#include <MovingAvgFilter.h>
#include <RaveCubeController.h>
#include <I2CController.h>

/* USER CODE BEGIN PV */
Adc1* adc1;
UartController uart2;
LedController* ledCtrl;
I2CController* i2c;
VoltageSignal vSignal;
FFT fft;
FFT_Result* fftResult;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
//static void MX_DMA_Init(void);
//static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

	adc1 = Adc1::GetInstance();
	ledCtrl = LedController::GetInstance();
	i2c = I2CController::GetInstance();

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();

//	GPIO_InitTypeDef GPIO_InitStruct = {0};

//	/*Configure GPIO pin : AdcTim Interrupt Toggle */
//	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_4 ;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	i2c->Init();
	SettingsController settingsCtrl(i2c);

	adc1->Init();
	uart2.Init();
	ledCtrl->Init(&settingsCtrl);

	RaveCubeController raveCtrl(ledCtrl, &uart2, &settingsCtrl);

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		if(ledCtrl->IsLedUpdateComplete())
		{
			ledCtrl->UpdateLEDColor();
		}

		if(adc1->IsConversionHalfComplete() || adc1->IsConversionComplete())
		{
			vSignal.UpdateAdcValues(adc1->GetAdcValues());
			fft.UpdateVoltageSignal(&vSignal, adc1->GetSampleFrequency());
			fftResult = fft.CalculateFFT();
			ledCtrl->CalculateBrightness(fftResult, vSignal.GetRMSValue());
			raveCtrl.SendStreamingData();
		}

		if(uart2.IsNewRxDataAvailable() == SET)
		{
			uint8_t* msgStr = uart2.GetRxMessage();
			raveCtrl.ExecuteCommand(msgStr);
		}

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
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Configure the main internal regulator output voltage
	*/
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
	Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
	Error_Handler();
	}
	/** Initializes the peripherals clocks
	*/
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_ADC12 | RCC_PERIPHCLK_I2C1;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
	Error_Handler();
	}

}


// This Callback method is called everytime the ADC has filled the data array
extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	adc1->AdcConvCompleteHandler();
}

extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	adc1->AdcConvHalfCompleteHandler();
}

extern "C" void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	ledCtrl->PulseFinishedHandler(htim, SET);
}

extern "C" void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
	// One led is finished, accValue buffer holds for 2
	ledCtrl->PulseFinishedHandler(htim, RESET);
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	ledCtrl->PeakTimerPeriodElapsedHandler(htim);
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uart2.TransmitCpltHandler();
}

extern "C" void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	i2c->TxCompleteHandler(hi2c);
}

//extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//
//}
//
//extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//
//}

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
