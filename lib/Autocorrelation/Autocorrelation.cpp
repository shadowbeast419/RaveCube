
#include <Autocorrelation.hpp>

Autocorrelation::Autocorrelation()
{

}

Autocorrelation::~Autocorrelation()
{

}

void Autocorrelation::Init()
{
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
		_ringBuffer[i].red = 0.0f;
        _ringBuffer[i].green = 0.0f;
        _ringBuffer[i].blue = 0.0f;
	}
}

void Autocorrelation::AddFftResult(FFT_Result* result)
{
    // Are we at the end of the Array?
    if(_elementCount >= _sequenceIntervals)
    {
        _bufferIsFull = true;
    }

    uint16_t dataCount = FFT_SAMPLE_COUNT / 2;
    _currentRingNode->red = 0.0f;
    _currentRingNode->green = 0.0f;
    _currentRingNode->blue = 0.0f;

    for(uint16_t i = 0; i < dataCount; i++)
    {
        if(result[i].frequency >= _colorBoundaries.Red.Min && result[i].frequency <= _colorBoundaries.Red.Max)
        {
            _currentRingNode->red += result[i].absoluteValue;
        }

        if(result[i].frequency >= _colorBoundaries.Green.Min && result[i].frequency <= _colorBoundaries.Green.Max)
        {
            _currentRingNode->green += result[i].absoluteValue;
        }

        if(result[i].frequency >= _colorBoundaries.Blue.Min && result[i].frequency <= _colorBoundaries.Blue.Max)
        {
            _currentRingNode->blue += result[i].absoluteValue;
        }
    }

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

    float32_t* pLagArray = _lagArray;
    float32_t maxLagValue = 0.0f;
    uint32_t maxLagIndex;

    StoreElementsIntoBuffer(color);

    for(uint16_t i = 0; i < _sequenceIntervals * 2; i++)
    {
        _lagArraySymmetric[i] = 0.0f;
    }

    arm_correlate_f32(pLagArray, _sequenceIntervals, pLagArray, _sequenceIntervals, _lagArraySymmetric);

    uint16_t startIndex = _sequenceIntervals - 1;

    for(uint16_t i = 0; i < _sequenceIntervals; i++)
    {
        pLagArray[i] = _lagArraySymmetric[startIndex + i];
    }

    arm_max_f32(pLagArray, _sequenceIntervals, &maxLagValue, &maxLagIndex);

    for(uint16_t i = 0; i < _sequenceIntervals; i++)
    {
        pLagArray[i] /= maxLagValue;
    }

    return pLagArray;
 }

void Autocorrelation::StoreElementsIntoBuffer(ColorSelection color)
{
    uint16_t dataCount = _elementCount;
    uint32_t sum = 0;
    float32_t avg = 0.0f;
    RingBufferNodeFrequencyEnergy* currentNode = _currentRingNode;

    for(uint16_t i = 0; i < dataCount; i++)
    {
        currentNode = currentNode->pPrev;;
    }

    for(uint16_t i = 0; i < dataCount; i++)
    {
        switch(color)
        {
            case Red:
                _lagArray[i] = currentNode->red;
                sum += currentNode->red;
                break;

            case Green:
                _lagArray[i] = currentNode->green;
                sum += currentNode->green;

                break;

            case Blue:
                _lagArray[i] = currentNode->blue;
                sum += currentNode->blue;

                break;
        }

        currentNode = currentNode->pNext;
    }

    avg = sum / (float32_t)dataCount;

    for(uint16_t i = 0; i < dataCount; i++)
    {
        _lagArray[i] -= avg;
    }
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