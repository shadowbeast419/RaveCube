
#ifndef _BEAT_DETECTOR_HPP
#define _BEAT_DETECTOR_HPP

#include <MovingAvgFilter.hpp>
#include <UartController.hpp>
#include <Autocorrelation.hpp>
#include <SettingsController.hpp>
#include <FFT.hpp>
#include <string>

#define FILTER_ORDER_MAX 3

struct BeatsPerMinute
{
    uint16_t Red;
    uint16_t Green;
    uint16_t Blue;
};

struct CorrelationResult
{
    BeatsPerMinute Bpm;
    CorrelationCoefficients CorrCoeff;
};
struct RingBufferNodeBPM
{
	struct RingBufferNodeBPM* pPrev;
	struct RingBufferNodeBPM* pNext;
    CorrelationResult result;
};
class BeatDetector
{
    public:
        BeatDetector(SettingsController* settingsCtrl, UartController* uartCtrl, 
            uint32_t sampleFrequency, uint16_t sampleCount);
        ~BeatDetector();

        CorrelationResult CalculateBeatsPerMinute(RgbLedBrightness rgbBrightness);
        void GetBpmBoundaries(uint16_t* minValue, uint16_t* maxValue);
        bool CalculateFilterLevels(CorrelationResult result, FilterLevels* filterLevels);

        bool                            EnableOutputToUart = true;
        bool                            UseAbsValueOfCorrelation = false;
    private:
        void InitRingBuffer();
        CorrelationResult AddElementToFilter(CorrelationResult bpm);
        void CalculateMaxLagValues(ColorSelection color, uint16_t* maxLagIndex, float32_t* maxLagValue);
        uint16_t CalculateBpmFromLagValue(uint16_t maxLagIndex);
        void WriteResultToUart(ColorSelection color, CorrelationResult result);

        Autocorrelation                 _correlation = Autocorrelation();
        SettingsController*             _settingsCtrl = NULL;
        UartController*                 _uartCtrl = NULL;
        uint32_t                        _sampleFrequency = 0;
        uint16_t                        _sampleCount = 0;
        float32_t                       _periodOfLag = 0.0f;
        char                            _transmitBuffer[150];
        RingBufferNodeBPM               _ringBufferBPM[FILTER_ORDER_MAX];
        RingBufferNodeBPM*              _currentNode = NULL;

        // Should be <1.0 with Max MvgAvg Filter Order < 128 (otherwise too much lightning :D)
        float32_t                       _bpmToFilterSensibility = 1.0f;
};

#endif
