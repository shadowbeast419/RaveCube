
#ifndef _AUTOCORRELATION_HPP
#define _AUTOCORRELATION_HPP

#include <main.hpp>
#include <FFT.hpp>
#include <RgbLedBrightness.hpp>
#include <FrequencyColorBoundaries.hpp>
#include <SettingsController.hpp>
#include <arm_math.h>

struct RingBufferNodeFrequencyEnergy
{
    struct RingBufferNodeFrequencyEnergy* pPrev;
    struct RingBufferNodeFrequencyEnergy* pNext;
    float32_t red;
    float32_t green;
    float32_t blue; 
};

struct CorrelationCoefficients
{
    float32_t Red;
    float32_t Green;
    float32_t Blue;
};

#define MAX_SEQUENCE_INTERVALS 100

class Autocorrelation
{
public:
    Autocorrelation();
    virtual ~Autocorrelation();     

    void                            Init();
    void                            AddFftResult(FFT_Result* fftResult);
    float32_t*                      Autocorrelate(int16_t* maxIndex, ColorSelection color);

    uint16_t                        GetSequenceIntervals();
    void                            SetSequenceIntervals(uint16_t sequenceIntervals);
    bool                            IsBufferFull();
    uint16_t                        GetDataCount();
    void                            ClearRingBuffer();
    const float32_t                 InvalidLagValue = -100.0f;
    uint16_t                        MinLag = 20;
    uint16_t                        MaxLag = 80;


private:
    void                            InitRingBuffer(uint16_t sequenceIntervals);
    void                            StoreElementsIntoBuffer(ColorSelection color);

    FrequencyColorBoundaries		_colorBoundaries;
    uint16_t                        _sequenceIntervals = MAX_SEQUENCE_INTERVALS;
    RingBufferNodeFrequencyEnergy   _ringBuffer[MAX_SEQUENCE_INTERVALS];
    RingBufferNodeFrequencyEnergy*  _currentRingNode = NULL;
    uint16_t                        _elementCount = 0;
    bool                            _bufferIsFull = false;  

    // Only create lags-array where elements are inside with more than one sample 
    // everything above doesn't make sense
    float32_t                       _lagArraySymmetric[MAX_SEQUENCE_INTERVALS * 2];
    float32_t                       _lagArray[MAX_SEQUENCE_INTERVALS];
};


#endif