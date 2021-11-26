/*
 * LedControllerTimer3.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: Christoph
 */

#include <PwmGenerator.h>

//PB6

PwmGenerator::PwmGenerator()
{

}

void PwmGenerator::InitPwm()
{


	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* TIMx clock enable */
	__HAL_RCC_TIM3_CLK_ENABLE();

	/* Enable GPIO Channel3/3N Clocks */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* Enable DMA clock */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/*##-2- Configure the NVIC for DMA #########################################*/
	/* NVIC configuration for DMA transfer complete interrupt */
	HAL_NVIC_SetPriority(DMA2_Channel1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Channel1_IRQn);

	for(uint32_t i = 0; i < _pwmDmaBufferSize; i++)
	{
		_aCCValueBuffer[i] = 0;
	}

	TIM_OC_InitTypeDef sConfig;

	_uwTimerPeriod = (SystemCoreClock / _pwmFrequency) - 1;
	_uwCCValueLow = (_dutyCycleLow * (_uwTimerPeriod - 1)) / 100;
	_uwCCValueHigh = (_dutyCycleHigh * (_uwTimerPeriod - 1)) / 100;

	_timHandlePwmDma.Instance = _timerInstance;

	_timHandlePwmDma.Init.Period            = _uwTimerPeriod;
	_timHandlePwmDma.Init.RepetitionCounter = 0;
	_timHandlePwmDma.Init.Prescaler         = 0;
	_timHandlePwmDma.Init.ClockDivision     = 0;
	_timHandlePwmDma.Init.CounterMode       = TIM_COUNTERMODE_UP;

	if (HAL_TIM_PWM_Init(&_timHandlePwmDma) != HAL_OK)
	{
		Error_Handler();
	}

	/*##-2- Configure the PWM channel 1 ########################################*/
	sConfig.OCMode       = TIM_OCMODE_PWM1;
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.Pulse        = this->_aCCValueBuffer[0];
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&_timHandlePwmDma, &sConfig, _timerChannel) != HAL_OK)
	{
		/* Configuration Error */
		Error_Handler();
	}

	EnablePwmOutput();

	/*##-3- Start PWM signal generation in DMA mode ############################*/
	if (HAL_TIM_PWM_Start_DMA(&_timHandlePwmDma,
			_timerChannel,
			(uint32_t*) _aCCValueBuffer ,
			_pwmDmaBufferSize) != HAL_OK)
	{
		/* Starting Error */
		Error_Handler();
	}
}

void PwmGenerator::EnablePwmOutput()
{
	GPIO_InitTypeDef   GPIO_InitStruct;
	/* Configure TIM3_Channel3 in output, push-pull & alternate function mode */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void PwmGenerator::DisablePwmOutput()
{
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);
}

void PwmGenerator::HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	static DMA_HandleTypeDef  hdma_tim;

	/* Set the parameters to be configured */
	hdma_tim.Init.Request  = DMA_REQUEST_TIM3_CH3;
	hdma_tim.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_tim.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_tim.Init.MemInc = DMA_MINC_ENABLE;
	hdma_tim.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD ;
	hdma_tim.Init.MemDataAlignment = DMA_MDATAALIGN_WORD ;
	hdma_tim.Init.Mode = DMA_CIRCULAR;
	hdma_tim.Init.Priority = DMA_PRIORITY_HIGH;
//	hdma_tim.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
//	hdma_tim.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
//	hdma_tim.Init.MemBurst = DMA_MBURST_SINGLE;
//	hdma_tim.Init.PeriphBurst = DMA_PBURST_SINGLE;

	/* Set hdma_tim instance */
	hdma_tim.Instance = DMA2_Channel1;

	/* Link hdma_tim to hdma[TIM_DMA_ID_CC3] (channel3) */
	__HAL_LINKDMA(htim, hdma[TIM_DMA_ID_CC3], hdma_tim);

	/* Initialize TIMx DMA handle */
	// HAL_DMA_Init(htim->hdma[TIM_DMA_ID_CC3]);

	if (HAL_DMA_Init(htim->hdma[TIM_DMA_ID_CC3]) != HAL_OK)
	{
	/* Initialization Error */
	while(1);
	}

	//  if(HAL_DMA_RegisterCallback(&hdma_tim, HAL_DMA_XFER_CPLT_CB_ID, &TIM_DMA_TransferComplete) != HAL_OK)
	//  {
	//	while(1);
	//  }
}

HAL_StatusTypeDef PwmGenerator::HAL_TIM_PWM_Init(TIM_HandleTypeDef *htim)
{
  /* Check the TIM handle allocation */
  if (htim == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_TIM_INSTANCE(htim->Instance));
  assert_param(IS_TIM_COUNTER_MODE(htim->Init.CounterMode));
  assert_param(IS_TIM_CLOCKDIVISION_DIV(htim->Init.ClockDivision));
  assert_param(IS_TIM_AUTORELOAD_PRELOAD(htim->Init.AutoReloadPreload));

  if (htim->State == HAL_TIM_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    htim->Lock = HAL_UNLOCKED;

#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
    /* Reset interrupt callbacks to legacy weak callbacks */
    TIM_ResetCallback(htim);

    if (htim->PWM_MspInitCallback == NULL)
    {
      htim->PWM_MspInitCallback = HAL_TIM_PWM_MspInit;
    }
    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    htim->PWM_MspInitCallback(htim);
#else
    /* Init the low level hardware : GPIO, CLOCK, NVIC and DMA */
    HAL_TIM_PWM_MspInit(htim);
#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
  }

  /* Set the TIM state */
  htim->State = HAL_TIM_STATE_BUSY;

  /* Init the base time for the PWM */
  TIM_Base_SetConfig(htim->Instance, &htim->Init);

  /* Initialize the DMA burst operation state */
  htim->DMABurstState = HAL_DMA_BURST_STATE_READY;

  /* Initialize the TIM channels state */
  TIM_CHANNEL_STATE_SET_ALL(htim, HAL_TIM_CHANNEL_STATE_READY);
  TIM_CHANNEL_N_STATE_SET_ALL(htim, HAL_TIM_CHANNEL_STATE_READY);

  /* Initialize the TIM state*/
  htim->State = HAL_TIM_STATE_READY;

  return HAL_OK;
}

void PwmGenerator::DmaInterruptHandler()
{
	HAL_DMA_IRQHandler(_timHandlePwmDma.hdma[TIM_DMA_ID_CC3]);
}

void PwmGenerator::Error_Handler()
{

}

PwmGenerator::~PwmGenerator()
{
	// TODO Auto-generated destructor stub
}

