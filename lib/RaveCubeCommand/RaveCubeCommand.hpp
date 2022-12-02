#ifndef INC_RAVECUBECOMMAND_H_
#define INC_RAVECUBECOMMAND_H_

#include <UartController.hpp>
#include <I2CController.hpp>
#include <stdio.h>
#include <cstring>
#include <cstdio>

// static const char* _commandPalette[] = {"filterc", "filterv", "fboundaries", "bfactors", "led_status", "gamma_status", "sdata", "settings"};

// enum RaveCubeCommandEnum
// {
// 	ColorFilter = 0,
// 	VoltageFilter = 1,
// 	FrequencyBoundaries = 2,
// 	BrigthnessFactors = 3,
// 	LEDStatus = 4,
// 	GammaStatus = 5,
// 	StreamData = 6,
// 	Settings = 7,
// };

enum CommandStatus
{
    Valid,
    WrongCommandString,
    WrongParamCount,
    WrongParamValue
};

class RaveCubeCommand
{
protected:
    UartController* _uartCtrl;
    I2CController* _i2cCtrl;

    virtual void SetDefaultValues();
    virtual uint8_t* ToBinary(uint16_t* dataLength);
    virtual void ToStruct(uint8_t* dataArray);

    void SendCommandResponse(CommandStatus errorStatus);

public:
    RaveCubeCommand(UartController* uartCtrl, I2CController* i2cController)
    {
        _uartCtrl = uartCtrl;
        _i2cCtrl = i2cController;
    };

    ~RaveCubeCommand();

    virtual CommandStatus Parse(char* str);
    virtual void Save();
    virtual void Load();

    static uint32_t NextPowerOfTwo(uint32_t x);
};

#endif