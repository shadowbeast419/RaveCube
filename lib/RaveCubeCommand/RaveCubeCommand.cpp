#include <RaveCubeCommand.hpp>

// RaveCubeCommand::RaveCubeCommand(UartController* uartCtrl, I2CController* i2cController)
// {
//     _uartCtrl = uartCtrl;
//     _i2cCtrl = i2cController;
// }

void RaveCubeCommand::Print()
{
	// ToString is overloaded by the command classes
	_uartCtrl->Transmit((uint8_t*) ToString());
}

// https://jameshfisher.com/2018/03/30/round-up-power-2/
// Rounds number up to next power of 2
uint32_t RaveCubeCommand::NextPowerOfTwo(uint32_t x) 
{ 	
	return x == 1 ? 1 : 1<<(32-__builtin_clz(x-1)); 
}

RaveCubeCommand::~RaveCubeCommand()
{

}
