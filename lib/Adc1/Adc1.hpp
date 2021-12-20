/*
 * Adc1.h
 *
 *  Created on: Oct 1, 2020
 *      Author: chris
 */

#ifndef INC_ADC1_H_
#define INC_ADC1_H_

#include "stm32g4xx_hal.h"
#include <main.hpp>
#include <AdcTimer2.hpp>
#include <stdio.h>
#include <string.h>
using namespace std;

#define ADC_CFGR_FIELDS_1  ((ADC_CFGR_RES    | ADC_CFGR_ALIGN   |\
                             ADC_CFGR_CONT   | ADC_CFGR_OVRMOD  |\
                             ADC_CFGR_DISCEN | ADC_CFGR_DISCNUM |\
                             ADC_CFGR_EXTEN  | ADC_CFGR_EXTSEL))   /*!< ADC_CFGR fields of parameters that can be updated
                                                                        when no regular conversion is on-going */

class Adc1
{
private:
	static Adc1 								_adc1Instance;
	uint16_t 									_adcConvertedData[2 * FFT_SAMPLE_COUNT];
	ADC_HandleTypeDef  							_adcHandle;
	ADC_ChannelConfTypeDef   					_sConfig;
	DMA_HandleTypeDef 							_dmaHandle;
	uint32_t 									_adcChannel = ADC_CHANNEL_1;
	AdcTimer2									_adcTimer2;
	__IO uint8_t								_adcConversionFinished = RESET;
	__IO uint8_t								_adcConversionHalfFinished = RESET;
	uint32_t									_sampleFrequency = SAMPLE_FREQ;

	void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);
	HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef *hadc);
	void Error_Handler(void);
	void MspInit(void);

public:

	Adc1();
	virtual ~Adc1();
	static Adc1* GetInstance() {return &_adc1Instance; }
	void Init();
	void AdcConvCompleteHandler(void);
	void AdcConvHalfCompleteHandler(void);
	void DmaInterruptHandler(void);
	void TimInterruptHandler(void);
	uint16_t* GetAdcValues(void);
	uint8_t IsConversionComplete(void);
	uint8_t	IsConversionHalfComplete(void);
	uint32_t GetSampleFrequency(void);
};


#endif /* INC_ADC1_H_ */
