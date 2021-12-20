/*
 * Adc1.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: chris
 */

#include <Adc1.hpp>

Adc1 Adc1::_adc1Instance;

Adc1::Adc1()
{

}

void Adc1::Init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin : AdcTim Interrupt Toggle */
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	ADC_MultiModeTypeDef multimode = {0};
	ADC_ChannelConfTypeDef sConfig = {0};

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */
	/** Common config
	*/
	_adcHandle.Instance = ADC1;
	_adcHandle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
	_adcHandle.Init.Resolution = ADC_RESOLUTION_12B;
	_adcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	_adcHandle.Init.GainCompensation = 0;
	_adcHandle.Init.ScanConvMode = ADC_SCAN_DISABLE;
	_adcHandle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	_adcHandle.Init.LowPowerAutoWait = DISABLE;
	_adcHandle.Init.ContinuousConvMode = DISABLE;
	_adcHandle.Init.NbrOfConversion = 1;
	_adcHandle.Init.DiscontinuousConvMode = DISABLE;
	_adcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T2_TRGO;
	_adcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	_adcHandle.Init.DMAContinuousRequests = ENABLE;
	_adcHandle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	_adcHandle.Init.OversamplingMode = DISABLE;

	if (HAL_ADC_Init(&_adcHandle) != HAL_OK)
	{
	Error_Handler();
	}
	/** Configure the ADC multi-mode
	*/
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&_adcHandle, &multimode) != HAL_OK)
	{
	Error_Handler();
	}
	/** Configure Regular Channel
	*/
	sConfig.Channel = _adcChannel;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&_adcHandle, &sConfig) != HAL_OK)
	{
	Error_Handler();
	}

	/* ### - 4 - Start conversion in DMA mode ################################# */
	if (HAL_ADC_Start_DMA(&_adcHandle,
						(uint32_t *)_adcConvertedData,
						2 * FFT_SAMPLE_COUNT
					   ) != HAL_OK)
	{
	Error_Handler();
	}

	_adcTimer2.Init();
}


Adc1::~Adc1()
{

}

void Adc1::Error_Handler(void)
{
	while(1)
	{

	}
}

HAL_StatusTypeDef Adc1::HAL_ADC_Init(ADC_HandleTypeDef *hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  uint32_t tmpCFGR;
  uint32_t tmp_adc_reg_is_conversion_on_going;
  __IO uint32_t wait_loop_index = 0UL;
  uint32_t tmp_adc_is_conversion_on_going_regular;
  uint32_t tmp_adc_is_conversion_on_going_injected;

  /* Check ADC handle */
  if (hadc == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_ADC_CLOCKPRESCALER(hadc->Init.ClockPrescaler));
  assert_param(IS_ADC_RESOLUTION(hadc->Init.Resolution));
  assert_param(IS_ADC_DATA_ALIGN(hadc->Init.DataAlign));
  assert_param(IS_ADC_GAIN_COMPENSATION(hadc->Init.GainCompensation));
  assert_param(IS_ADC_SCAN_MODE(hadc->Init.ScanConvMode));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.ContinuousConvMode));
  assert_param(IS_ADC_EXTTRIG_EDGE(hadc->Init.ExternalTrigConvEdge));
  assert_param(IS_ADC_EXTTRIG(hadc, hadc->Init.ExternalTrigConv));
  assert_param(IS_ADC_SAMPLINGMODE(hadc->Init.SamplingMode));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.DMAContinuousRequests));
  assert_param(IS_ADC_EOC_SELECTION(hadc->Init.EOCSelection));
  assert_param(IS_ADC_OVERRUN(hadc->Init.Overrun));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.LowPowerAutoWait));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.OversamplingMode));

  if (hadc->Init.ScanConvMode != ADC_SCAN_DISABLE)
  {
    assert_param(IS_ADC_REGULAR_NB_CONV(hadc->Init.NbrOfConversion));
    assert_param(IS_FUNCTIONAL_STATE(hadc->Init.DiscontinuousConvMode));

    if (hadc->Init.DiscontinuousConvMode == ENABLE)
    {
      assert_param(IS_ADC_REGULAR_DISCONT_NUMBER(hadc->Init.NbrOfDiscConversion));
    }
  }

  /* DISCEN and CONT bits cannot be set at the same time */
  assert_param(!((hadc->Init.DiscontinuousConvMode == ENABLE) && (hadc->Init.ContinuousConvMode == ENABLE)));

  /* Actions performed only if ADC is coming from state reset:                */
  /* - Initialization of ADC MSP                                              */
  if (hadc->State == HAL_ADC_STATE_RESET)
  {
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
    /* Init the ADC Callback settings */
    hadc->ConvCpltCallback              = HAL_ADC_ConvCpltCallback;                 /* Legacy weak callback */
    hadc->ConvHalfCpltCallback          = HAL_ADC_ConvHalfCpltCallback;             /* Legacy weak callback */
    hadc->LevelOutOfWindowCallback      = HAL_ADC_LevelOutOfWindowCallback;         /* Legacy weak callback */
    hadc->ErrorCallback                 = HAL_ADC_ErrorCallback;                    /* Legacy weak callback */
    hadc->InjectedConvCpltCallback      = HAL_ADCEx_InjectedConvCpltCallback;       /* Legacy weak callback */
    hadc->InjectedQueueOverflowCallback = HAL_ADCEx_InjectedQueueOverflowCallback;  /* Legacy weak callback */
    hadc->LevelOutOfWindow2Callback     = HAL_ADCEx_LevelOutOfWindow2Callback;      /* Legacy weak callback */
    hadc->LevelOutOfWindow3Callback     = HAL_ADCEx_LevelOutOfWindow3Callback;      /* Legacy weak callback */
    hadc->EndOfSamplingCallback         = HAL_ADCEx_EndOfSamplingCallback;          /* Legacy weak callback */

    if (hadc->MspInitCallback == NULL)
    {
      hadc->MspInitCallback = HAL_ADC_MspInit; /* Legacy weak MspInit  */
    }

    /* Init the low level hardware */
    hadc->MspInitCallback(hadc);
#else
    /* Init the low level hardware */
    HAL_ADC_MspInit(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */

    /* Set ADC error code to none */
    ADC_CLEAR_ERRORCODE(hadc);

    /* Initialize Lock */
    hadc->Lock = HAL_UNLOCKED;
  }

  /* - Exit from deep-power-down mode and ADC voltage regulator enable        */
  if (LL_ADC_IsDeepPowerDownEnabled(hadc->Instance) != 0UL)
  {
    /* Disable ADC deep power down mode */
    LL_ADC_DisableDeepPowerDown(hadc->Instance);

    /* System was in deep power down mode, calibration must
     be relaunched or a previously saved calibration factor
     re-applied once the ADC voltage regulator is enabled */
  }

  if (LL_ADC_IsInternalRegulatorEnabled(hadc->Instance) == 0UL)
  {
    /* Enable ADC internal voltage regulator */
    LL_ADC_EnableInternalRegulator(hadc->Instance);

    /* Note: Variable divided by 2 to compensate partially              */
    /*       CPU processing cycles, scaling in us split to not          */
    /*       exceed 32 bits register capacity and handle low frequency. */
    wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US / 10UL) * (SystemCoreClock / (100000UL * 2UL)));
    while (wait_loop_index != 0UL)
    {
      wait_loop_index--;
    }
  }

  /* Verification that ADC voltage regulator is correctly enabled, whether    */
  /* or not ADC is coming from state reset (if any potential problem of       */
  /* clocking, voltage regulator would not be enabled).                       */
  if (LL_ADC_IsInternalRegulatorEnabled(hadc->Instance) == 0UL)
  {
    /* Update ADC state machine to error */
    SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL);

    /* Set ADC error code to ADC peripheral internal error */
    SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);

    tmp_hal_status = HAL_ERROR;
  }

  /* Configuration of ADC parameters if previous preliminary actions are      */
  /* correctly completed and if there is no conversion on going on regular    */
  /* group (ADC may already be enabled at this point if HAL_ADC_Init() is     */
  /* called to update a parameter on the fly).                                */
  tmp_adc_reg_is_conversion_on_going = LL_ADC_REG_IsConversionOngoing(hadc->Instance);

  if (((hadc->State & HAL_ADC_STATE_ERROR_INTERNAL) == 0UL)
      && (tmp_adc_reg_is_conversion_on_going == 0UL)
     )
  {
    /* Set ADC state */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_REG_BUSY,
                      HAL_ADC_STATE_BUSY_INTERNAL);

    /* Configuration of common ADC parameters                                 */

    /* Parameters update conditioned to ADC state:                            */
    /* Parameters that can be updated only when ADC is disabled:              */
    /*  - clock configuration                                                 */
    if (LL_ADC_IsEnabled(hadc->Instance) == 0UL)
    {
      if (__LL_ADC_IS_ENABLED_ALL_COMMON_INSTANCE(__LL_ADC_COMMON_INSTANCE(hadc->Instance)) == 0UL)
      {
        /* Reset configuration of ADC common register CCR:                      */
        /*                                                                      */
        /*   - ADC clock mode and ACC prescaler (CKMODE and PRESC bits)are set  */
        /*     according to adc->Init.ClockPrescaler. It selects the clock      */
        /*    source and sets the clock division factor.                        */
        /*                                                                      */
        /* Some parameters of this register are not reset, since they are set   */
        /* by other functions and must be kept in case of usage of this         */
        /* function on the fly (update of a parameter of ADC_InitTypeDef        */
        /* without needing to reconfigure all other ADC groups/channels         */
        /* parameters):                                                         */
        /*   - when multimode feature is available, multimode-related           */
        /*     parameters: MDMA, DMACFG, DELAY, DUAL (set by API                */
        /*     HAL_ADCEx_MultiModeConfigChannel() )                             */
        /*   - internal measurement paths: Vbat, temperature sensor, Vref       */
        /*     (set into HAL_ADC_ConfigChannel() or                             */
        /*     HAL_ADCEx_InjectedConfigChannel() )                              */
        LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(hadc->Instance), hadc->Init.ClockPrescaler);
      }
    }

    /* Configuration of ADC:                                                  */
    /*  - resolution                               Init.Resolution            */
    /*  - data alignment                           Init.DataAlign             */
    /*  - external trigger to start conversion     Init.ExternalTrigConv      */
    /*  - external trigger polarity                Init.ExternalTrigConvEdge  */
    /*  - continuous conversion mode               Init.ContinuousConvMode    */
    /*  - overrun                                  Init.Overrun               */
    /*  - discontinuous mode                       Init.DiscontinuousConvMode */
    /*  - discontinuous mode channel count         Init.NbrOfDiscConversion   */
    tmpCFGR  = (ADC_CFGR_CONTINUOUS((uint32_t)hadc->Init.ContinuousConvMode)           |
                hadc->Init.Overrun                                                     |
                hadc->Init.DataAlign                                                   |
                hadc->Init.Resolution                                                  |
                ADC_CFGR_REG_DISCONTINUOUS((uint32_t)hadc->Init.DiscontinuousConvMode));

    if (hadc->Init.DiscontinuousConvMode == ENABLE)
    {
      tmpCFGR |= ADC_CFGR_DISCONTINUOUS_NUM(hadc->Init.NbrOfDiscConversion);
    }

    /* Enable external trigger if trigger selection is different of software  */
    /* start.                                                                 */
    /* Note: This configuration keeps the hardware feature of parameter       */
    /*       ExternalTrigConvEdge "trigger edge none" equivalent to           */
    /*       software start.                                                  */
    if (hadc->Init.ExternalTrigConv != ADC_SOFTWARE_START)
    {
      tmpCFGR |= ((hadc->Init.ExternalTrigConv & ADC_CFGR_EXTSEL)
                  | hadc->Init.ExternalTrigConvEdge
                 );
    }

    /* Update Configuration Register CFGR */
    MODIFY_REG(hadc->Instance->CFGR, ADC_CFGR_FIELDS_1, tmpCFGR);

    /* Configuration of sampling mode */
    MODIFY_REG(hadc->Instance->CFGR2, ADC_CFGR2_BULB | ADC_CFGR2_SMPTRIG, hadc->Init.SamplingMode);

    /* Parameters update conditioned to ADC state:                            */
    /* Parameters that can be updated when ADC is disabled or enabled without */
    /* conversion on going on regular and injected groups:                    */
    /*  - Gain Compensation               Init.GainCompensation               */
    /*  - DMA continuous request          Init.DMAContinuousRequests          */
    /*  - LowPowerAutoWait feature        Init.LowPowerAutoWait               */
    /*  - Oversampling parameters         Init.Oversampling                   */
    tmp_adc_is_conversion_on_going_regular = LL_ADC_REG_IsConversionOngoing(hadc->Instance);
    tmp_adc_is_conversion_on_going_injected = LL_ADC_INJ_IsConversionOngoing(hadc->Instance);
    if ((tmp_adc_is_conversion_on_going_regular == 0UL)
        && (tmp_adc_is_conversion_on_going_injected == 0UL)
       )
    {
      tmpCFGR = (ADC_CFGR_DFSDM(hadc)                                            |
                 ADC_CFGR_AUTOWAIT((uint32_t)hadc->Init.LowPowerAutoWait)        |
                 ADC_CFGR_DMACONTREQ((uint32_t)hadc->Init.DMAContinuousRequests));

      MODIFY_REG(hadc->Instance->CFGR, ADC_CFGR_FIELDS_2, tmpCFGR);

      if (hadc->Init.GainCompensation != 0UL)
      {
        SET_BIT(hadc->Instance->CFGR2, ADC_CFGR2_GCOMP);
        MODIFY_REG(hadc->Instance->GCOMP, ADC_GCOMP_GCOMPCOEFF, hadc->Init.GainCompensation);
      }
      else
      {
        CLEAR_BIT(hadc->Instance->CFGR2, ADC_CFGR2_GCOMP);
        MODIFY_REG(hadc->Instance->GCOMP, ADC_GCOMP_GCOMPCOEFF, 0UL);
      }

      if (hadc->Init.OversamplingMode == ENABLE)
      {
        assert_param(IS_ADC_OVERSAMPLING_RATIO(hadc->Init.Oversampling.Ratio));
        assert_param(IS_ADC_RIGHT_BIT_SHIFT(hadc->Init.Oversampling.RightBitShift));
        assert_param(IS_ADC_TRIGGERED_OVERSAMPLING_MODE(hadc->Init.Oversampling.TriggeredMode));
        assert_param(IS_ADC_REGOVERSAMPLING_MODE(hadc->Init.Oversampling.OversamplingStopReset));

        /* Configuration of Oversampler:                                      */
        /*  - Oversampling Ratio                                              */
        /*  - Right bit shift                                                 */
        /*  - Triggered mode                                                  */
        /*  - Oversampling mode (continued/resumed)                           */
        MODIFY_REG(hadc->Instance->CFGR2,
                   ADC_CFGR2_OVSR  |
                   ADC_CFGR2_OVSS  |
                   ADC_CFGR2_TROVS |
                   ADC_CFGR2_ROVSM,
                   ADC_CFGR2_ROVSE                       |
                   hadc->Init.Oversampling.Ratio         |
                   hadc->Init.Oversampling.RightBitShift |
                   hadc->Init.Oversampling.TriggeredMode |
                   hadc->Init.Oversampling.OversamplingStopReset
                  );
      }
      else
      {
        /* Disable ADC oversampling scope on ADC group regular */
        CLEAR_BIT(hadc->Instance->CFGR2, ADC_CFGR2_ROVSE);
      }

    }

    /* Configuration of regular group sequencer:                              */
    /* - if scan mode is disabled, regular channels sequence length is set to */
    /*   0x00: 1 channel converted (channel on regular rank 1)                */
    /*   Parameter "NbrOfConversion" is discarded.                            */
    /*   Note: Scan mode is not present by hardware on this device, but       */
    /*   emulated by software for alignment over all STM32 devices.           */
    /* - if scan mode is enabled, regular channels sequence length is set to  */
    /*   parameter "NbrOfConversion".                                         */

    if (hadc->Init.ScanConvMode == ADC_SCAN_ENABLE)
    {
      /* Set number of ranks in regular group sequencer */
      MODIFY_REG(hadc->Instance->SQR1, ADC_SQR1_L, (hadc->Init.NbrOfConversion - (uint8_t)1));
    }
    else
    {
      CLEAR_BIT(hadc->Instance->SQR1, ADC_SQR1_L);
    }

    /* Initialize the ADC state */
    /* Clear HAL_ADC_STATE_BUSY_INTERNAL bit, set HAL_ADC_STATE_READY bit */
    ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_BUSY_INTERNAL, HAL_ADC_STATE_READY);
  }
  else
  {
    /* Update ADC state machine to error */
    SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL);

    tmp_hal_status = HAL_ERROR;
  }

  /* Return function status */
  return tmp_hal_status;
}

void Adc1::HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

	/* DMA controller clock enable */
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    _dmaHandle.Instance = DMA1_Channel1;
    _dmaHandle.Init.Request = DMA_REQUEST_ADC1;
    _dmaHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    _dmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    _dmaHandle.Init.MemInc = DMA_MINC_ENABLE;
    _dmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    _dmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    _dmaHandle.Init.Mode = DMA_CIRCULAR;
    _dmaHandle.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&_dmaHandle) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&_adcHandle,DMA_Handle,_dmaHandle);

	/* NVIC configuration for DMA Input data interrupt */
	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	/* NVIC configuration for ADC interrupt */
	/* Priority: high-priority */
//	HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
//	HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }

}

void Adc1::AdcConvCompleteHandler(void)
{
	_adcConversionFinished = SET;
	_adcConversionHalfFinished = RESET;
}

void Adc1::AdcConvHalfCompleteHandler(void)
{
	_adcConversionHalfFinished = SET;
	_adcConversionFinished = RESET;
}

uint8_t Adc1::IsConversionComplete(void)
{
	return _adcConversionFinished;
}

uint8_t	Adc1::IsConversionHalfComplete(void)
{
	return _adcConversionHalfFinished;
}

void Adc1::DmaInterruptHandler(void)
{
	HAL_DMA_IRQHandler(&_dmaHandle);
}

void Adc1::TimInterruptHandler(void)
{
	_adcTimer2.InterruptHandler();
}

uint16_t* Adc1::GetAdcValues(void)
{
	// Calculation is too slow
	if(_adcConversionHalfFinished && _adcConversionFinished)
	{
		_adcConversionHalfFinished = RESET;
		_adcConversionFinished = RESET;
		// return the last part because it is newer
		return &_adcConvertedData[FFT_SAMPLE_COUNT];
	}
	else if(_adcConversionHalfFinished)
	{
		_adcConversionHalfFinished = RESET;
		return _adcConvertedData;
	}
	else if(_adcConversionFinished)
	{
		_adcConversionFinished = RESET;
		return &_adcConvertedData[FFT_SAMPLE_COUNT];
	}

	return _adcConvertedData;
}

uint32_t Adc1::GetSampleFrequency(void)
{
	return this->_sampleFrequency;
}
