/*
 * UartController.h
 *
 *  Created on: Oct 6, 2020
 *      Author: Christoph
 */

#ifndef INC_UARTCONTROLLER_H_
#define INC_UARTCONTROLLER_H_

#include "stm32g4xx_hal.h"
#include <cstring>

#define RX_BUFFER_LENGTH ((uint16_t)256)
#define TX_BUFFER_LENGTH ((uint16_t)256)

class UartController
{
public:
	UartController();
	virtual ~UartController();
	void Init();
	void UartInterruptHandler();
	void Dma1Channel2Handler();
	void Dma1Channel3Handler();
	void Transmit(uint8_t* data);
	uint8_t IsNewRxDataAvailable();
	uint8_t* GetRxMessage();
	void TransmitCpltHandler();
	uint8_t IsTxBusy();

private:
	UART_HandleTypeDef 					huart2;
	DMA_HandleTypeDef 					hdma_usart2_tx;
	DMA_HandleTypeDef 					hdma_usart2_rx;
	uint8_t 							_newDataReceived = RESET;
	uint8_t								_txBuffer[TX_BUFFER_LENGTH];
	uint8_t 							_rxBuffer[RX_BUFFER_LENGTH];
	uint8_t								_rxStrContent[RX_BUFFER_LENGTH];
	uint8_t 							_prevRxBufferEndPos = 0;
	uint8_t								_prevCounterValue = RX_BUFFER_LENGTH;
	uint8_t								_transmitComplete = SET;

	HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);
	void HAL_UART_MspInit(UART_HandleTypeDef* huart);
	void RxIdleHandler();
	void Error_Handler();
};

#endif /* INC_UARTCONTROLLER_H_ */
