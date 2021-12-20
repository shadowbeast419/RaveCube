/*
 * LedControllerTimer3.h
 *
 *  Created on: Oct 8, 2020
 *      Author: Christoph
 */

#ifndef INC_PWMGENERATOR_H_
#define INC_PWMGENERATOR_H_

#include "stm32g4xx_hal.h"
#include <RgbLedBrightness.hpp>

class PwmGenerator
{
private:
	void Error_Handler();

protected:
	static const uint32_t					_pwmFrequency = 800000;
	static const uint32_t					_dutyCycleLow = 32;
	static const uint32_t					_dutyCycleHigh = 64;
	static const uint32_t 					_ledCount = 142;
	static const uint32_t					_resetOffset = 3;
	RgbLedBrightness 						_led_matrix[_ledCount + _resetOffset];
	static const uint32_t					_pwmDmaBufferSize = 48;
	uint32_t 								_aCCValueBuffer[_pwmDmaBufferSize];
	uint32_t 								_uwTimerPeriod;
	uint32_t 								_uwCCValueLow;
	uint32_t 								_uwCCValueHigh;
	TIM_TypeDef* 							_timerInstance = TIM3;
	const uint32_t							_timerChannel = TIM_CHANNEL_3;
	TIM_HandleTypeDef						_timHandlePwmDma;

	HAL_StatusTypeDef HAL_TIM_PWM_Init(TIM_HandleTypeDef *htim);
	void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim);
	void MspInit(TIM_HandleTypeDef *htim);
	void InitPwm();
	void EnablePwmOutput();
	void DisablePwmOutput();

public:
	PwmGenerator();
	virtual ~PwmGenerator();
	void DmaInterruptHandler();
};

#endif /* INC_PWMGENERATOR_H_ */
