
#ifndef _BEAT_DETECTOR_HPP
#define _BEAT_DETECTOR_HPP

#include <UartController.hpp>
#include <Autocorrelation.hpp>
#include <SettingsController.hpp>
#include <FFT.hpp>

class BeatDetector
{
    public:
        BeatDetector(SettingsController* settingsCtrl, UartController* uartCtrl);
        ~BeatDetector();

        uint16_t CalculateBeatsPerMinute(FFT_Result* freqEnergies);

    private:
        Autocorrelation                 _correlation = Autocorrelation();
        SettingsController*             _settingsCtrl = NULL;
        UartController*                 _uartCtrl = NULL;
};

#endif
