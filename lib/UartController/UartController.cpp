/*
 * UartController.cpp
 *
 *  Created on: Oct 6, 2020
 *      Author: Christoph
 */

#include <UartController.hpp>

UartController::UartController()
{
	// TODO Auto-generated constructor stub
	memset(_rxStrContent, 0, RX_BUFFER_LENGTH);
}

UartController::~UartController()
{
	// TODO Auto-generated destructor stub
}

void UartController::Transmit(uint8_t* data)
{
	HAL_StatusTypeDef status = HAL_BUSY;

	uint16_t dataLength = (uint16_t)strlen((char*)data);

	memcpy((uint8_t*)_txBuffer, (uint8_t*)data, dataLength);

	// do
	// {
	// 	status = HAL_UART_GetState(&huart2);

	// } while (status != HAL_UART_STATE_READY);

	while(_transmitComplete == RESET);

	status = HAL_UART_Transmit_DMA(&huart2, (uint8_t*)_txBuffer, dataLength);

	/*##-1- Start the transmission process #####################################*/
	/* User start transmission data through "TxBuffer" buffer */
	if(status != HAL_OK)
	{
	/* Transfer error in transmission process */
	Error_Handler();
	}

	_transmitComplete = RESET;
}

void UartController::Init()
{
	/* DMA1_Channel1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
	/* DMA1_Channel2_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
	Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
	Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
	{
	Error_Handler();
	}

	if (HAL_UART_Receive_DMA(&huart2, (uint8_t *)_rxBuffer, RX_BUFFER_LENGTH) != HAL_OK)
	{
	/* Transfer error in reception process */
	Error_Handler();
	}

	__HAL_UART_DISABLE_IT(&huart2, UART_IT_ERR);
	__HAL_UART_DISABLE_IT(&huart2, UART_IT_PE);
}

void UartController::HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_USART2_CLK_ENABLE();

	/**USART2 GPIO Configuration
	PA2     ------> USART2_TX
	PA3     ------> USART2_RX
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Channel2;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_USART2_RX;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Channel3;
    hdma_usart2_tx.Init.Request = DMA_REQUEST_USART2_TX;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);

    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

HAL_StatusTypeDef UartController::HAL_UART_Init(UART_HandleTypeDef *huart)
{
	/* Check the UART handle allocation */
	if (huart == NULL)
	{
	return HAL_ERROR;
	}

	if (huart->Init.HwFlowCtl != UART_HWCONTROL_NONE)
	{
	/* Check the parameters */
	assert_param(IS_UART_HWFLOW_INSTANCE(huart->Instance));
	}
	else
	{
	/* Check the parameters */
	assert_param((IS_UART_INSTANCE(huart->Instance)) || (IS_LPUART_INSTANCE(huart->Instance)));
	}

	if (huart->gState == HAL_UART_STATE_RESET)
	{
	/* Allocate lock resource and initialize it */
	huart->Lock = HAL_UNLOCKED;

	#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
	UART_InitCallbacksToDefault(huart);

	if (huart->MspInitCallback == NULL)
	{
	  huart->MspInitCallback = HAL_UART_MspInit;
	}

	/* Init the low level hardware */
	huart->MspInitCallback(huart);
	#else
	/* Init the low level hardware : GPIO, CLOCK */
	HAL_UART_MspInit(huart);
	#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
	}

	huart->gState = HAL_UART_STATE_BUSY;

	__HAL_UART_DISABLE(huart);

	/* Set the UART Communication parameters */
	if (UART_SetConfig(huart) == HAL_ERROR)
	{
	return HAL_ERROR;
	}

	if (huart->AdvancedInit.AdvFeatureInit != UART_ADVFEATURE_NO_INIT)
	{
	UART_AdvFeatureConfig(huart);
	}

	/* In asynchronous mode, the following bits must be kept cleared:
	- LINEN and CLKEN bits in the USART_CR2 register,
	- SCEN, HDSEL and IREN  bits in the USART_CR3 register.*/
	CLEAR_BIT(huart->Instance->CR2, (USART_CR2_LINEN | USART_CR2_CLKEN));
	CLEAR_BIT(huart->Instance->CR3, (USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN));

	__HAL_UART_ENABLE(huart);

	/* TEACK and/or REACK to check before moving huart->gState and huart->RxState to Ready */
	return (UART_CheckIdleState(huart));
}

void UartController::UartInterruptHandler()
{
	HAL_UART_IRQHandler(&huart2);

    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET)   // Judging whether it is idle interruption
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);                     // Clear idle interrupt sign (otherwise it will continue to enter interrupt)
        RxIdleHandler();
    }
}

void UartController::RxIdleHandler()
{
    // Calculate the length of the received data, counter will be reseted after each receiving!
    uint8_t dmaCounterValue = __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
    uint8_t dataCountToCopy = 0;

    // Buffer overflow
    if(dmaCounterValue > _prevCounterValue)
    {
    	// Copy the part of data which is at the end of the buffer first
    	uint8_t firstPartOffset = RX_BUFFER_LENGTH - _prevCounterValue;
    	uint8_t firstPartDataLength = RX_BUFFER_LENGTH - firstPartOffset;
    	memcpy(_rxStrContent, _rxBuffer + firstPartOffset, firstPartDataLength);

    	uint8_t secondPartDataLength = RX_BUFFER_LENGTH - dmaCounterValue;
    	memcpy(_rxStrContent + firstPartDataLength, _rxBuffer, secondPartDataLength);

    	dataCountToCopy = firstPartDataLength + secondPartDataLength;
    	_prevRxBufferEndPos = secondPartDataLength;
    }
    // Normal mode
    else
    {
        uint8_t dataOffset = _prevRxBufferEndPos;
        uint8_t dataLength = RX_BUFFER_LENGTH - dmaCounterValue - dataOffset;

    	// At initialization the IdleInterrupt is called, skip that
    	// (counter is at max because no data is available)
    	if(dataLength != 0)
    	{
        	memcpy(_rxStrContent, _rxBuffer + dataOffset, dataLength);

        	_prevRxBufferEndPos = dataLength + dataOffset;
        	dataCountToCopy = dataLength;
    	}
    }

    if(dataCountToCopy > 0)
    {
    	_rxStrContent[dataCountToCopy] = '\0';
    	_newDataReceived = SET;
    	_prevCounterValue = dmaCounterValue;
    }
}

uint8_t UartController::IsNewRxDataAvailable()
{
	return _newDataReceived;
}

uint8_t* UartController::GetRxMessage()
{
	if(_newDataReceived == SET)
	{
		_newDataReceived = RESET;
		return _rxStrContent;
	}

	return (uint8_t*)"No data available";
}

void UartController::Dma1Channel2Handler()
{
	HAL_DMA_IRQHandler(&hdma_usart2_rx);
}

void UartController::Dma1Channel3Handler()
{
	HAL_DMA_IRQHandler(&hdma_usart2_tx);
}

void UartController::Error_Handler()
{
	while(1)
	{

	}
}

void UartController::TransmitCpltHandler()
{
	_transmitComplete = SET;
}

uint8_t UartController::IsTxBusy()
{
	return !_transmitComplete;
}
