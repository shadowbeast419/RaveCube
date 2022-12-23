#include <BrightnessFactors.hpp>

bool BrightnessFactors::IsDataValid()
{
    if(Red >= 0.0f && Green >= 0.0f && Blue >= 0.0f && All >= 0.0f)
        return true;

    return false;
}

void BrightnessFactors::ToString(char* strBuffer)
{
    sprintf(strBuffer, "Red %f, Green %f, Blue %f, All %f \n", Red, Green, 
        Blue, All);
}