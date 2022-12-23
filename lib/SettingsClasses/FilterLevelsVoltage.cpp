#include <FilterLevelsVoltage.hpp>

bool FilterLevelsVoltage::IsDataValid()
{
    if(Voltage >= FilterLevelsMin && PeakVoltage >= FilterLevelsMin && Voltage <= FilterLevelsMax && PeakVoltage <= FilterLevelsMax)
        return true;
    
    return false;
}