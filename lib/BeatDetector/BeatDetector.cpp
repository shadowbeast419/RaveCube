
#include <BeatDetector.hpp>

BeatDetector::BeatDetector(SettingsController* settingsCtrl, UartController* uartCtrl)
{
    _settingsCtrl = settingsCtrl;
    _uartCtrl = uartCtrl;
    _correlation.Init(_settingsCtrl);
}


// BPM of 0 means no result
uint16_t BeatDetector::CalculateBeatsPerMinute(FFT_Result* freqEnergies)
{   
    _correlation.AddFrequencyEnergy(freqEnergies, MAX_FFT_RESULT_INDEX);

    int16_t maxLagsIndex = 0;
    float32_t* lagArray = _correlation.Autocorrelate(&maxLagsIndex);

    if(maxLagsIndex == -1)
        return 0;

    float32_t maxLagValue = 0.0f;
    uint16_t maxLagIndex = 0;

    for(uint16_t i = 0; i < maxLagsIndex; i++)
    {
        if(lagArray[i] > maxLagValue)
        {
            maxLagValue = lagArray[i];
            maxLagIndex = i;
        }
    }

    // lagIndex + 1 is the lag
    uint16_t strongestBpm = (uint16_t)((maxLagIndex + 1.0f) * (1.0f / ((float32_t)SAMPLE_FREQ)));
    
    char transmitBuffer[50];
    sprintf(transmitBuffer, "Strongest BPM Count: %i", strongestBpm);
    _uartCtrl->Transmit((uint8_t*)transmitBuffer);

    return strongestBpm;
}