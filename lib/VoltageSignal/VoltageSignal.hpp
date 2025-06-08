/*
 * VoltageSignal.h
 *
 *  Created on: 2 Oct 2020
 *      Author: Christoph
 */

#ifndef INC_VOLTAGESIGNAL_H_
#define INC_VOLTAGESIGNAL_H_


#include <arm_math.h>
#include "stm32g4xx_hal.h"
#include <main.hpp>

class VoltageSignal
{
public:

	VoltageSignal();
	virtual ~VoltageSignal();
	void UpdateAdcValues(uint16_t* adcValues);
	float32_t* GetSignal();
	float32_t GetRMSValue();
	float32_t GetMeanValue();

private:
	/// @brief Apply scaling factor for better accuracy in FFT (floats)
	const float32_t VoltageScalingFactor = 1.0f;

	float32_t 	_voltageValues[FFT_SAMPLE_COUNT];
	float32_t 	_rmsValue = 0.0f;
	float32_t 	_meanValue = 0.0f;

#ifdef ENABLE_WINDOWING
	float32_t	_windowWeights[FFT_SAMPLE_COUNT];

#endif

};

#endif /* INC_VOLTAGESIGNAL_H_ */
