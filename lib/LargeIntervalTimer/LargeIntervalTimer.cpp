/*
 * VoltagePeakTimer.cpp
 *
 *  Created on: 6 May 2021
 *      Author: Christoph
 */

#include <LargeIntervalTimer.h>

LargeIntervalTimer::LargeIntervalTimer()
{
  instance = TIM4;
  _intervalSeconds = 0.5f;
}

LargeIntervalTimer::LargeIntervalTimer(TIM_TypeDef* timerInstance, float32_t intervalSeconds)
{
  instance = timerInstance;
  _intervalSeconds = intervalSeconds;
}

void LargeIntervalTimer::Init()
{
	/* Set timer instance */
	_timHandle.Instance = instance;

	uint32_t timer_prescaler = 0xFFFF - 1; 						//(65535)
	uint32_t timer_clock_frequency = (SystemCoreClock / (timer_prescaler));             /* Timer clock frequency */

	float32_t period = (timer_clock_frequency * _intervalSeconds) - 1.0f;

	/* Configure timer parameters */
	_timHandle.Init.Period            = (uint32_t)period;
	_timHandle.Init.Prescaler         = timer_prescaler;
	_timHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	_timHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	_timHandle.Init.RepetitionCounter = 0x0;
	_timHandle.Init.AutoReloadPreload	= 0x0;

	if (this->HAL_TIM_Base_Init(&_timHandle) != HAL_OK)
	{
	/* Timer initialization Error */
		while(1);
	}

	if (HAL_TIM_Base_Start_IT(&_timHandle) != HAL_OK)
	{
	/* Starting Error */
		while(1);
	}
}

HAL_StatusTypeDef LargeIntervalTimer::HAL_TIM_Base_Init(TIM_HandleTypeDef *htim)
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
    this->HAL_TIM_Base_MspInit(htim);
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

void LargeIntervalTimer::HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	__HAL_RCC_TIM4_CLK_ENABLE();

	HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

void LargeIntervalTimer::InterruptHandler()
{
	HAL_TIM_IRQHandler(&this->_timHandle);
}

LargeIntervalTimer::~LargeIntervalTimer()
{
	// TODO Auto-generated destructor stub
}

