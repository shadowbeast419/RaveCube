#include <FrequencyBoundary.hpp>

FrequencyBoundary::FrequencyBoundary()
{
    Min = 0;
    Max = 0;
}

FrequencyBoundary::FrequencyBoundary(uint16_t min, uint16_t max)
{
    Min = min;
    Max = max;
}

bool FrequencyBoundary::IsDataValid()
{
    if(Min >= 0 && Max >= 0 && Min < Max)
    {
        return true;
    }

    return false;
}