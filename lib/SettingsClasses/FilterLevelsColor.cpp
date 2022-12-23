#include <FilterLevelsColor.hpp>

bool FilterLevelsColor::IsDataValid()
{
    if(RedBrightness >= FilterLevelsMin && GreenBrightness >= FilterLevelsMin && BlueBrightness >= FilterLevelsMin && 
    RedBrightness <= FilterLevelsMax && GreenBrightness <= FilterLevelsMax && BlueBrightness <= FilterLevelsMax)
    {
        return true;
    }

    return false;
}