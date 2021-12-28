
#ifndef _AUTOCORRELATION_HPP
#define _AUTOCORRELATION_HPP

#include <main.hpp>
#include <FFT.hpp>
#include <RgbLedBrightness.hpp>
#include <SettingsController.hpp>
#include <arm_math.h>

struct RingBufferNodeFrequencyEnergy
{
	struct RingBufferNodeFrequencyEnergy* pPrev;
	struct RingBufferNodeFrequencyEnergy* pNext;
	float32_t energy;
};

#define MAX_SEQUENCE_INTERVALS 100

class Autocorrelation
{
public:
    Autocorrelation();
    ~Autocorrelation();     

    void                            Init(SettingsController* settingsCtrl);
    void                            AddFrequencyEnergy(FFT_Result* fftResult, int sampleCount);
    float32_t*                      Autocorrelate(int16_t* maxIndex);

    uint16_t                        GetSequenceIntervals();
    void                            SetSequenceIntervals(uint16_t sequenceIntervals);
    bool                            IsBufferFull();
    uint16_t                        GetDataCount();
    void                            ClearRingBuffer();

private:
    void                            InitRingBuffer(uint16_t sequenceIntervals);
    float32_t                       CalculateMeanOfBuffer();
    float32_t                       CalculateDeviationOfBuffer(float32_t mean);
    float32_t                       GetElementOfBuffer(uint16_t index);

    SettingsController*             _settingsCtrl = NULL;
    uint16_t                        _sequenceIntervals = 50;
    RingBufferNodeFrequencyEnergy   _ringBuffer[MAX_SEQUENCE_INTERVALS];
    RingBufferNodeFrequencyEnergy*  _currentRingNode = NULL;
    uint16_t                        _elementCount = 0;
    bool                            _bufferIsFull = false;  

    // Only create lags-array where elements are inside with more than one sample 
    // everything above doesn't make sense
    float32_t                       _lagArray[MAX_SEQUENCE_INTERVALS];
};


#endif