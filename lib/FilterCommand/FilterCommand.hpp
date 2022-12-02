#ifndef INC_FILTERCOMMAND_HPP_
#define INC_FILTERCOMMAND_HPP_

#include <RaveCubeCommand.hpp>
#include <MovingAvgFilter.hpp>

/// @brief Params: Red Green Blue Voltage PeakVoltage
class FilterCommand : public RaveCubeCommand
{
private:
    const char*         _cmdStr = "filter";
    const uint8_t       _eepromSection = 1;
    uint8_t             _numberOfParams = 5;

    FilterLevels        _filterLevels = {0};
    MovingAvgFilter*    _movingAvgFilter;
    bool                _dataValid = false;

    // 5 times uint16_t -> 10 bytes (20 if there is space in between)
    uint8_t             _binaryData[20];

    bool                IsDataValid(FilterLevels filterLevels);
    void                SetDefaultValues();

public:
    FilterCommand(UartController* uartCtrl, I2CController* i2cCtrl, MovingAvgFilter* movingAvgFilter) : RaveCubeCommand(uartCtrl, i2cCtrl)
    {
        this->_movingAvgFilter = movingAvgFilter;
	    SetDefaultValues();
    };

    ~FilterCommand();

    CommandStatus Parse(char* str);

    /// @brief 
    /// @param dataLength Length of the returned DataArray in Bytes
    /// @return 
    uint8_t* ToBinary(uint16_t* dataLength);

    /// @brief Transforms the binary data to the original data struct
    void ToStruct(uint8_t* dataArray);

    /// @brief Sets the appropriate parameters in the specific classes and stores data to EEPROM
    void Save();

    /// @brief Loads data from EEPROM
    void Load();
};

#endif