/*
 * RgbLedBrightness.h
 *
 *  Created on: 21 Oct 2020
 *      Author: Christoph
 */

#ifndef INC_RGBLEDBRIGHTNESS_H_
#define INC_RGBLEDBRIGHTNESS_H_

#include "stm32g4xx_hal.h"
#include <main.hpp>
#include <arm_math.h>
#include <stdio.h>

struct RgbLedBrightness
{
	uint16_t Red;
	uint16_t Green;
	uint16_t Blue;
	uint8_t Reset;
};

struct HSVBrightness
{
	uint16_t Angle;
	float32_t Saturation;
	float32_t Brightness;
};

#endif /* INC_RGBLEDBRIGHTNESS_H_ */
