#ifndef INC_BOUNDARYCOMMAND_HPP_
#define INC_BOUNDARYCOMMAND_HPP_

#include <RaveCubeCommand.hpp>
#include <LedController.hpp>
#include <FrequencyColorBoundaries.hpp>

/// @brief Params: RedMin RedMax GreenMin GreenMax BlueMin BlueMax
class BoundaryCommand : public RaveCubeCommand
{
private:
    const char*         _cmdStr = "boundary";
    const uint8_t       _eepromSection = 3;
    uint8_t             _numberOfParams = 6;

    LedController*             _ledController;       
    FrequencyColorBoundaries   _colorBoundaries = FrequencyColorBoundaries();

    char*   ToString();

public:
    BoundaryCommand(UartController* uartCtrl, I2CController* i2cCtrl, LedController* ledController);
    ~BoundaryCommand();

    CommandStatus Parse(char* str);

    /// @brief Sets the appropriate parameters in the specific classes and stores data to EEPROM
    void Save();

    /// @brief Loads data from EEPROM
    void Load();

    void SendCommandResponse(CommandStatus errorStatus);
};

#endif