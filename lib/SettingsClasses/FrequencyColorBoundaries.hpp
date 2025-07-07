#ifndef FREQUENCY_COLOR_BOUNDARIES
#define FREQUENCY_COLOR_BOUNDARIES

#include <RaveCubeSetting.hpp>
#include <FrequencyBoundary.hpp>

class FrequencyColorBoundaries : public RaveCubeSetting
{
	public:
		FrequencyBoundary Red = FrequencyBoundary(0, 250);
		FrequencyBoundary Green = FrequencyBoundary(250, 2000);
		FrequencyBoundary Blue = FrequencyBoundary(2000, 6000);

		FrequencyColorBoundaries();
        ~FrequencyColorBoundaries();

		bool operator==(FrequencyColorBoundaries boundary2)
		{
			if(boundary2.Red == Red && boundary2.Green == Green && boundary2.Blue == Blue)
				return true;
			
			return false;
		}

		bool IsDataValid();
		void ToString(char* strBuffer);
};

#endif