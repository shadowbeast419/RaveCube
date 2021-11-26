/*
 * VoltageSignal.cpp
 *
 *  Created on: 2 Oct 2020
 *      Author: Christoph
 */

#include <VoltageSignal.h>

VoltageSignal::VoltageSignal()
{
#ifdef ENABLE_WINDOWING

	float32_t cosValue = 0.0f;

	for(uint32_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		cosValue = 2.0f * (PI) * (i / (float32_t)(FFT_SAMPLE_COUNT - 1));

		// Blackman Harris window
		_windowWeights[i] = 0.35875f + 0.48829f * arm_cos_f32(cosValue) + 0.14128f *
				arm_cos_f32(cosValue * 2.0f) + 0.01168f * arm_cos_f32(cosValue * 3.0f);

		// Hanning window
		//_windowWeights[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * (PI) * (i / (float)(FFT_SAMPLE_COUNT - 1))));
	}

#endif
}

VoltageSignal::~VoltageSignal()
{

}

void VoltageSignal::UpdateAdcValues(uint16_t* adcValues)
{
	uint32_t squaredSum = 0.0f;
	int32_t sum = 0.0f;
	int16_t average = 0;

	for(uint32_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		// Apply scaling factor for better accuracy in FFT
		_voltageValues[i] = (int16_t)((float32_t)adcValues[i] * (2971.0f * VOLTAGE_SCALING_FACTOR  / (float32_t)0xFFF));
		sum += _voltageValues[i];
	}

	average = sum / FFT_SAMPLE_COUNT;
	_offsetVoltage = average;

	// Remove DC part and calculate peak voltage
	for(uint32_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		_voltageValues[i] -= average;
		squaredSum += (_voltageValues[i] * _voltageValues[i]);
	}

	squaredSum /= FFT_SAMPLE_COUNT;
	_rmsValue = (uint16_t)sqrtf(squaredSum);

}

float32_t VoltageSignal::GetRMSValue()
{
	return _rmsValue / VOLTAGE_SCALING_FACTOR;
}

int16_t* VoltageSignal::GetSignal()
{
#ifdef ENABLE_WINDOWING
	for(uint16_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		_voltageValues[i] = (int16_t)(_voltageValues[i] * _windowWeights[i]);
	}
#endif

	return this->_voltageValues;
}
