/*
 * RGBLedController.cpp
 *
 *  Created on: 4 Oct 2020
 *      Author: Christoph
 */

#include <LedController.hpp>

LedController LedController::_rgbInstance;

LedController* LedController::GetInstance()
{
	return &_rgbInstance;
}

LedController::LedController()
{

}

void LedController::Init(MovingAvgFilter* movingAvgFilter)
{
	_peakTimer.Init();
	_movAvgFilter = movingAvgFilter;

	this->InitLedMatrix();
	this->InitPwm();
}
void LedController::InitLedMatrix()
{
	// Initialize the first 3 LEDs as reset pulse
	for(uint32_t i = 0; i < _resetOffset; i++)
	{
		LedController::_led_matrix[i].Reset = 1;
	}

	uint32_t length = _ledCount + _resetOffset;

	for(uint32_t i = _resetOffset; i < length; i++)
	{
		_led_matrix[i].Red = 0x00;
		_led_matrix[i].Green = 0x00;
		_led_matrix[i].Blue = 0x00;
		_led_matrix[i].Reset = 0;
	}

	UpdateLEDColor();
	UpdatePwmSettingsBuffer(_currentLedIndex++, SET);
}

// 24bit or 48bit written
void LedController::PulseFinishedHandler(TIM_HandleTypeDef *htim, uint8_t isFullyCpltFlag)
{
	// Update the LED PWM values for each LED
	if((htim->Instance == _timHandlePwmDma.Instance) && (_dmaBufferNeedsUpdate == SET))
	{
		// If Half Cplt, LED number is even
		UpdatePwmSettingsBuffer(_currentLedIndex++, !isFullyCpltFlag);

		if(_currentLedIndex == (_ledCount + _resetOffset))
		{
			_currentLedIndex = 0;
			_dmaBufferNeedsUpdate = RESET;
		}
	}
}

void LedController::PeakTimerInterruptHandler()
{
	this->_peakTimer.InterruptHandler();
}

void LedController::PeakTimerPeriodElapsedHandler(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == _peakTimer.instance)
	{
		if(_peakRmsVoltage > 15.0f)
		{
			_movAvgFilter->AddPeakVoltageValue(_peakRmsVoltage);
		}

		_peakRmsVoltage = 0.0f;
	}
}

void LedController::UpdatePwmSettingsBuffer(uint32_t ledIndexTmp, uint8_t ledIsOdd)
{
	uint8_t startIndex = 0;

	if(ledIsOdd == SET)
	{
		startIndex = 0;
	}
	else
	{
		startIndex = _pwmDmaBufferSize / 2;
	}

	// Send 24 Reset Pulses, because of code structure and convenience ;)
	if(_led_matrix[ledIndexTmp].Reset == SET)
	{
		uint16_t maxIndex = startIndex + (_pwmDmaBufferSize / 2);

		for(uint16_t i = startIndex; i < maxIndex; i++)
		{
			_aCCValueBuffer[i] = 0;
		}

		return;
	}

	// First Color
	for(uint8_t i = 0; i < 8; i ++)
	{
		// Set the duty cycle buffer according to the color bits
		if(_led_matrix[ledIndexTmp].Green & (0x80 >> i))
			_aCCValueBuffer[startIndex + i] = _uwCCValueHigh;
		else
			_aCCValueBuffer[startIndex + i] = _uwCCValueLow;
	}

	startIndex += 8;

	// Second Color
	for(uint8_t i = 0; i < 8; i ++)
	{
		// Set the duty cycle buffer according to the color bits
		if(_led_matrix[ledIndexTmp].Red & (0x80 >> i))
			_aCCValueBuffer[startIndex + i] = _uwCCValueHigh;
		else
			_aCCValueBuffer[startIndex + i] = _uwCCValueLow;
	}

	startIndex += 8;

	// Third Color
	for(uint8_t i = 0; i < 8; i ++)
	{
		// Set the duty cycle buffer according to the color bits
		if(_led_matrix[ledIndexTmp].Blue & (0x80 >> i))
			_aCCValueBuffer[startIndex + i] = _uwCCValueHigh;
		else
			_aCCValueBuffer[startIndex + i] = _uwCCValueLow;
	}
}

void LedController::CalculateBrightness(FFT_Result* fftResult, float32_t rmsVoltage, bool startupIterationsComplete)
{
	float32_t voltageRatio = 0.0f;

	_movAvgFilter->AddVoltageValue(rmsVoltage);

	if(!startupIterationsComplete)
		return;

	_filteredRmsVoltage = _movAvgFilter->GetAverageVoltage();

	if(_filteredRmsVoltage > _peakRmsVoltage)
	{
		_peakRmsVoltage = _filteredRmsVoltage;
	}

//	_rmsVoltageSumOverInterval += _filteredRmsVoltage;
//	_rmsVoltageSumCounter++;

	_filteredPeakRmsVoltage = _movAvgFilter->GetAveragePeakVoltage();



#ifndef USE_HSV

	if(_filteredPeakRmsVoltage > 0.0f)
	{
		voltageRatio = (_filteredRmsVoltage / _filteredPeakRmsVoltage);
	}

	_currentBrightness = this->CalculateRGBBrightness(fftResult, voltageRatio);

#endif

#ifdef USE_HSV

	if(_filteredPeakRmsVoltage > 0.0f)
	{
		voltageRatio = (_filteredRmsVoltage / _filteredPeakRmsVoltage) * _brightnessFactors.All;
	}

	HSVBrightness hsvParamsRed = CalculateHSVBrightness(fftResult, voltageRatio, Red);
	RgbLedBrightness rgbParamsRed = TransfromHSVToRGB(hsvParamsRed);

	HSVBrightness hsvParamsGreen = CalculateHSVBrightness(fftResult, voltageRatio, Green);
	RgbLedBrightness rgbParamsGreen = TransfromHSVToRGB(hsvParamsGreen);

	HSVBrightness hsvParamsBlue = CalculateHSVBrightness(fftResult, voltageRatio, Blue);
	RgbLedBrightness rgbParamsBlue = TransfromHSVToRGB(hsvParamsBlue);

	RgbLedBrightness sumParams = {0};

	sumParams.Red = (rgbParamsRed.Red + rgbParamsGreen.Red + rgbParamsBlue.Red) / 3;
	sumParams.Green = (rgbParamsRed.Green + rgbParamsGreen.Green + rgbParamsBlue.Green) / 3;
	sumParams.Blue = (rgbParamsRed.Blue + rgbParamsGreen.Blue + rgbParamsBlue.Blue) / 3;

	sumParams.Red = (uint16_t)(sumParams.Red * _brightnessFactors.Red);
	sumParams.Green = (uint16_t)(sumParams.Green * _brightnessFactors.Green);
	sumParams.Blue = (uint16_t)(sumParams.Blue * _brightnessFactors.Blue);

	_currentUnfilteredBrightness = sumParams;

	// sumParams = ApplyOvershoot(sumParams);

	_movAvgFilter->AddBrightnessValue(sumParams);
	sumParams = _movAvgFilter->GetAverageBrightness();

	sumParams.Red = sumParams.Red < _maxBrightness ? sumParams.Red : _maxBrightness;
	sumParams.Green = sumParams.Green < _maxBrightness ? sumParams.Green : _maxBrightness;
	sumParams.Blue = sumParams.Blue < _maxBrightness ? sumParams.Blue : _maxBrightness;

	_currentBrightness = sumParams;

#endif

	_currentBrightness.Red = gamma8[_currentBrightness.Red];
	_currentBrightness.Green = gamma8[_currentBrightness.Green];
	_currentBrightness.Blue = gamma8[_currentBrightness.Blue];
}

void LedController::UpdateLEDColor()
{
	uint32_t max_i = _ledCount + _resetOffset;

	for(uint32_t i = _resetOffset; i < max_i; i++)
	{
		_led_matrix[i].Red = _currentBrightness.Red;
		_led_matrix[i].Green = _currentBrightness.Green;
		_led_matrix[i].Blue = _currentBrightness.Blue;
		_led_matrix[i].Reset = 0;
	}

	_ledMatrixFilled = SET;
	_dmaBufferNeedsUpdate = SET;
}

uint16_t LedController::CalculateBrightnessValueLinear(float32_t voltageRatio, float32_t userFactor, float32_t amplitudeRatio)
{
	return (uint16_t)(_maxBrightness * voltageRatio * amplitudeRatio * userFactor);
}

uint16_t LedController::CalculateBrightnessValueLog(float32_t voltageRatio, float32_t userFactor, float32_t amplitudeRatio)
{
	if(amplitudeRatio < 0.1f || voltageRatio < 0.1f)
		return 0;

	return (uint16_t)(255.0f * (1.0f + log10f(voltageRatio)) * (1.0f + log10f(amplitudeRatio * userFactor)));
}

HSVBrightness LedController::CalculateHSVBrightness(FFT_Result* fftResult, float32_t voltageRatio, ColorSelection selection)
{
	float32_t redFrequencyPart = 0.0f, greenFrequencyPart = 0.0f, blueFrequencyPart = 0.0f, sumFrequencyAmplitudes = 0.0f;

	CalculateFrequencyEnergy(fftResult, &redFrequencyPart, &greenFrequencyPart, &blueFrequencyPart);
	sumFrequencyAmplitudes = redFrequencyPart + greenFrequencyPart + blueFrequencyPart;

	uint16_t strongestFreq;
	HSVBrightness hsvParams;

	switch(selection)
	{
	case Red:
		strongestFreq = GetMaxFrequencyFromRange(fftResult, _colorBoundaries.Red.Min,
				_colorBoundaries.Red.Max);

		hsvParams = GetHSVParametersFromColor(redFrequencyPart / sumFrequencyAmplitudes, strongestFreq, voltageRatio, selection);

		break;
	case Green:
		strongestFreq = GetMaxFrequencyFromRange(fftResult, _colorBoundaries.Green.Min,
				_colorBoundaries.Green.Max);

		hsvParams = GetHSVParametersFromColor(greenFrequencyPart / sumFrequencyAmplitudes, strongestFreq, voltageRatio, selection);

		break;
	case Blue:
		strongestFreq = GetMaxFrequencyFromRange(fftResult, _colorBoundaries.Blue.Min,
				_colorBoundaries.Blue.Max);

		hsvParams = GetHSVParametersFromColor(blueFrequencyPart / sumFrequencyAmplitudes, strongestFreq, voltageRatio, selection);

		break;
	}

	return hsvParams;
}

uint16_t LedController::GetMaxFrequencyFromRange(FFT_Result* fftResult, uint16_t lower, uint16_t upper)
{
	uint16_t lowerIndex = lower / HZ_PER_SAMPLE;
	uint16_t upperIndex = upper / HZ_PER_SAMPLE;
	FFT_Result maxFreq = {0};

	for(uint16_t i = lowerIndex; i <= upperIndex; i++)
	{
		if(fftResult[i].absoluteValue > maxFreq.absoluteValue)
		{
			maxFreq = fftResult[i];
		}
	}

	return maxFreq.frequency;
}

HSVBrightness LedController::GetHSVParametersFromColor(float32_t freqEnergyRatio, uint16_t strongestFrequency, float32_t voltageRatio, ColorSelection color)
{
	HSVBrightness hsvParams = {0};
	uint16_t angle = 0;
	float32_t k = 0.0f, d = 0.0f;

	float32_t brightness = freqEnergyRatio * voltageRatio;

	hsvParams.Saturation = 1.0f;
	hsvParams.Brightness = brightness <= 1.0f ? brightness : 1.0f;

	switch(color)
	{
	case Red:
		k = (15.0f - 0.0f) / (((float32_t)_colorBoundaries.Red.Max) - ((float32_t)_colorBoundaries.Red.Min));

		d = 15.0f - ((float32_t)_colorBoundaries.Red.Max) * k;


		break;
	case Green:
		k = (135.0f - 105.0f) / (((float32_t)_colorBoundaries.Green.Max) - ((float32_t)_colorBoundaries.Green.Min));

		d = 135.0f - ((float32_t)_colorBoundaries.Green.Max) * k;

		break;
	case Blue:
		k = (240.0f - 210.0f) / (((float32_t)_colorBoundaries.Blue.Max) - ((float32_t)_colorBoundaries.Blue.Min));

		d = 240.0f - ((float32_t)_colorBoundaries.Blue.Max) * k;

		break;
	}

	angle = (uint16_t)(((float32_t)strongestFrequency) * k + d);

	hsvParams.Angle = angle <= 240 ? angle : 240;

	return hsvParams;
}

RgbLedBrightness LedController::TransfromHSVToRGB(HSVBrightness source)
{
	// https://de.wikipedia.org/wiki/HSV-Farbraum
	// h_i -> round off
	uint16_t h_i = (uint16_t)(source.Angle / 60.0f);
	float32_t f = (source.Angle / 60.0f) - h_i;

	float32_t p = source.Brightness * ( 1.0f - source.Saturation );
	float32_t q = source.Brightness * ( 1.0f - source.Saturation * f);
	float32_t t = source.Brightness * ( 1.0f - source.Saturation * (1.0f - f));

	// RGB values between 0 and 1
	float32_t red, green, blue;

	switch(h_i)
	{
	case 1:
		red = q;
		green = source.Brightness;
		blue = p;

		break;
	case 2:
		red = p;
		green = source.Brightness;
		blue = t;

		break;
	case 3:
		red = p;
		green = q;
		blue = source.Brightness;

		break;
	case 4:
		red = t;
		green = p;
		blue = source.Brightness;

		break;
	case 5:
		red = source.Brightness;
		green = p;
		blue = q;

		break;
	default:	// 0 or 6
		red = source.Brightness;
		green = t;
		blue = p;

		break;
	}

	RgbLedBrightness brightness = {0};

	brightness.Red = (uint16_t)(red * _maxBrightness);
	brightness.Green = (uint16_t)(green * _maxBrightness);
	brightness.Blue = (uint16_t)(blue * _maxBrightness);

	return brightness;
}


void LedController::CalculateFrequencyEnergy(FFT_Result* fftResult, float32_t* red, float32_t* green, float32_t* blue)
{
	for(uint16_t i = 0; i < MAX_FFT_RESULT_INDEX; i++)
	{
		if((fftResult[i].frequency >= _colorBoundaries.Red.Min) &&
				(fftResult[i].frequency < _colorBoundaries.Red.Max))
		{
			*red += fftResult[i].absoluteValue;
		}

		if((fftResult[i].frequency >= _colorBoundaries.Green.Min ) &&
				(fftResult[i].frequency < _colorBoundaries.Green.Max))
		{
			*green += fftResult[i].absoluteValue;
		}

		if((fftResult[i].frequency >= _colorBoundaries.Blue.Min ) &&
				(fftResult[i].frequency < _colorBoundaries.Blue.Max))
		{
			*blue += fftResult[i].absoluteValue;
		}
	}
}

RgbLedBrightness LedController::CalculateRGBBrightness(FFT_Result* fftResult, float32_t voltageRatio)
{
	float32_t redFrequencyPart = 0.0f, greenFrequencyPart = 0.0f, blueFrequencyPart = 0.0f, sumFrequencyAmplitudes = 0.0f;
	RgbLedBrightness filteredBrightness = {0};

	CalculateFrequencyEnergy(fftResult, &redFrequencyPart, &greenFrequencyPart, &blueFrequencyPart);
	sumFrequencyAmplitudes = redFrequencyPart + greenFrequencyPart + blueFrequencyPart;

#ifdef EXP_MODE
	// Amplify the colors which are most present compared to the whole signal
//	_currentBrightness.Red = (uint8_t)(255.0f * (1.0f - expf((amplitudeAfterFilter.red * _brightnessFactors.Red * -1.0f) / amplitudeAfterFilter.sum)));
//	_currentBrightness.Green = (uint8_t)(255.0f * (1.0f - expf((amplitudeAfterFilter.green * _brightnessFactors.Green * -1.0f) / amplitudeAfterFilter.sum)));
//	_currentBrightness.Blue = (uint8_t)(255.0f * (1.0f - expf((amplitudeAfterFilter.blue * _brightnessFactors.Blue * -1.0f) / avgFrequencyAmplitudes.sum)));

//	float32_t amplitudeLog = log10f(_settingsCtrl->settingsData.amplitude);

	uint16_t redBrightness = CalculateBrightnessValueLog(voltageRatio, _settingsCtrl->settingsData.factors.Red, filteredBrightness.red, sumAmplitudes);
	uint16_t greenBrightness = CalculateBrightnessValueLog(voltageRatio, _settingsCtrl->settingsData.factors.Green, filteredBrightness.green, sumAmplitudes);
	uint16_t blueBrightness = CalculateBrightnessValueLog(voltageRatio, _settingsCtrl->settingsData.factors.Blue, filteredBrightness.blue, sumAmplitudes);
#endif

#ifdef LOG_MODE_2

	float32_t logAmplitude = logf(_settingsCtrl->settingsData.amplitude);

	uint16_t redBrightness = (uint16_t)(255.0f * _settingsCtrl->settingsData.factors.Red *
			 (log10f(filteredBrightness.red) / logAmplitude) );

	uint16_t greenBrightness = (uint16_t)(255.0f * _settingsCtrl->settingsData.factors.Green *
			(log10f(filteredBrightness.green) / logAmplitude) );

	uint16_t blueBrightness = (uint16_t)(255.0f * _settingsCtrl->settingsData.factors.Blue *
			(log10f(filteredBrightness.blue) / logAmplitude) );

#endif

#ifdef LINEAR
	uint16_t redBrightness = 0;
	uint16_t greenBrightness = 0;
	uint16_t blueBrightness = 0;

	redBrightness = CalculateBrightnessValueLinear(voltageRatio, _brightnessFactors.Red, redFrequencyPart / sumFrequencyAmplitudes);
	greenBrightness = CalculateBrightnessValueLinear(voltageRatio, _brightnessFactors.Green, greenFrequencyPart / sumFrequencyAmplitudes);
	blueBrightness = CalculateBrightnessValueLinear(voltageRatio, _brightnessFactors.Blue, blueFrequencyPart / sumFrequencyAmplitudes);

#endif
	_movAvgFilter->AddBrightnessValue({redBrightness, greenBrightness, blueBrightness});
	filteredBrightness = _movAvgFilter->GetAverageBrightness();

	RgbLedBrightness brightness = ApplyOvershoot(filteredBrightness);

	brightness.Red = brightness.Red < _maxBrightness ? brightness.Red : _maxBrightness;
	brightness.Green = brightness.Green < _maxBrightness ? brightness.Green : _maxBrightness;
	brightness.Blue = brightness.Blue < _maxBrightness ? brightness.Blue : _maxBrightness;

	return brightness;
}

RgbLedBrightness LedController::ApplyOvershoot(RgbLedBrightness source)
{
	uint16_t brightnessArray[3] = {0};
	uint16_t maxIndex = 0;
	float32_t overshootFactor = 0.0f;
	RgbLedBrightness brightness = {0};
	uint16_t currentMaxBrightness = 0;

	brightnessArray[0] = source.Red;
	brightnessArray[1] = source.Green;
	brightnessArray[2] = source.Blue;

	for(uint16_t i = 0; i < 3; i++)
	{
		if(brightnessArray[i] > currentMaxBrightness)
		{
			currentMaxBrightness = brightnessArray[i];
			maxIndex = i;
		}
	}

	if(currentMaxBrightness > _maxBrightness)
	{
		overshootFactor = (float32_t)_maxBrightness / (float32_t)currentMaxBrightness;

		for(uint16_t i = 0; i < 3; i++)
		{
			if(i == maxIndex)
				continue;

			brightnessArray[i] = (uint16_t)(brightnessArray[i] * overshootFactor);
		}
	}

	brightness.Red = brightnessArray[0];
	brightness.Green = brightnessArray[1];
	brightness.Blue = brightnessArray[2];

	return brightness;
}

RgbLedBrightness LedController::PrioritizeStrongestColor(RgbLedBrightness brightness)
{
	uint16_t brightnessArray[3] = {0};
	uint16_t highestIndex = 0, middleIndex = 0, lowestIndex = 0;
	uint16_t highestBrightness = 0, middleBrightness = 0, lowestBrightness = 0;

	brightnessArray[0] = brightness.Red;
	brightnessArray[1] = brightness.Green;
	brightnessArray[2] = brightness.Blue;

	// Get the ranks of brightness values
	for(uint16_t i = 0; i < 3; i++)
	{
		if(brightnessArray[i] > highestBrightness)
		{
			highestBrightness = brightnessArray[i];
			highestIndex = i;
		}
	}

	for(uint16_t i = 0; i < 3; i++)
	{
		if(i == highestIndex)
			continue;

		if(brightnessArray[i] > middleBrightness)
		{
			middleBrightness = brightnessArray[i];
			middleIndex = i;
		}
	}

	for(uint16_t i = 0; i < 3; i++)
	{
		if(i == highestIndex || i == middleIndex)
			continue;

		lowestBrightness = brightnessArray[i];
		lowestIndex = i;
	}

	// All colors are over a certain value -> looks white. Prio the highest color
	if(lowestBrightness > _prioThreshold && middleBrightness > _prioThreshold && highestBrightness > _prioThreshold)
	{
		brightnessArray[lowestIndex] *= _prioAttenuation;
	}

	brightness.Red = brightnessArray[0];
	brightness.Green = brightnessArray[1];
	brightness.Blue = brightnessArray[2];

	return brightness;
}

uint8_t LedController::IsLedUpdateComplete()
{
	return _ledMatrixFilled && !_dmaBufferNeedsUpdate;
}

void LedController::SetFilterOrder(FilterLevels filterOrders)
{
	_movAvgFilter->SetFilterOrder(filterOrders);
}

void LedController::SetColorFilterOrder(FilterLevelsColor filterOrdersColor)
{
	_movAvgFilter->SetColorFilterOrder(filterOrdersColor);
}

FilterLevels LedController::GetFilterOrder()
{
	return _movAvgFilter->GetFilterOrder();
}

void LedController::ResetLedMatrix()
{
	_currentLedIndex = 0;
	InitLedMatrix();
}

RgbLedBrightness LedController::GetCurrentBrightness()
{
	return _currentBrightness;
}

RgbLedBrightness LedController::GetCurrentUnfilteredBrightness()
{
	return _currentUnfilteredBrightness;
}

float32_t LedController::GetRMSVoltage()
{
	return _filteredRmsVoltage;
}

float32_t LedController::GetPeakRMSVoltage()
{
	return _filteredPeakRmsVoltage;
}

void LedController::SetBrightnessFactors(BrightnessFactors factors)
{
	_brightnessFactors = factors;
}

BrightnessFactors LedController::GetBrightnessFactors()
{
	return _brightnessFactors;
}

void LedController::SetColorBoundaries(FrequencyColorBoundaries boundaries)
{
	_colorBoundaries = boundaries;
}

FrequencyColorBoundaries LedController::GetColorBoundaries()
{
	return _colorBoundaries;
}

LedController::~LedController()
{
	
}

void LedController::ErrorHandler()
{
	while(1)
	{

	}
}

