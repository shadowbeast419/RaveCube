/*
 * I2C.cpp
 *
 *  Created on: 21 Mar 2021
 *      Author: Christoph
 */

#include <I2CController.h>

I2CController I2CController::_i2cInstance;

I2CController::I2CController()
{
	// TODO Auto-generated constructor stub

}

void I2CController::Init()
{
	/* USER CODE END I2C1_Init 1 */
	_hi2c1.Instance = I2C1;
	_hi2c1.Init.Timing = 0x70F3546F;
	_hi2c1.Init.OwnAddress1 = 160;
	_hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	_hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	_hi2c1.Init.OwnAddress2 = 0;
	_hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	_hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	_hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (this->HAL_I2C_Init(&_hi2c1) != HAL_OK)
	{
	Error_Handler();
	}
	/** Configure Analogue filter
	*/
	if (HAL_I2CEx_ConfigAnalogFilter(&_hi2c1, I2C_ANALOGFILTER_DISABLE) != HAL_OK)
	{
	Error_Handler();
	}
	/** Configure Digital filter
	*/
	if (HAL_I2CEx_ConfigDigitalFilter(&_hi2c1, 0x0) != HAL_OK)
	{
	Error_Handler();
	}
	/** I2C Fast mode Plus enable
	*/
}

HAL_StatusTypeDef I2CController::HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
	/* Check the I2C handle allocation */
	if (hi2c == NULL)
	{
	return HAL_ERROR;
	}

	/* Check the parameters */
	assert_param(IS_I2C_ALL_INSTANCE(hi2c->Instance));
	assert_param(IS_I2C_OWN_ADDRESS1(hi2c->Init.OwnAddress1));
	assert_param(IS_I2C_ADDRESSING_MODE(hi2c->Init.AddressingMode));
	assert_param(IS_I2C_DUAL_ADDRESS(hi2c->Init.DualAddressMode));
	assert_param(IS_I2C_OWN_ADDRESS2(hi2c->Init.OwnAddress2));
	assert_param(IS_I2C_OWN_ADDRESS2_MASK(hi2c->Init.OwnAddress2Masks));
	assert_param(IS_I2C_GENERAL_CALL(hi2c->Init.GeneralCallMode));
	assert_param(IS_I2C_NO_STRETCH(hi2c->Init.NoStretchMode));

	if (hi2c->State == HAL_I2C_STATE_RESET)
	{
	/* Allocate lock resource and initialize it */
	hi2c->Lock = HAL_UNLOCKED;

	#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
	/* Init the I2C Callback settings */
	hi2c->MasterTxCpltCallback = HAL_I2C_MasterTxCpltCallback; /* Legacy weak MasterTxCpltCallback */
	hi2c->MasterRxCpltCallback = HAL_I2C_MasterRxCpltCallback; /* Legacy weak MasterRxCpltCallback */
	hi2c->SlaveTxCpltCallback  = HAL_I2C_SlaveTxCpltCallback;  /* Legacy weak SlaveTxCpltCallback  */
	hi2c->SlaveRxCpltCallback  = HAL_I2C_SlaveRxCpltCallback;  /* Legacy weak SlaveRxCpltCallback  */
	hi2c->ListenCpltCallback   = HAL_I2C_ListenCpltCallback;   /* Legacy weak ListenCpltCallback   */
	hi2c->MemTxCpltCallback    = HAL_I2C_MemTxCpltCallback;    /* Legacy weak MemTxCpltCallback    */
	hi2c->MemRxCpltCallback    = HAL_I2C_MemRxCpltCallback;    /* Legacy weak MemRxCpltCallback    */
	hi2c->ErrorCallback        = HAL_I2C_ErrorCallback;        /* Legacy weak ErrorCallback        */
	hi2c->AbortCpltCallback    = HAL_I2C_AbortCpltCallback;    /* Legacy weak AbortCpltCallback    */
	hi2c->AddrCallback         = HAL_I2C_AddrCallback;         /* Legacy weak AddrCallback         */

	if (hi2c->MspInitCallback == NULL)
	{
	  hi2c->MspInitCallback = HAL_I2C_MspInit; /* Legacy weak MspInit  */
	}

	/* Init the low level hardware : GPIO, CLOCK, CORTEX...etc */
	hi2c->MspInitCallback(hi2c);
	#else
	/* Init the low level hardware : GPIO, CLOCK, CORTEX...etc */
	this->HAL_I2C_MspInit(hi2c);
	#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
	}

	hi2c->State = HAL_I2C_STATE_BUSY;

	/* Disable the selected I2C peripheral */
	__HAL_I2C_DISABLE(hi2c);

	/*---------------------------- I2Cx TIMINGR Configuration ------------------*/
	/* Configure I2Cx: Frequency range */
	hi2c->Instance->TIMINGR = hi2c->Init.Timing & TIMING_CLEAR_MASK;

	/*---------------------------- I2Cx OAR1 Configuration ---------------------*/
	/* Disable Own Address1 before set the Own Address1 configuration */
	hi2c->Instance->OAR1 &= ~I2C_OAR1_OA1EN;

	/* Configure I2Cx: Own Address1 and ack own address1 mode */
	if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
	{
	hi2c->Instance->OAR1 = (I2C_OAR1_OA1EN | hi2c->Init.OwnAddress1);
	}
	else /* I2C_ADDRESSINGMODE_10BIT */
	{
	hi2c->Instance->OAR1 = (I2C_OAR1_OA1EN | I2C_OAR1_OA1MODE | hi2c->Init.OwnAddress1);
	}

	/*---------------------------- I2Cx CR2 Configuration ----------------------*/
	/* Configure I2Cx: Addressing Master mode */
	if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_10BIT)
	{
	hi2c->Instance->CR2 = (I2C_CR2_ADD10);
	}
	/* Enable the AUTOEND by default, and enable NACK (should be disable only during Slave process */
	hi2c->Instance->CR2 |= (I2C_CR2_AUTOEND | I2C_CR2_NACK);

	/*---------------------------- I2Cx OAR2 Configuration ---------------------*/
	/* Disable Own Address2 before set the Own Address2 configuration */
	hi2c->Instance->OAR2 &= ~I2C_DUALADDRESS_ENABLE;

	/* Configure I2Cx: Dual mode and Own Address2 */
	hi2c->Instance->OAR2 = (hi2c->Init.DualAddressMode | hi2c->Init.OwnAddress2 | (hi2c->Init.OwnAddress2Masks << 8));

	/*---------------------------- I2Cx CR1 Configuration ----------------------*/
	/* Configure I2Cx: Generalcall and NoStretch mode */
	hi2c->Instance->CR1 = (hi2c->Init.GeneralCallMode | hi2c->Init.NoStretchMode);

	/* Enable the selected I2C peripheral */
	__HAL_I2C_ENABLE(hi2c);

	hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
	hi2c->State = HAL_I2C_STATE_READY;
	hi2c->PreviousState = I2C_STATE_NONE;
	hi2c->Mode = HAL_I2C_MODE_NONE;

	return HAL_OK;
}

void I2CController::HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(hi2c->Instance==I2C1)
	{
		/* Peripheral clock enable */
		__HAL_RCC_I2C1_CLK_ENABLE();

		/* USER CODE BEGIN SysInit */
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		// Configure PA5 and PA6 as floating input because of double use on STM32 Nucleo board
		GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct = {0};

		/**I2C1 GPIO Configuration
		PA15     ------> I2C1_SCL
		PB7     ------> I2C1_SDA
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct = {0};

		GPIO_InitStruct.Pin = GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* I2C1 interrupt Init */
		HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
		HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

		/* I2C1 DMA Init */
		/* I2C1_TX Init */
		hdma_i2c1_tx.Instance = DMA2_Channel2;
		hdma_i2c1_tx.Init.Request = DMA_REQUEST_I2C1_TX;
		hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
		hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;
		if (HAL_DMA_Init(&hdma_i2c1_tx) != HAL_OK)
		{
		  Error_Handler();
		}

		__HAL_LINKDMA(hi2c,hdmatx,hdma_i2c1_tx);

		/* DMA interrupt init */
		HAL_NVIC_SetPriority(DMA2_Channel2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA2_Channel2_IRQn);

		/* I2C1_RX Init */
		hdma_i2c1_rx.Instance = DMA2_Channel3;
		hdma_i2c1_rx.Init.Request = DMA_REQUEST_I2C1_RX;
		hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
		hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_HIGH;
		if (HAL_DMA_Init(&hdma_i2c1_rx) != HAL_OK)
		{
		  Error_Handler();
		}

		__HAL_LINKDMA(hi2c,hdmarx,hdma_i2c1_rx);

		HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);

		// while( HAL_I2C_IsDeviceReady(&this->_hi2c1, this->_devAddressRead << 8, 5, 100) != HAL_OK );
	}
}

void I2CController::WriteDataEEPROM(SettingsFrame* data, uint8_t section)
{
	uint8_t* address = _aTxBuffer;

	memcpy(address, &data->amplitude, sizeof(data->amplitude));
	address += sizeof(data->amplitude);

	memcpy(address, &data->boundaries, sizeof(data->boundaries));
	address += sizeof(data->boundaries);

	memcpy(address, &data->filterOrders, sizeof(data->filterOrders));
	address += sizeof(data->filterOrders);

	memcpy(address, &data->controlFlag, sizeof(data->controlFlag));
	address += sizeof(data->controlFlag);

	memcpy(address, &data->factors, sizeof(data->factors));
	address += sizeof(data->factors);

	memcpy(address, &data->gammaStatus, sizeof(data->gammaStatus));
	address += sizeof(data->gammaStatus);

	memcpy(address, &data->ledStatus, sizeof(data->ledStatus));
	address += sizeof(data->ledStatus);

	uint16_t dataToWrite = sizeof(data->amplitude) + sizeof(data->boundaries) +
			sizeof(data->filterOrders) + sizeof(data->controlFlag) + sizeof(data->factors)
			+ sizeof(data->gammaStatus) + sizeof(data->ledStatus);

	TransmitData(section * EEPROM_SECTIONSIZE, dataToWrite);
	HAL_Delay(EEPROM_WRITE * 2);
}

void I2CController::ReadDataEEPROM(SettingsFrame* data, uint8_t section)
{
	uint16_t dataToRead = sizeof(data->amplitude) + sizeof(data->boundaries) +
			sizeof(data->filterOrders) + sizeof(data->controlFlag) + sizeof(data->factors)
			+ sizeof(data->gammaStatus) + sizeof(data->ledStatus);

	uint8_t* address = _aRxBuffer;

	ReceiveData(section * EEPROM_SECTIONSIZE, dataToRead);
	HAL_Delay(EEPROM_WRITE * 2);

	memcpy(&data->amplitude, address , sizeof(data->amplitude));
	address += sizeof(data->amplitude);

	memcpy(&data->boundaries, address, sizeof(data->boundaries));
	address += sizeof(data->boundaries);

	memcpy(&data->filterOrders, address, sizeof(data->filterOrders));
	address += sizeof(data->filterOrders);

	memcpy(&data->controlFlag, address,  sizeof(data->controlFlag));
	address += sizeof(data->controlFlag);

	memcpy(&data->factors, address, sizeof(data->factors));
	address += sizeof(data->factors);

	memcpy(&data->gammaStatus, address, sizeof(data->gammaStatus));
	address += sizeof(data->gammaStatus);

	memcpy(&data->ledStatus, address, sizeof(data->ledStatus));
	address += sizeof(data->ledStatus);
}

void I2CController::TransmitData(uint16_t address, uint16_t size)
{
	uint16_t counter = 0;
	HAL_StatusTypeDef result = HAL_OK;

	while (counter < size)
	{
		uint16_t diff = size - counter;

		HAL_I2C_ClearBusyFlagErrata_2_14_7(&_hi2c1);

		if (diff >= EEPROM_MAXPKT)
		{
//				HAL_I2C_Master_Seq_Transmit_DMA();

//			result = HAL_I2C_Master_Transmit_DMA(&_hi2c1, _devAddressWrite, &_aTxBuffer[counter], EEPROM_MAXPKT);

//				//Multi-Byte
			result = HAL_I2C_Mem_Write_DMA(&_hi2c1, _devAddressWrite,
					address + counter, _medAddSize,
					&_aTxBuffer[counter], EEPROM_MAXPKT);

			if(result != HAL_OK)
			{
				break;
			}

			while (HAL_I2C_GetState(&_hi2c1) != HAL_I2C_STATE_READY)
			{
			}

			counter += EEPROM_MAXPKT;
		}
		else
		{
			//and the remaining ones...low packet size
//			result = HAL_I2C_Master_Transmit_DMA(&_hi2c1, _devAddressWrite, &_aTxBuffer[counter], diff);

//			//Multi-Byte
			result = HAL_I2C_Mem_Write_DMA(&_hi2c1, _devAddressWrite,
					address + counter, _medAddSize,
					&_aTxBuffer[counter], diff);

			if(result != HAL_OK)
			{
				break;
			}

			while (HAL_I2C_GetState(&_hi2c1) != HAL_I2C_STATE_READY)
			{
			}

			counter += diff;
		}
	}
}

void I2CController::ReceiveData(uint16_t address, uint16_t size)
{
	uint16_t counter = 0;
	HAL_StatusTypeDef result = HAL_OK;

	while (counter < size)
	{
		uint16_t diff = size - counter;

		HAL_I2C_ClearBusyFlagErrata_2_14_7(&_hi2c1);

		if (diff >= EEPROM_MAXPKT)
		{
			//Multi-Byte
//			result = HAL_I2C_Master_Receive_DMA(&_hi2c1, _devAddressRead, &_aRxBuffer[counter], EEPROM_MAXPKT);

			result = HAL_I2C_Mem_Read_DMA(&_hi2c1, _devAddressRead,
					address + counter, _medAddSize,
					&_aRxBuffer[counter], EEPROM_MAXPKT);

			if(result != HAL_OK)
			{
				break;
			}

			while (HAL_I2C_GetState(&_hi2c1) != HAL_I2C_STATE_READY)
			{
			}

			counter += EEPROM_MAXPKT;
		}
		else
		{
//			result = HAL_I2C_Master_Receive_DMA(&_hi2c1, _devAddressRead, &_aRxBuffer[counter], diff);

			//Multi-Byte
			result = HAL_I2C_Mem_Read_DMA(&_hi2c1, _devAddressRead,
					address + counter, _medAddSize,
					&_aRxBuffer[counter], diff);

			if(result != HAL_OK)
			{
				break;
			}

			while (HAL_I2C_GetState(&_hi2c1) != HAL_I2C_STATE_READY)
			{
			}

			counter += diff;
		}
	}
}

void I2CController::HAL_I2C_ClearBusyFlagErrata_2_14_7(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1
    __HAL_I2C_DISABLE(hi2c);

    // 2
    GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);

    // 3
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_RESET) {
        for(;;){}
    }
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_RESET) {
        for(;;){}
    }

    // 4
    GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);

    // 5
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET) {
        for(;;){}
    }

    // 6
	GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);

    // 7
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_SET) {
        for(;;){}
    }

    // 8
	GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

    // 9
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_RESET) {
        for(;;){}
    }

    // 10
	GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);

    // 11
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_RESET) {
        for(;;){}
    }

    // 12
	GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   // 13
   hi2c->Instance->CR1 |= I2C_CR1_SWRST;

   // 14
   hi2c->Instance->CR1 ^= I2C_CR1_SWRST;

   // 15
   __HAL_I2C_ENABLE(hi2c);
}

void I2CController::TxCompleteHandler(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance==_hi2c1.Instance)
	{
		__HAL_I2C_GENERATE_NACK(&this->_hi2c1);
	}
}

void I2CController::Error_Handler()
{
	while(1)
	{

	}
}

void I2CController::EvHandler()
{
	HAL_I2C_EV_IRQHandler(&_hi2c1);
}

void I2CController::ErHandler()
{
	HAL_I2C_ER_IRQHandler(&_hi2c1);
}

void I2CController::RxHandler()
{
	HAL_DMA_IRQHandler(&hdma_i2c1_rx);
}

void I2CController::TxHandler()
{
	HAL_DMA_IRQHandler(&hdma_i2c1_tx);
}

I2CController::~I2CController()
{
	// TODO Auto-generated destructor stub
}

