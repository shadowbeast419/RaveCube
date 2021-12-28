
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
		_ringBuffer[i].energy = 0.0f;
	}
}

void Autocorrelation::AddFrequencyEnergy(FFT_Result* fftResult, int sampleCount)
{
    // Are we at the end of the Array?
    if(_elementCount >= _sequenceIntervals)
    {
        _bufferIsFull = true;
    }

    // Get the frequency boundary of the BASS
    FrequencyBoundary lowestFreqBoundary = _settingsCtrl->GetLowestFrequencyBoundary();
    float32_t sumOfFreqEnergy = 0.0f;

    for(int i = 0; i < sampleCount; i++)
    {
        if(fftResult[i].frequency >= lowestFreqBoundary.Min && 
            fftResult[i].frequency <= lowestFreqBoundary.Max)
        {
            sumOfFreqEnergy += fftResult[i].absoluteValue;
        }
    }

    _currentRingNode->energy = sumOfFreqEnergy;
    _currentRingNode = _currentRingNode->pNext;

    if(!_bufferIsFull)
        _elementCount++;
}

float32_t* Autocorrelation::Autocorrelate(int16_t* maxIndex)
{
    // ElementIndex can be the count as maximum
    *maxIndex = _elementCount - 1;

    uint16_t dataCount = _elementCount;
    float32_t mean = CalculateMeanOfBuffer();
    float32_t deviation = CalculateDeviationOfBuffer(mean);

    for(uint16_t j = 0; j < dataCount; j++)
    {
        _lagArray[j] = 0.0f;

        for(uint16_t i = 0; i < dataCount - j; i++)
        {
            _lagArray[j] += ((GetElementOfBuffer(j) - mean) * 
                (GetElementOfBuffer(i+j) - mean));
        }

        if(deviation > 0.0f)
        {
            _lagArray[j] /= deviation;
        }
        else
        {
            _lagArray[j] = -100.0f;
        }
    }

    return _lagArray;
}

float32_t Autocorrelation::CalculateMeanOfBuffer()
{
    uint16_t dataCount = _elementCount;
    float32_t sum = 0.0f;
    RingBufferNodeFrequencyEnergy* currentNode = _currentRingNode;

    for(uint16_t i = 0; i < dataCount; i++)
    {
        sum += currentNode->energy;
        currentNode = currentNode->pPrev;
    }

    return sum / (float32_t)dataCount;
}

float32_t Autocorrelation::CalculateDeviationOfBuffer(float32_t mean)
{
    uint16_t dataCount = _elementCount;
    float32_t sum = 0.0f;
    RingBufferNodeFrequencyEnergy* currentNode = _currentRingNode;

    for(uint16_t i = 0; i < dataCount; i++)
    {
        sum += ((currentNode->energy - mean) * (currentNode->energy - mean));
        currentNode = currentNode->pPrev;
    }

    return sum;
}

float32_t Autocorrelation::GetElementOfBuffer(uint16_t index)
{
    if(index >= _elementCount)
        return -1;

    uint16_t prevElements = _elementCount - index;
    RingBufferNodeFrequencyEnergy* currentNode = _currentRingNode;

    for(uint16_t i = 0; i < prevElements; i++)
    {
        currentNode = currentNode->pPrev;
    }

    return currentNode->energy;
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