#ifndef FILTER_LEVELS_HPP
#define FILTER_LEVELS_HPP

#include <RaveCubeSetting.hpp>
#include <FilterLevelsColor.hpp>
#include <FilterLevelsVoltage.hpp>

class FilterLevels : public RaveCubeSetting
{
	public:
		FilterLevelsColor Color;
		FilterLevelsVoltage Voltage;

		FilterLevels();
		~FilterLevels();

		bool operator==(FilterLevels levels2)
		{
			if(levels2.Color == Color && levels2.Voltage == Voltage)
				return true;

			return false;
		}

		bool IsDataValid();
		void ToString(char* strBuffer);

};

#endif