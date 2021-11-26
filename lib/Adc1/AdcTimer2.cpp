/*
 * AdcTimer2.cpp
 *
 *  Created on: 6 Oct 2020
 *      Author: Christoph
 */

#include <AdcTimer2.h>

AdcTimer2::AdcTimer2()
{

}

void AdcTimer2::Init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin : AdcTim Interrupt Toggle */
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	uint32_t timer_clock_frequency = 170000000;             /* Timer clock frequency */

	/* Timer prescaler calculation */
	/* (computation for timer 16 bits, additional + 1 to round the prescaler up) */
	// timer_prescaler = (timer_clock_frequency / (TIMER_PRESCALER_MAX_VALUE * TIMER_FREQUENCY_RANGE_MIN)) +1;

	/* Set timer instance */
	_timHandleAdcDma.Instance = TIM2;

	/* Configure timer parameters */
	_timHandleAdcDma.Init.Period            = ((uint32_t)((timer_clock_frequency / _sampleFrequency) - 1));
	// TimHandle.Init.Prescaler         = (timer_prescaler - 1);
	_timHandleAdcDma.Init.Prescaler 		 = 0;
	_timHandleAdcDma.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	_timHandleAdcDma.Init.CounterMode       = TIM_COUNTERMODE_UP;
	_timHandleAdcDma.Init.RepetitionCounter = 0x0;
	_timHandleAdcDma.Init.AutoReloadPreload	= 0x0;

	if (HAL_TIM_Base_Init(&_timHandleAdcDma) != HAL_OK)
	{
	/* Timer initialization Error */
	Error_Handler();
	}

	TIM_MasterConfigTypeDef master_timer_config;

//	/* Timer TRGO selection */
	master_timer_config.MasterOutputTrigger = TIM_TRGO_UPDATE;
	master_timer_config.MasterOutputTrigger2 = TIM_TRGO2_UPDATE;
	master_timer_config.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	if (HAL_TIMEx_MasterConfigSynchronization(&_timHandleAdcDma, &master_timer_config) != HAL_OK)
	{
	/* Timer TRGO selection Error */
	Error_Handler();
	}

	if(_enableTimerInterrupt)
	{
		/*##-2- Start the TIM Base generation in interrupt mode ####################*/
		/* Start Channel1 */
		if (HAL_TIM_Base_Start_IT(&_timHandleAdcDma) != HAL_OK)
		{
		/* Starting Error */
		Error_Handler();
		}
	}
	else
	{
		/*##-2- Start the TIM Base generation in interrupt mode ####################*/
		/* Start Channel1 */
		if (HAL_TIM_Base_Start(&_timHandleAdcDma) != HAL_OK)
		{
		/* Starting Error */
		Error_Handler();
		}
	}
}

HAL_StatusTypeDef AdcTimer2::HAL_TIM_Base_Init(TIM_HandleTypeDef *htim)
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

    if (htim->Base_MspInitCallback == NULL)
    {
      htim->Base_MspInitCallback = HAL_TIM_Base_MspInit;
    }
    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    htim->Base_MspInitCallback(htim);
#else
    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    HAL_TIM_Base_MspInit(htim);
#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
  }

  /* Set the TIM state */
  htim->State = HAL_TIM_STATE_BUSY;

  /* Set the Time Base configuration */
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

void AdcTimer2::HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	__HAL_RCC_TIM2_CLK_ENABLE();

	if(_enableTimerInterrupt)
	{
	    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
	    HAL_NVIC_EnableIRQ(TIM2_IRQn);
	}
}

void AdcTimer2::InterruptHandler()
{
	HAL_TIM_IRQHandler(&this->_timHandleAdcDma);
}

void AdcTimer2::Error_Handler()
{
	while(1)
	{

	}
}


AdcTimer2::~AdcTimer2()
{
	// TODO Auto-generated destructor stub
}

