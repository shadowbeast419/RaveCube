/*
 * SettingsStruct.h
 *
 *  Created on: Apr 11, 2021
 *      Author: Christoph
 */

#ifndef INC_SETTINGSSTRUCTS_H_
#define INC_SETTINGSSTRUCTS_H_

#include "stm32g4xx_hal.h"
#include <arm_math.h>
#include <MovingAvgFilter.h>

struct FrequencyBoundary
{
	uint16_t Min;
	uint16_t Max;
};

struct FrequencyColorBoundaries
{
	FrequencyBoundary Red;
	FrequencyBoundary Green;
	FrequencyBoundary Blue;
};

struct BrightnessFactors
{
	float32_t Red;
	float32_t Green;
	float32_t Blue;
};

struct SettingsFrame
{
	int8_t controlFlag;
	uint8_t ledStatus;
	uint8_t gammaStatus;
	float32_t amplitude;
	BrightnessFactors factors;
	FrequencyColorBoundaries boundaries;
	MovingAvgFilterOrder filterOrders;
};

#endif /* INC_SETTINGSSTRUCT_H_ */
