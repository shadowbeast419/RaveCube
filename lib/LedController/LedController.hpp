/*
 * RGBLedController.h
 *
 *  Created on: 4 Oct 2020
 *      Author: Christoph
 */

#ifndef INC_LEDCONTROLLER_H_
#define INC_LEDCONTROLLER_H_

//#define EXP_MODE
//#define LOG_MODE_2
#define LINEAR
#define USE_HSV

#include "stm32g4xx_hal.h"
#include <RgbLedBrightness.hpp>
#include <LargeIntervalTimer.hpp>
#include <PwmGenerator.hpp>
#include <MovingAvgFilter.hpp>
#include <SettingsController.hpp>
#include <FFT.hpp>
#include <FilterLevels.hpp>
#include <FrequencyColorBoundaries.hpp>
#include <BrightnessFactors.hpp>

const uint8_t gamma8[256] = {
	    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
	    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
	    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
	    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
	   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
	   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
	   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
	   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
	   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
	   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
	   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
	  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
	  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
	  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
	  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

class SettingsController;

class LedController : public PwmGenerator
{

private:
	static LedController					_rgbInstance;
	uint32_t								_currentLedIndex = 0;
	uint32_t								_currentPulseIndex = 0;
	__IO uint8_t							_ledMatrixFilled = RESET;
	MovingAvgFilter* 						_movAvgFilter;
	RgbLedBrightness						_currentBrightness = {0};
	RgbLedBrightness						_currentUnfilteredBrightness = {0};
	BrightnessFactors						_brightnessFactors = BrightnessFactors();
	FrequencyColorBoundaries				_colorBoundaries;

	const uint16_t							_maxBrightness = 255;
	float32_t								_filteredPeakRmsVoltage = 0.0f;
	float32_t								_filteredMeanVoltage = 0.0f;
	float32_t								_peakMeanVoltage = 0.0f;
	uint8_t									_dmaBufferNeedsUpdate = SET;
	LargeIntervalTimer						_peakTimer = LargeIntervalTimer(TIM4, 0.1f);

	/// @brief For Priotizing strongest Color (not used)
	uint16_t 								_prioThreshold = 50;
	/// @brief For Priotizing strongest Color (not used)
	float32_t 								_prioAttenuation = 0.3f;

	LedController();
	void InitLedMatrix();
	void UpdatePwmSettingsBuffer(uint32_t ledIndexTmp, uint8_t ledIsOdd);
	void ErrorHandler(void);
	uint16_t CalculateBrightnessValueLinear(float32_t voltageRatio, float32_t userFactor, float32_t amplitudeRatio);
	uint16_t CalculateBrightnessValueLog(float32_t voltageRatio, float32_t userFactor, float32_t amplitudeRatio);
	void CalculateFrequencyEnergy(FFT_Result* fftResult, float32_t* red, float32_t* green, float32_t* blue);
	HSVBrightness CalculateHSVBrightness(FFT_Result* fftResult, float32_t voltageRatio, ColorSelection selection);
	HSVBrightness GetHSVParametersFromColor(float32_t freqEnergyRatio, uint16_t strongestFrequency, float32_t voltageRatio, ColorSelection color);
	uint16_t GetMaxFrequencyFromRange(FFT_Result* fftResult, uint16_t lower, uint16_t upper);

	RgbLedBrightness TransfromHSVToRGB(HSVBrightness source);
	RgbLedBrightness CalculateRGBBrightness(FFT_Result* fftResult, float32_t voltageRatio);
	RgbLedBrightness ApplyOvershoot(RgbLedBrightness source);
	RgbLedBrightness PrioritizeStrongestColor(RgbLedBrightness brightness);

public:
	void Init(MovingAvgFilter* movingAvgFilter);
	static LedController* GetInstance();
	void PulseFinishedHandler(TIM_HandleTypeDef *htim, uint8_t isFullyCpltFlag);
	void PeakTimerInterruptHandler();
	void PeakTimerPeriodElapsedHandler(TIM_HandleTypeDef *htim);

	uint8_t IsLedUpdateComplete();
	void CalculateBrightness(FFT_Result* fftResult, float32_t peakVoltage, bool startupIterationsComplete);
	RgbLedBrightness FilterBrightness(RgbLedBrightness brightness);
	void UpdateLEDColor();

	RgbLedBrightness GetCurrentBrightness();
	RgbLedBrightness GetCurrentUnfilteredBrightness();

	void SetFilterOrder(FilterLevels filterOrders);
	FilterLevels GetFilterOrder();
	void SetColorFilterOrder(FilterLevelsColor filterOrdersColor);

	float32_t GetRMSVoltage();
	float32_t GetPeakRMSVoltage();

	void SetBrightnessFactors(BrightnessFactors factors);
	BrightnessFactors GetBrightnessFactors();

	void SetColorBoundaries(FrequencyColorBoundaries boundaries);
	FrequencyColorBoundaries GetColorBoundaries();

	void ResetLedMatrix();

	virtual ~LedController();
};

#endif /* INC_LEDCONTROLLER_H_ */
