#ifndef FILTER_LEVELS_VOLTAGE_HPP
#define FILTER_LEVELS_VOLTAGE_HPP

#include <stdio.h>

class FilterLevelsVoltage
{
	public:
		static const uint16_t FilterLevelsMin = 3;
		static const uint16_t FilterLevelsMax = 150;
		static const uint16_t FilterLevelVoltageInit = FilterLevelsMax - 1;
		static const uint16_t FilterLevelPeakVoltageInit = 100;

		uint16_t Voltage = FilterLevelVoltageInit;
		uint16_t PeakVoltage = FilterLevelPeakVoltageInit;

		bool operator==(FilterLevelsVoltage levels2)
		{
			if(levels2.Voltage == Voltage && levels2.PeakVoltage == PeakVoltage)
				return true;

			return false;
		}

		bool IsDataValid();
};

#endif