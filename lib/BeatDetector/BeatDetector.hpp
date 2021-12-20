
#ifndef _BEAT_DETECTOR_HPP
#define _BEAT_DETECTOR_HPP

#include <Autocorrelation.hpp>

class BeatDetector
{
    public:
        BeatDetector();
        ~BeatDetector();

    private:
        Autocorrelation _correlation = Autocorrelation();
};


#endif
