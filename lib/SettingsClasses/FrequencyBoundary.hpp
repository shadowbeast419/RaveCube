#ifndef FREQUENCY_BOUNDARY_HPP
#define FREQUENCY_BOUNDARY_HPP

#include <stdio.h>

class FrequencyBoundary
{
	public:
		uint16_t Min;
		uint16_t Max;

		FrequencyBoundary();
		FrequencyBoundary(uint16_t min, uint16_t max);
        bool IsDataValid();

		bool operator==(FrequencyBoundary boundary2)
		{
			if(boundary2.Min == Min && boundary2.Max == Max)
				return true;
			
			return false;
		}
};

#endif