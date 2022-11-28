/*
 * FFT.cpp
 *
 *  Created on: 2 Oct 2020
 *      Author: Christoph
 */

#include <FFT.hpp>

FFT::FFT()
{
	arm_rfft_fast_init_f32(&_rfftFastInstance, FFT_SAMPLE_COUNT);
}

void FFT::UpdateVoltageSignal(VoltageSignal* voltageSignal, uint32_t sampleFrequency)
{
	_sampleFrequency = sampleFrequency;
	_voltageSignal = voltageSignal;
}


// https://community.arm.com/support-forums/f/armds-forum/6243/problem-with-arm_rfft_fast_f32
FFT_Result* FFT::CalculateFFT()
{
	// Take the absolute value of the Nyquist frequency data and save it to variable
	uint32_t fftBinCount = FFT_SAMPLE_COUNT / 2;
	float32_t* signalArray = _voltageSignal->GetSignal();
	float32_t maxValue;
	uint32_t maxIndex;
	uint16_t hzPerSample = HZ_PER_SAMPLE;

	arm_rfft_fast_f32(&_rfftFastInstance, signalArray, _spectrumBuffer, 0);
	arm_cmplx_mag_f32(_spectrumBuffer, _absSpectrumBuffer, fftBinCount);
	arm_max_f32(_absSpectrumBuffer, fftBinCount, &maxValue, &maxIndex);

	for(uint16_t i = 0; i < fftBinCount; i++)
	{
		// For better value resolution, skip the real "absolute" values of the FFT
		// _fftResults[i].absoluteValue = absSpectrumBuffer[i] / FFT_SAMPLE_COUNT;
		_fftResults[i].absoluteValue = _absSpectrumBuffer[i];
		_fftResults[i].frequency = i * hzPerSample;
	}

/* 	for(uint16_t i = 0; i< FFT_SAMPLE_COUNT; i++)
	{
		// Convert
		signalArrayFixed[i] = (int16_t)signalArray[i];
	}

	for(uint16_t i = 0; i < FFT_SAMPLE_COUNT; i++)
	{
		// Convert voltage signal to q15
		signalBuffer = (float32_t)signalArray[i] / (MAX_ADC_AC_INPUT_VOLTAGE * VOLTAGE_SCALING_FACTOR);
		signalArray[i] = (int16_t)(signalBuffer * 32768.0f);
	}

	arm_rfft_q15(&_fftInstance, signalArrayFixed, _fftValuesBuffer);

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
	} */

	return _fftResults;
}

FFT::~FFT()
{

}
