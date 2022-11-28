
#ifndef _AUTOCORRELATION_HPP
#define _AUTOCORRELATION_HPP

#include <main.hpp>
#include <FFT.hpp>
#include <RgbLedBrightness.hpp>
#include <SettingsController.hpp>
#include <arm_math.h>

struct RingBufferNodeRgbBrightness
{
	struct RingBufferNodeRgbBrightness* pPrev;
	struct RingBufferNodeRgbBrightness* pNext;
	RgbLedBrightness rgbBrightness;
};

struct CorrelationCoefficients
{
    float32_t Red;
    float32_t Green;
    float32_t Blue;
};

#define MAX_SEQUENCE_INTERVALS 80

class Autocorrelation
{
public:
    Autocorrelation();
    ~Autocorrelation();     

    void                            Init(SettingsController* settingsCtrl);
    void                            AddRgbBrightness(RgbLedBrightness rgbBrightness);
    float32_t*                      Autocorrelate(int16_t* maxIndex, ColorSelection color);

    uint16_t                        GetSequenceIntervals();
    void                            SetSequenceIntervals(uint16_t sequenceIntervals);
    bool                            IsBufferFull();
    uint16_t                        GetDataCount();
    void                            ClearRingBuffer();
    const float32_t                 InvalidLagValue = -100.0f;
    uint16_t                        MinLag = 10;
    uint16_t                        MaxLag = MAX_SEQUENCE_INTERVALS;


private:
    void                            InitRingBuffer(uint16_t sequenceIntervals);
    float32_t                       CalculateMeanOfBuffer(ColorSelection color);
    float32_t                       CalculateDeviationOfBuffer(float32_t mean, ColorSelection color);
    float32_t                       GetElementOfBuffer(uint16_t index, ColorSelection color);

    SettingsController*             _settingsCtrl = NULL;
    uint16_t                        _sequenceIntervals = MAX_SEQUENCE_INTERVALS;
    RingBufferNodeRgbBrightness     _ringBuffer[MAX_SEQUENCE_INTERVALS];
    RingBufferNodeRgbBrightness*    _currentRingNode = NULL;
    uint16_t                        _elementCount = 0;
    bool                            _bufferIsFull = false;  

    // Only create lags-array where elements are inside with more than one sample 
    // everything above doesn't make sense
    float32_t                       _lagArrayRed[MAX_SEQUENCE_INTERVALS];
    float32_t                       _lagArrayGreen[MAX_SEQUENCE_INTERVALS];
    float32_t                       _lagArrayBlue[MAX_SEQUENCE_INTERVALS];
};


#endif