
#ifndef _BEAT_DETECTOR_HPP
#define _BEAT_DETECTOR_HPP

#include <UartController.hpp>
#include <Autocorrelation.hpp>
#include <SettingsController.hpp>
#include <FFT.hpp>

#define FILTER_ORDER_MAX 20

struct RingBufferNodeBPM
{
	struct RingBufferNodeBPM* pPrev;
	struct RingBufferNodeBPM* pNext;
	uint16_t bpm;
};

class BeatDetector
{
    public:
        BeatDetector(SettingsController* settingsCtrl, UartController* uartCtrl, 
            uint32_t sampleFrequency, uint16_t sampleCount);
        ~BeatDetector();

        uint16_t CalculateBeatsPerMinute(FFT_Result* freqEnergies);

    private:
        void InitRingBuffer();
        uint16_t AddElementToFilter(uint16_t bpm);

        Autocorrelation                 _correlation = Autocorrelation();
        SettingsController*             _settingsCtrl = NULL;
        UartController*                 _uartCtrl = NULL;
        uint32_t                        _sampleFrequency = 0;
        uint16_t                        _sampleCount = 0;
        char                            _transmitBuffer[150];
        RingBufferNodeBPM               _ringBufferBPM[FILTER_ORDER_MAX];
        RingBufferNodeBPM*              _currentNode = NULL;
};

#endif
