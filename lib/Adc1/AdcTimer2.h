/*
 * AdcTimer2.h
 *
 *  Created on: 6 Oct 2020
 *      Author: Christoph
 */

#ifndef INC_ADCTIMER2_H_
#define INC_ADCTIMER2_H_

#include "stm32g4xx_hal.h"
#include "main.h"

class AdcTimer2
{
public:
	AdcTimer2();
	virtual ~AdcTimer2();
	void Init();
	void InterruptHandler();

private:
	TIM_HandleTypeDef 			_timHandleAdcDma;
	uint32_t 					_sampleFrequency = SAMPLE_FREQ;
	uint8_t						_enableTimerInterrupt = RESET;
	HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim);
	void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base);
	void Error_Handler();

};

#endif /* INC_ADCTIMER2_H_ */
