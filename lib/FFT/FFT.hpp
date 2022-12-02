/*
 * FFT.h
 *
 *  Created on: 2 Oct 2020
 *      Author: Christoph
 */

#ifndef INC_FFT_H_
#define INC_FFT_H_

#include <arm_math.h>
#include <stdio.h>
#include <VoltageSignal.hpp>

struct FFT_Result
{
	uint16_t 	frequency;
	float32_t 	absoluteValue;
};

class FFT
{
public:
	FFT();
	virtual ~FFT();
	void UpdateVoltageSignal(VoltageSignal* voltageSignal, uint32_t sampleFrequency);
	FFT_Result* CalculateFFT();
	float GetMaxFreqAmplitude();

private:
	uint32_t 						_sampleFrequency;
	VoltageSignal* 					_voltageSignal;
	arm_rfft_fast_instance_f32		_rfftFastInstance;
	FFT_Result 						_fftResults[FFT_SAMPLE_COUNT / 2];
	float32_t 						_spectrumBuffer[FFT_SAMPLE_COUNT];
	float32_t 						_absSpectrumBuffer[FFT_SAMPLE_COUNT];
};



#endif /* INC_FFT_H_ */
