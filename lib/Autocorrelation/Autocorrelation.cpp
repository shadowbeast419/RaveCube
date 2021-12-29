
#include <Autocorrelation.hpp>

Autocorrelation::Autocorrelation()
{

}

void Autocorrelation::Init(SettingsController* settingsCtrl)
{
    _settingsCtrl = settingsCtrl;
    InitRingBuffer(_sequenceIntervals);
}


void Autocorrelation::ClearRingBuffer()
{
    InitRingBuffer(_sequenceIntervals);

    _elementCount = 0;
    _bufferIsFull = false;
}

void Autocorrelation::InitRingBuffer(uint16_t sequenceIntervals)
{
    RgbLedBrightness emptyBrightness = {0};

    // Same for amplitude voltage
	for(uint16_t i = sequenceIntervals - 1; i > 0; i--)
	{
		_ringBuffer[i].pPrev = &_ringBuffer[i-1];
	}

	for(uint16_t i = 0; i < (uint16_t)(sequenceIntervals - 1); i++)
	{
		_ringBuffer[i].pNext = &_ringBuffer[i+1];
	}

	_ringBuffer[0].pPrev = &_ringBuffer[sequenceIntervals - 1];
	_ringBuffer[sequenceIntervals - 1].pNext = &_ringBuffer[0];

	_currentRingNode = &_ringBuffer[0];

	for(uint16_t i = 0; i < sequenceIntervals; i++)
	{
		_ringBuffer[i].rgbBrightness = emptyBrightness;
	}
}

void Autocorrelation::AddRgbBrightness(RgbLedBrightness rgbBrightness)
{
    // Are we at the end of the Array?
    if(_elementCount >= _sequenceIntervals)
    {
        _bufferIsFull = true;
    }

    // // Get the frequency boundary of the BASS
    // FrequencyBoundary lowestFreqBoundary = _settingsCtrl->GetLowestFrequencyBoundary();
    // float32_t sumOfFreqEnergy = 0.0f;

    _currentRingNode->rgbBrightness = rgbBrightness;
    _currentRingNode = _currentRingNode->pNext;

    if(!_bufferIsFull)
        _elementCount++;
}

float32_t* Autocorrelation::Autocorrelate(int16_t* maxIndex, ColorSelection color)
{
    // ElementIndex can be the count as maximum
    *maxIndex = MaxLag - 1;

    if(_elementCount < MaxLag)
    {
        *maxIndex = -1;
        return NULL;
    }

    uint16_t dataCount = _elementCount;
    float32_t mean = CalculateMeanOfBuffer(color);
    float32_t deviation = CalculateDeviationOfBuffer(mean, color);
    float32_t* pLagArray = NULL;

    switch(color)
    {
        case Red:
            pLagArray = _lagArrayRed;
            break;
        case Green:
            pLagArray = _lagArrayGreen;
            break;
        case Blue:
            pLagArray = _lagArrayBlue;
            break;
    }

    uint16_t lagArrayIndex = 0;

    for(uint16_t lag = MinLag; lag < MaxLag; lag++)
    {   
        lagArrayIndex = lag - MinLag;
        pLagArray[lagArrayIndex] = 0.0f;

        for(uint16_t i = 0; i < dataCount - lagArrayIndex; i++)
        {
            pLagArray[lagArrayIndex] += ((GetElementOfBuffer(lagArrayIndex, color) - mean) * 
                (GetElementOfBuffer(i+lagArrayIndex, color) - mean));
        }

        if(deviation > 0.0f)
        {
            pLagArray[lagArrayIndex] /= deviation;
        }
        else
        {
            pLagArray[lagArrayIndex] = InvalidLagValue;
        }
    }

    return pLagArray;
}

float32_t Autocorrelation::CalculateMeanOfBuffer(ColorSelection color)
{
    uint16_t dataCount = _elementCount;
    uint32_t sum = 0;
    RingBufferNodeRgbBrightness* currentNode = _currentRingNode;

    for(uint16_t i = 0; i < dataCount; i++)
    {
        switch(color)
        {
            case Red:
                sum += currentNode->rgbBrightness.Red;
                break;

            case Green:
                sum += currentNode->rgbBrightness.Green;
                break;

            case Blue:
                sum += currentNode->rgbBrightness.Blue;
                break;
        }

        currentNode = currentNode->pPrev;
    }

    return (float32_t)sum / (float32_t)dataCount;
}

float32_t Autocorrelation::CalculateDeviationOfBuffer(float32_t mean, ColorSelection color)
{
    uint16_t dataCount = _elementCount;
    float32_t sum = 0.0f;
    RingBufferNodeRgbBrightness* currentNode = _currentRingNode;
    float32_t brightness = 0;

    for(uint16_t i = 0; i < dataCount; i++)
    {   
        switch(color)
        {
            case Red:
                brightness = (float32_t)currentNode->rgbBrightness.Red;
                break;
            case Green:
                brightness = (float32_t)currentNode->rgbBrightness.Green;
                break;
            case Blue:
                brightness = (float32_t)currentNode->rgbBrightness.Blue;
                break;
        }

        sum += ((brightness - mean) * (brightness - mean));
        currentNode = currentNode->pPrev;
    }

    return sum;
}

float32_t Autocorrelation::GetElementOfBuffer(uint16_t index, ColorSelection color)
{
    // Out of range
    if(index >= _elementCount)
        return 0.0f;

    uint16_t prevElements = _elementCount - index - 1;
    RingBufferNodeRgbBrightness* currentNode = _currentRingNode;

    for(uint16_t i = 0; i < prevElements; i++)
    {
        currentNode = currentNode->pPrev;
    }

    float32_t brightness = 0.0f;

    switch(color)
    {
        case Red:
            brightness = (float32_t)currentNode->rgbBrightness.Red;
            break;
        case Green:
            brightness = (float32_t)currentNode->rgbBrightness.Green;
            break;
        case Blue:
            brightness = (float32_t)currentNode->rgbBrightness.Blue;
            break;
    } 

    return (float32_t)brightness;
}

uint16_t Autocorrelation::GetDataCount()
{
    return _elementCount;
}

bool Autocorrelation::IsBufferFull()
{
    return _bufferIsFull;
}

void Autocorrelation::SetSequenceIntervals(uint16_t sequenceIntervals)
{
    _sequenceIntervals = sequenceIntervals;
}

uint16_t Autocorrelation::GetSequenceIntervals()
{
    return _sequenceIntervals;
}