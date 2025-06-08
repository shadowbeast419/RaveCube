
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
#include <main.hpp>
#include <FFT.hpp>
#include <Adc1.hpp>
#include <VoltageSignal.hpp>
#include <UartController.hpp>
#include <SettingsController.hpp>
#include <LedController.hpp>
#include <MovingAvgFilter.hpp>
#include <RaveCubeController.hpp>
#include <I2CController.hpp>
#include <BeatDetector.hpp>
#include <RaveCubeCommand.hpp>
#include <BoundaryCommand.hpp>
#include <FilterCommand.hpp>
#include <FactorCommand.hpp>

Adc1* adc1;

LedController* ledCtrl;
I2CController* i2c;
FFT_Result* fftResult;

// Declare globally to see the RAM usage
UartController uart2;
VoltageSignal vSignal;
MovingAvgFilter movingAvgFilter;
FFT fft;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void SendStreamingData(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	adc1 = Adc1::GetInstance();
	ledCtrl = LedController::GetInstance();
	i2c = I2CController::GetInstance();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();

	i2c->Init();
	adc1->Init();
	uart2.Init();

	// SettingsController settingsCtrl(i2c);

	ledCtrl->Init(&movingAvgFilter);

	RaveCubeCommand* raveCubeCommand[3];
	FilterCommand filterCommand(&uart2, i2c, &movingAvgFilter);
	FactorCommand factorCommmand(&uart2, i2c, ledCtrl);
	BoundaryCommand boundaryCommand(&uart2, i2c, ledCtrl);

	raveCubeCommand[0] = &filterCommand;
	raveCubeCommand[1] = &factorCommmand;
	raveCubeCommand[2] = &boundaryCommand;

	for(uint8_t i = 0; i < COMMAND_COUNT; i++)
	{
		raveCubeCommand[i]->Load();
	}

	BeatDetector beatDetector(&uart2, (uint32_t)SAMPLE_FREQ, (uint16_t)FFT_SAMPLE_COUNT);
	beatDetector.EnableOutputToUart = true;
	beatDetector.UseAbsValueOfCorrelation = true;

	CorrelationResult corrResult = {0};
	FilterLevelsColor calculatedFilterLevels = {0};
	bool calcSuccessful = false;

	#ifdef ENABLE_STARTUP_SEQUENCE
	uint32_t iterationCounter = 0;
	bool startupIterationsComplete = false;
	#endif

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

			#ifdef ENABLE_STARTUP_SEQUENCE

			ledCtrl->CalculateBrightness(fftResult, vSignal.GetRMSValue(), startupIterationsComplete);

			iterationCounter++;

			// At this point the Filters have a valid result
			if(iterationCounter > FilterLevelsVoltage::FilterLevelsMax * 2)
				startupIterationsComplete = true;

			#endif
			
			#ifndef ENABLE_STARTUP_SEQUENCE
			ledCtrl->CalculateBrightness(fftResult, vSignal.GetMeanValue(), true);	
			#endif
			
			#ifdef VERBOSE_MODE
			SendStreamingData();
			#endif

			corrResult = beatDetector.CalculateBeatsPerMinute(ledCtrl->GetCurrentUnfilteredBrightness());
			calcSuccessful = beatDetector.CalculateFilterLevels(corrResult, &calculatedFilterLevels);

			if(calcSuccessful)
			{
				// raveCtrl.ChangeFilterOrder(updatedFilterLevels, false);
				// ledCtrl->SetColorFilterOrder(calculatedFilterLevels);
			}

			// raveCtrl.SendStreamingData();
		}

		if(uart2.IsNewRxDataAvailable() == SET)
		{
			uint8_t* msgStr = uart2.GetRxMessage();
			// raveCtrl.ExecuteCommand(msgStr);

			for(int i = 0; i < COMMAND_COUNT; i++)
			{
				CommandStatus cmdStatus = raveCubeCommand[i]->Parse((char*)msgStr);

				// Look for the next command if invalid
				if(cmdStatus != Valid)
				{
					raveCubeCommand[i]->SendCommandResponse(cmdStatus);
					continue;
				}

				// Valid command recognized
				raveCubeCommand[i]->Save();
			}
		}
	}
}

void SendStreamingData()
{
	char transmitBuffer[100];

	for(uint16_t i = 0; i < MAX_FFT_RESULT_INDEX; i++)
	{
		sprintf(transmitBuffer, "_fft %u %f_\n", fftResult[i].frequency, fftResult[i].absoluteValue);

		uart2.Transmit((uint8_t*)transmitBuffer);
		while(uart2.IsTxBusy());
	}

	sprintf(transmitBuffer, "_volt %f %f_\n", movingAvgFilter.GetAverageVoltage(), movingAvgFilter.GetAveragePeakVoltage());

	uart2.Transmit((uint8_t*)transmitBuffer);
	while(uart2.IsTxBusy());
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

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT


void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif
