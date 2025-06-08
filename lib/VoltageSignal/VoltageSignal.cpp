/*
 * VoltageSignal.cpp
 *
 *  Created on: 2 Oct 2020
 *      Author: Christoph
 */

#include <VoltageSignal.hpp>

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
	float32_t sum = 0.0f;
	float32_t absValue = 0.0f;
	float32_t sumSquared = 0.0f;
	float32_t rmsValue = 0.0f;
	float32_t sqrRootResult = 0.0f;

	for(uint32_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		_voltageValues[i] = ((float32_t)adcValues[i] * (2971.0f * VoltageScalingFactor  / (float32_t)0xFFF)); // - 1485.0f;
		sum += _voltageValues[i];
	}

	_meanValue = sum / (float32_t)FFT_SAMPLE_COUNT;
	_rmsValue = _meanValue;

	for(uint32_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		// Remove DC part
		_voltageValues[i] -= _meanValue;
		
		absValue = fabs(_voltageValues[i]);
		sumSquared +=  absValue * absValue;
	}

	arm_sqrt_f32(sumSquared / (float32_t)FFT_SAMPLE_COUNT, &sqrRootResult);

	// https://musicinformationretrieval.com/energy.html
	_rmsValue = sqrRootResult;
	_meanValue = sqrRootResult;
}

float32_t VoltageSignal::GetRMSValue()
{
	return _rmsValue;
}

float32_t VoltageSignal::GetMeanValue()
{
	return  _meanValue;
}

float32_t* VoltageSignal::GetSignal()
{
#ifdef ENABLE_WINDOWING
	for(uint16_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		_voltageValues[i] = _voltageValues[i] * _windowWeights[i];
	}
#endif

	return this->_voltageValues;
}
