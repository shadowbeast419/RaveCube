/*
 * I2C.h
 *
 *  Created on: 21 Mar 2021
 *      Author: Christoph
 */

#ifndef INC_I2CCONTROLLER_H_
#define INC_I2CCONTROLLER_H_

#include "stm32g4xx_hal.h"
#include "main.h"
#include <SettingsController.h>
#include <string.h>
#include <SettingsStructs.h>

#define TX_BUFFER_SIZE 128
#define RX_BUFFER_SIZE 128

#define TIMING_CLEAR_MASK   (0xF0FFFFFFU)  /*!< I2C TIMING clear register Mask */
#define I2C_STATE_NONE      ((uint32_t)(HAL_I2C_MODE_NONE))

#define EEPROM_MAXPKT             	64              	//(page size)
#define EEPROM_WRITE              	10              	//time to wait in ms
#define EEPROM_SECTIONSIZE			64

class I2CController
{
private:
	static I2CController			_i2cInstance;
	I2C_HandleTypeDef 				_hi2c1;
	DMA_HandleTypeDef 				hdma_i2c1_tx;
	DMA_HandleTypeDef 				hdma_i2c1_rx;
	const uint16_t					_slaveAddress = 0x0000;
	uint8_t							_aTxBuffer[TX_BUFFER_SIZE];
	uint8_t							_aRxBuffer[RX_BUFFER_SIZE];
	const uint16_t					_devAddressWrite = 0xA0;
	const uint16_t					_devAddressRead = 0xA0;
	const uint16_t					_medAddSize = I2C_MEMADD_SIZE_16BIT;

	void Error_Handler();
	void TransmitData(uint16_t address, uint16_t size);
	void ReceiveData(uint16_t address, uint16_t size);
	void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
	HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
	void HAL_I2C_ClearBusyFlagErrata_2_14_7(I2C_HandleTypeDef *hi2c);

public:
	I2CController();
	virtual ~I2CController();

	static I2CController* GetInstance() {return &_i2cInstance; }
	void Init();
	void EvHandler();	// i2c event interrupt
	void ErHandler();	// i2c error interrupt
	void RxHandler();
	void TxHandler();
	void TxCompleteHandler(I2C_HandleTypeDef *hi2c);

	void WriteDataEEPROM(SettingsFrame* data, uint8_t section);
	void ReadDataEEPROM(SettingsFrame* data, uint8_t section);
};

#endif /* INC_I2CCONTROLLER_H_ */
