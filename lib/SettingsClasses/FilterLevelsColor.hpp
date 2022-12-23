#ifndef FILTER_LEVELS_COLOR_HPP
#define FILTER_LEVELS_COLOR_HPP

#include <stdio.h>

class FilterLevelsColor
{
	public:
		static const uint16_t FilterLevelsMin = 3;
		static const uint16_t FilterLevelsMax = 150;
		static const uint16_t FilterLevelsInit = 30;

		uint16_t RedBrightness = FilterLevelsInit;
		uint16_t GreenBrightness = FilterLevelsInit;
		uint16_t BlueBrightness = FilterLevelsInit;
		
        bool IsDataValid();

		bool operator==(FilterLevelsColor levels2)
		{
			if(levels2.RedBrightness == RedBrightness && levels2.GreenBrightness == GreenBrightness && levels2.BlueBrightness == BlueBrightness)
				return true;

			return false;
		}
};

#endif