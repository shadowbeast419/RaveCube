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
    Read,
    WrongCommandString,
    WrongParamCount,
    WrongParamValue
};

class RaveCubeCommand
{
protected:
    UartController* _uartCtrl;
    I2CController* _i2cCtrl;
    
    char _messageBuffer[120];
    char _toStringBuffer[120];

    /// @brief Buffer for the binary data conversion
    uint8_t  _binaryData[40];

public:
    RaveCubeCommand(UartController* uartCtrl, I2CController* i2cController)
    {
        _uartCtrl = uartCtrl;
        _i2cCtrl = i2cController;
    }

    ~RaveCubeCommand();

    template <typename T> 
    T BinaryToData(uint8_t* binaryInput)
    {
        uint32_t dataLength = sizeof(T);
        T data;

        memset(&data, 0, (size_t)dataLength);
        memcpy(&data, binaryInput, dataLength);

        return data;
    }

    /// @brief Returns data in binary form
    /// @tparam T 
    /// @param dataStruct 
    /// @param binaryOutput 
    /// @return The length of the data, rounded up to the next power of 2
    template <typename T>
    uint32_t DataToBinary(T dataStruct, uint8_t* binaryOutput)
    {
        uint32_t dataLength = sizeof(T);

        memset(binaryOutput, 0, (size_t)dataLength);
        memcpy(binaryOutput, &dataStruct, (size_t)dataLength);

        return dataLength;
    }

    virtual char* ToString() = 0;
    virtual CommandStatus Parse(char* str) = 0;
    virtual void Save() = 0;
    virtual void Load() = 0;
    virtual void SendCommandResponse(CommandStatus errorStatus) = 0;

    void Print();

    static uint32_t NextPowerOfTwo(uint32_t x);
};

#endif