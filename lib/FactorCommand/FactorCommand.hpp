#ifndef INC_FACTORCOMMAND_HPP_
#define INC_FACTORCOMMAND_HPP_

#include <RaveCubeCommand.hpp>
#include <BrightnessFactors.hpp>
#include <LedController.hpp>

/// @brief Params: Red Green Blue All
class FactorCommand : public RaveCubeCommand
{
private:
    const char*         _cmdStr = "factor";
    const uint8_t       _eepromSection = 2;
    uint8_t             _numberOfParams = 4;

    LedController*      _ledController;       
    BrightnessFactors   _brightnessFactors = BrightnessFactors();

    // 5 times uint16_t -> 10 bytes (20 if there is space in between)
    uint8_t             _binaryData[40];


    char*               ToString();

public:
    FactorCommand(UartController* uartCtrl, I2CController* i2cCtrl, LedController* ledController);
    ~FactorCommand();

    CommandStatus Parse(char* str);

    /// @brief Sets the appropriate parameters in the specific classes and stores data to EEPROM
    void Save();

    /// @brief Loads data from EEPROM
    void Load();

    void SendCommandResponse(CommandStatus errorStatus);
};

#endif