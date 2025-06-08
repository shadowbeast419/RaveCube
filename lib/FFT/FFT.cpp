/*
 * FFT.cpp
 *
 *  Created on: 2 Oct 2020
 *      Author: Christoph
 */

#include <FFT.hpp>

FFT::FFT()
{
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
	uint16_t hzPerSample = HZ_PER_SAMPLE;

	arm_rfft_fast_init_256_f32(&_rfftFastInstance);
	arm_rfft_fast_f32(&_rfftFastInstance, signalArray, _spectrumBuffer, 0);
	arm_cmplx_mag_f32(_spectrumBuffer, _absSpectrumBuffer, fftBinCount);

	for(uint16_t i = 0; i < fftBinCount; i++)
	{
		// For better value resolution, skip the real "absolute" values of the FFT
		// _fftResults[i].absoluteValue = absSpectrumBuffer[i] / FFT_SAMPLE_COUNT;
		_fftResults[i].absoluteValue = _absSpectrumBuffer[i];
		_fftResults[i].frequency = i * hzPerSample;
	}
	return _fftResults;
}

FFT::~FFT()
{

}
