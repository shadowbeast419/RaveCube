/*
 * FFT.cpp
 *
 *  Created on: 2 Oct 2020
 *      Author: Christoph
 */

#include <FFT.h>

FFT::FFT()
{
	// Bit not reversed -> "Normal" Output
	arm_status status = arm_rfft_init_q15(&_fftInstance, FFT_SAMPLE_COUNT, 0, 1);

	if(status != ARM_MATH_SUCCESS)
	{
		while(1)
		{
			// Error
		}
	}
}

void FFT::UpdateVoltageSignal(VoltageSignal* voltageSignal, uint32_t sampleFrequency)
{
	_sampleFrequency = sampleFrequency;
	_voltageSignal = voltageSignal;
}

FFT_Result* FFT::CalculateFFT()
{
	FFT_Result result;
	int16_t* signalArray;
	float32_t signalBuffer = 0;

	signalArray = _voltageSignal->GetSignal();

	for(uint16_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		// Convert voltage signal to q15
		signalBuffer = (float32_t)signalArray[i] / (MAX_ADC_AC_INPUT_VOLTAGE * VOLTAGE_SCALING_FACTOR);
		signalArray[i] = (int16_t)(signalBuffer * 32768.0f);
	}

	arm_rfft_q15(&_fftInstance, signalArray, _fftValuesBuffer);

	float32_t magnitudeBuffer = 0.0f;
	uint16_t complexPairCounter = 0;
	float32_t realValueBuffer, imagValueBuffer;

	for(uint16_t i = 0; i < 2 * (FFT_SAMPLE_COUNT - 1); i = i+2)
	{
		if(complexPairCounter == MAX_FFT_RESULT_INDEX)
			break;

		// Convert back to float numbers
		realValueBuffer = (float32_t)(_fftValuesBuffer[i] / 32768.0f);
		imagValueBuffer = (float32_t)(_fftValuesBuffer[i + 1] / 32768.0f);

		realValueBuffer = realValueBuffer * (MAX_ADC_AC_INPUT_VOLTAGE * VOLTAGE_SCALING_FACTOR);
		imagValueBuffer = imagValueBuffer * (MAX_ADC_AC_INPUT_VOLTAGE * VOLTAGE_SCALING_FACTOR);

		magnitudeBuffer = sqrtf(realValueBuffer * realValueBuffer + imagValueBuffer * imagValueBuffer);

		result.absoluteValue = magnitudeBuffer / FFT_SAMPLE_COUNT;
		result.frequency = complexPairCounter * HZ_PER_SAMPLE;

		_fftResults[complexPairCounter] = result;

		complexPairCounter++;
	}

	return _fftResults;
}

FFT::~FFT()
{

}
