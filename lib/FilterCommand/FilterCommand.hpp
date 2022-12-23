#ifndef INC_FILTERCOMMAND_HPP_
#define INC_FILTERCOMMAND_HPP_

#include <RaveCubeCommand.hpp>
#include <FilterLevels.hpp>
#include <MovingAvgFilter.hpp>

/// @brief Params: Red Green Blue Voltage PeakVoltage
class FilterCommand : public RaveCubeCommand
{
private:
    const char*         _cmdStr = "filter";
    const uint8_t       _eepromSection = 1;
    uint8_t             _numberOfParams = 5;

    FilterLevels        _filterLevels = FilterLevels();
    MovingAvgFilter*    _movingAvgFilter;

    // 5 times uint16_t -> 10 bytes (20 if there is space in between)
    uint8_t             _binaryData[20];

    char*               ToString();

public:
    FilterCommand(UartController* uartCtrl, I2CController* i2cCtrl, MovingAvgFilter* movingAvgFilter);
    ~FilterCommand();

    CommandStatus Parse(char* str);

    /// @brief Sets the appropriate parameters in the specific classes and stores data to EEPROM
    void Save();
    /// @brief Loads data from EEPROM
    void Load();

    void SendCommandResponse(CommandStatus errorStatus);

};

#endif