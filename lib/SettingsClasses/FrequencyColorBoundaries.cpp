
#include <FrequencyColorBoundaries.hpp>

FrequencyColorBoundaries::FrequencyColorBoundaries() : RaveCubeSetting()
{

}

bool FrequencyColorBoundaries::IsDataValid()
{
    return Red.IsDataValid() && Green.IsDataValid() && Blue.IsDataValid();
}

void FrequencyColorBoundaries::ToString(char* strBuffer)
{
    sprintf(strBuffer, "Red %i %i, Green %i %i, Blue %i %i \n", Red.Min, Red.Max, 
        Green.Min, Green.Max, Blue.Min, Blue.Max);
}

FrequencyColorBoundaries::~FrequencyColorBoundaries()
{

}