#ifndef BRIGHTNESS_FACTORS_HPP
#define BRIGHTNESS_FACTORS_HPP

#include <stdio.h>
#include <arm_math.h>
#include <RaveCubeSetting.hpp>

class BrightnessFactors : public RaveCubeSetting
{
	public:
		float32_t Red = 3.25f;
		float32_t Green = 1.55f;
		float32_t Blue = 1.15f;
		float32_t All = 1.5f;

		bool operator==(BrightnessFactors factors2)
		{
			if(fabs(factors2.Red - Red) < 0.01f && fabs(factors2.Green - Green) < 0.01f && fabs(factors2.Blue - Blue) < 0.01f && fabs(factors2.All - All) < 0.01f) 
				return true;
			
			return false;
		}

		bool IsDataValid();
		void ToString(char* strBuffer);
};


#endif