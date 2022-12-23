#include <FilterLevels.hpp>

FilterLevels::FilterLevels()
{

}

bool FilterLevels::IsDataValid()
{
    return Color.IsDataValid() && Voltage.IsDataValid();
}

void FilterLevels::ToString(char* strBuffer)
{
    sprintf(strBuffer, "Red %i, Green %i, Blue %i, Voltage %i, PeakVoltage %i \n", Color.RedBrightness, 
        Color.GreenBrightness, Color.BlueBrightness, Voltage.Voltage, Voltage.PeakVoltage);
}

FilterLevels::~FilterLevels()
{

}