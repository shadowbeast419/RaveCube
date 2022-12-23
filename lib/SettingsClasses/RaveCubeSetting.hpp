
#ifndef RAVECUBESETTING_HPP
#define RAVECUBESETTING_HPP

class RaveCubeSetting
{	
	public:
        RaveCubeSetting();
        ~RaveCubeSetting();

		virtual bool IsDataValid() = 0;
		virtual void ToString(char* strBuffer) = 0;
};

#endif