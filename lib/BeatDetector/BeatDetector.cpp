

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
    _periodOfLag = (float32_t)_sampleCount / (float32_t)_sampleFrequency;   // Duration of one "Lag" value

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

    CorrelationResult coeffs = {0};

	for(uint16_t i = 0; i < FILTER_ORDER_MAX; i++)
	{
		_ringBufferBPM[i].result = coeffs;
	}
}

// Adds the Calculated BPM to the MovingAverageFilter
// Returns the weighted average of the BPM's for each color
CorrelationResult BeatDetector::AddElementToFilter(CorrelationResult corrResult)
{
    _currentNode->result = corrResult;

    float32_t sumBpm = 0.0f;
    float32_t sumLagValue = 0.0f;
    float32_t weight = 2.0f / (((float32_t)FILTER_ORDER_MAX) * ((float32_t)(FILTER_ORDER_MAX) + 1.0f));
    CorrelationResult filteredResult = {0};

	// Stores the start index
	RingBufferNodeBPM* pBufferBpm = _currentNode;
    RingBufferNodeBPM* pBufferFirst = NULL;

    // Move the pointer to a previous position depending on the order of the filter
	for(uint16_t i = 0; i < FILTER_ORDER_MAX; i++)
	{
		pBufferBpm = pBufferBpm->pPrev;
	}

    pBufferFirst = pBufferBpm;

	// Calculate the sum of the previous values (depending on the order of the filter)
    // Red bpm
	for(uint16_t i = 0; i < FILTER_ORDER_MAX; i++)
	{
        sumBpm += (pBufferBpm->result.Bpm.Red * ((float32_t)i + 1.0f));
        sumLagValue += (pBufferBpm->result.CorrCoeff.Red * ((float32_t)i + 1.0f));
		pBufferBpm = pBufferBpm->pNext;
	}

    filteredResult.Bpm.Red = (uint16_t)(sumBpm * weight);
    filteredResult.CorrCoeff.Red = sumLagValue * weight;

    sumBpm = 0.0f;
    sumLagValue = 0.0f;
    pBufferBpm = pBufferFirst;

    // Green bpm
    for(uint16_t i = 0; i < FILTER_ORDER_MAX; i++)
	{
        sumBpm += (pBufferBpm->result.Bpm.Green * ((float32_t)i + 1.0f));
        sumLagValue += (pBufferBpm->result.CorrCoeff.Green * ((float32_t)i + 1.0f));
		pBufferBpm = pBufferBpm->pNext;
	}

    filteredResult.Bpm.Green = (uint16_t)(sumBpm * weight);
    filteredResult.CorrCoeff.Green = sumLagValue * weight;

    sumBpm = 0.0f;
    sumLagValue = 0.0f;
    pBufferBpm = pBufferFirst;

    // Blue bpm
    for(uint16_t i = 0; i < FILTER_ORDER_MAX; i++)
	{
        sumBpm += (pBufferBpm->result.Bpm.Blue * ((float32_t)i + 1.0f));
        sumLagValue += (pBufferBpm->result.CorrCoeff.Blue * ((float32_t)i + 1.0f));
		pBufferBpm = pBufferBpm->pNext;
	}

    filteredResult.Bpm.Blue = (uint16_t)(sumBpm * weight);
    filteredResult.CorrCoeff.Blue = sumLagValue * weight;

    _currentNode = _currentNode->pNext;

    // Return the weighted averages
	return filteredResult;
}

// BPM of 0 means no result
CorrelationResult BeatDetector::CalculateBeatsPerMinute(RgbLedBrightness rgbBrightness)
{
    // Adds new FFT Samples to the RingBuffer   
    _correlation.AddRgbBrightness(rgbBrightness);

    CorrelationResult corrResult = {0};
    BeatsPerMinute bpm = {0};

    if(!_correlation.IsBufferFull())
        return corrResult;

    uint16_t maxLagIndex = 0;
    float32_t maxLagValue = 0.0f;

    // BPM Calculations for Red color
    CalculateMaxLagValues(Red, &maxLagIndex, &maxLagValue);
    bpm.Red = CalculateBpmFromLagValue(maxLagIndex);
    corrResult.CorrCoeff.Red = maxLagValue;

    // BPM Calculations for Green color
    CalculateMaxLagValues(Green, &maxLagIndex, &maxLagValue);
    bpm.Green = CalculateBpmFromLagValue(maxLagIndex);
    corrResult.CorrCoeff.Green = maxLagValue;

    // BPM Calculations for Blue color
    CalculateMaxLagValues(Blue, &maxLagIndex, &maxLagValue);
    bpm.Blue = CalculateBpmFromLagValue(maxLagIndex);
    corrResult.CorrCoeff.Blue = maxLagValue;

    corrResult.Bpm = bpm;

    CorrelationResult filteredResult = AddElementToFilter(corrResult);

    if(EnableOutputToUart)
    {
        WriteResultToUart(Red, filteredResult);
        while(_uartCtrl->IsTxBusy());

        WriteResultToUart(Green, filteredResult);
        while(_uartCtrl->IsTxBusy());

        WriteResultToUart(Blue, filteredResult);
        while(_uartCtrl->IsTxBusy());
    }

    _correlation.ClearRingBuffer();

    return filteredResult;
}

void BeatDetector::CalculateMaxLagValues(ColorSelection color, uint16_t* maxLag, float32_t* maxLagValue)
{
    int16_t maxCorrelationIndex = 0;
    float32_t* lagArray = NULL;

    lagArray = _correlation.Autocorrelate(&maxCorrelationIndex, color);

    // Certain amount of data is needed for proper correlation.
    if(maxCorrelationIndex == -1 || maxCorrelationIndex < _correlation.MinLag * 2)
    {
        *maxLag = -1;
        *maxLagValue = _correlation.InvalidLagValue;
        return;
    }

    *maxLagValue = _correlation.InvalidLagValue;
    *maxLag = 0;
    float32_t absValue = 0.0f;
    uint16_t lagArrayIndex = 0;

    // Skip the lags smaller than minLag
    for(uint16_t lag = _correlation.MinLag; lag < maxCorrelationIndex; lag++)
    {
        lagArrayIndex = lag - _correlation.MinLag;

        // Skip the lags higher than maxLag
        if(lag >= _correlation.MaxLag)
            break;

        absValue = UseAbsValueOfCorrelation ? fabs(lagArray[lagArrayIndex]) : lagArray[lagArrayIndex];

        // Ignore negative values
        absValue = absValue < 0.0f ? 0.0f : absValue;

        if(absValue != _correlation.InvalidLagValue && absValue > *maxLagValue)
        {
            *maxLagValue = absValue; 
            *maxLag = lag + 1;
        }
    }
}

uint16_t BeatDetector::CalculateBpmFromLagValue(uint16_t maxLag)
{
    return (uint16_t)(60.0f / (_periodOfLag * ((float32_t)maxLag)));
}

void BeatDetector::WriteResultToUart(ColorSelection color, CorrelationResult result)
{
    uint16_t bpm = 0;
    float32_t lagValue = 0.0f;
    char const* colorString = NULL;

    switch(color)
    {
        case Red:
            bpm = result.Bpm.Red;
            lagValue = result.CorrCoeff.Red;
            colorString = "Red";

            break;
        case Green:
            bpm = result.Bpm.Green;
            lagValue = result.CorrCoeff.Green;
            colorString = "Green";

            break;
        case Blue:
            bpm = result.Bpm.Blue;
            lagValue = result.CorrCoeff.Blue;
            colorString = "Blue";

            break;
    }

    sprintf(_transmitBuffer, "%s BPM Value: %u, Lag value: %f\n",
        colorString, bpm, lagValue);

    _uartCtrl->Transmit((uint8_t*)_transmitBuffer);
}

void BeatDetector::GetBpmBoundaries(uint16_t* minValue, uint16_t* maxValue)
{
    *maxValue = 60.0f / (_correlation.MinLag * _periodOfLag);
    *minValue = 60.0f / (_correlation.MaxLag * _periodOfLag);
}

bool BeatDetector::CalculateFilterLevels(CorrelationResult result, FilterLevels* filterLevels)
{
    if(result.Bpm.Red == 0 || result.Bpm.Green == 0 || result.Bpm.Blue == 0)
    {
        return false;
    }

    float32_t k = 0.0f;
    float32_t d = 0.0f;
    uint16_t bpmMin = 0;
    uint16_t bpmMax = 0;

    GetBpmBoundaries(&bpmMin, &bpmMax);

    k = ((float32_t)COLOR_FILTER_ORDER_MIN - (float32_t)COLOR_FILTER_ORDER_MAX) / ((float32_t)bpmMax - (float32_t)bpmMin);
    d = (float32_t)COLOR_FILTER_ORDER_MIN - ((float32_t)bpmMax * k);

    k = k * _bpmToFilterSensibility;
    d = d / _bpmToFilterSensibility;

    filterLevels->RedBrightness = (uint16_t)(k * (float32_t)result.Bpm.Red + d);
    filterLevels->GreenBrightness = (uint16_t)(k * (float32_t)result.Bpm.Green + d);
    filterLevels->BlueBrightness = (uint16_t)(k * (float32_t)result.Bpm.Blue + d);

    // Red color check
    if(filterLevels->RedBrightness < COLOR_FILTER_ORDER_MIN)
        filterLevels->RedBrightness = COLOR_FILTER_ORDER_MIN;

    if(filterLevels->RedBrightness >= COLOR_FILTER_ORDER_MAX)
        filterLevels->RedBrightness = COLOR_FILTER_ORDER_MAX - 1;

    // Green color check
    if(filterLevels->GreenBrightness < COLOR_FILTER_ORDER_MIN)
        filterLevels->GreenBrightness = COLOR_FILTER_ORDER_MIN;

    if(filterLevels->GreenBrightness >= COLOR_FILTER_ORDER_MAX)
        filterLevels->GreenBrightness = COLOR_FILTER_ORDER_MAX - 1;

    // Blue color check
    if(filterLevels->BlueBrightness < COLOR_FILTER_ORDER_MIN)
        filterLevels->BlueBrightness = COLOR_FILTER_ORDER_MIN;

    if(filterLevels->BlueBrightness >= COLOR_FILTER_ORDER_MAX)
        filterLevels->BlueBrightness = COLOR_FILTER_ORDER_MAX - 1;

    return true;
}