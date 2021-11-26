/*
 * FFT.h
 *
 *  Created on: 2 Oct 2020
 *      Author: Christoph
 */

#ifndef INC_FFT_H_
#define INC_FFT_H_

#include "main.h"
#include <arm_math.h>
#include <VoltageSignal.h>
#include <stdio.h>

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

	const uint16_t					ScalingMulitplicator = 15;

private:
	uint32_t 						_sampleFrequency;
	VoltageSignal* 					_voltageSignal;
	FFT_Result 						_fftResults[MAX_FFT_RESULT_INDEX];
	// float32_t 						dest[FFT_SAMPLE_COUNT / 2];
	arm_rfft_instance_q15 			_fftInstance;
	q15_t							_fftValuesBuffer[2 * FFT_SAMPLE_COUNT];
};



#endif /* INC_FFT_H_ */
