

#include <main.hpp>
#include <BeatDetector.hpp>

BeatDetector::BeatDetector(SettingsController* settingsCtrl, UartController* uartCtrl, 
    uint32_t sampleFrequency, uint16_t sampleCount)
{
    _settingsCtrl = settingsCtrl;
    _uartCtrl = uartCtrl;
    _correlation.Init(_settingsCtrl);
    _sampleFrequency = sampleFrequency;
    _sampleCount = sampleCount;

    InitRingBuffer();
}

void BeatDetector::InitRingBuffer()
{
	for(uint16_t i = FILTER_ORDER_MAX - 1; i > 0; i--)
	{
		_ringBufferBPM[i].pPrev = &_ringBufferBPM[i-1];
	}

	for(uint16_t i = 0; i < (uint16_t)(FILTER_ORDER_MAX - 1); i++)
	{
		_ringBufferBPM[i].pNext = &_ringBufferBPM[i+1];
	}

	_ringBufferBPM[0].pPrev = &_ringBufferBPM[FILTER_ORDER_MAX - 1];
	_ringBufferBPM[FILTER_ORDER_MAX - 1].pNext = &_ringBufferBPM[0];

	_currentNode = &_ringBufferBPM[0];

	for(uint16_t i = 0; i < FILTER_ORDER_MAX; i++)
	{
		_ringBufferBPM[i].bpm = 0;
	}
}

uint16_t BeatDetector::AddElementToFilter(uint16_t bpm)
{
    _currentNode->bpm = bpm;
    _currentNode = _currentNode->pNext;

    float32_t sum = 0.0f;

	// Stores the start index
	RingBufferNodeBPM* pBufferBpm = _currentNode;

	// Move the pointer to a previous position depending on the order of the filter
	for(uint16_t i = 0; i < FILTER_ORDER_MAX; i++)
	{
		pBufferBpm = pBufferBpm->pPrev;
	}

	// Calculate the sum of the upcoming values (depending on the order of the filter)
	for(uint16_t i = 0; i < FILTER_ORDER_MAX; i++)
	{
		sum += pBufferBpm->bpm;
		pBufferBpm = pBufferBpm->pNext;
	}

	return (uint16_t)(sum / (float32_t)(FILTER_ORDER_MAX));
}

// BPM of 0 means no result
uint16_t BeatDetector::CalculateBeatsPerMinute(FFT_Result* freqEnergies)
{   
    _correlation.AddFrequencyEnergy(freqEnergies, MAX_FFT_RESULT_INDEX);

    if(!_correlation.IsBufferFull())
        return 0;

    int16_t maxLagsIndex = 0;
    float32_t* lagArray = _correlation.Autocorrelate(&maxLagsIndex);

    // Not enough samples yet and skip lag value of 0 (= lag of 1)
    if(maxLagsIndex < 2)
        return 0;

    float32_t maxLagValue = -100.0f;
    uint16_t maxLagIndex = 0;

    for(uint16_t i = 1; i < maxLagsIndex; i++)
    {
        if(lagArray[i] > maxLagValue)
        {
            maxLagValue = lagArray[i];
            maxLagIndex = i;
        }
    }

    // lagIndex + 1 is the lag
    uint strongestBpm = 0;
    
    if(maxLagValue > 0.0f)
    {
        strongestBpm = (uint16_t)((60.0f * (float32_t)(_sampleFrequency)) / (((float32_t)maxLagIndex + 1.0) * ((float32_t)_sampleCount)));
        strongestBpm = AddElementToFilter(strongestBpm);

        sprintf(_transmitBuffer, "Strongest BPM Value: %u, Data Count in Buffer: %u, Max Lag at %u with value %f\n", 
            strongestBpm, _correlation.GetDataCount(), maxLagIndex + 1, maxLagValue);

        // _uartCtrl->Transmit((uint8_t*)_transmitBuffer);
    }

    _correlation.ClearRingBuffer();

    return strongestBpm;
}