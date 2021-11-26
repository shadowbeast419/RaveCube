/*
 * VoltagePeakTimer.h
 *
 *  Created on: 6 May 2021
 *      Author: Christoph
 */

#ifndef INC_LARGEINTERVALTIMER_H_
#define INC_LARGEINTERVALTIMER_H_

#include "stm32g4xx_hal.h"
#include <arm_math.h>
#include <stdio.h>

class LargeIntervalTimer
{
private:
	TIM_HandleTypeDef 			_timHandle;
	float32_t 					_intervalSeconds = 0.5f;
	HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim);
	void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base);

public:
	TIM_TypeDef*				instance = TIM4;

	LargeIntervalTimer();
	LargeIntervalTimer(TIM_TypeDef* instance, float32_t intervalSeconds);
	virtual ~LargeIntervalTimer();
	void Init();
	void InterruptHandler();
};

#endif /* INC_LARGEINTERVALTIMER_H_ */
