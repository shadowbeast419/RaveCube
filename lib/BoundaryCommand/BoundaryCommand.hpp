#ifndef INC_BOUNDARYCOMMAND_HPP_
#define INC_BOUNDARYCOMMAND_HPP_

#include <RaveCubeCommand.hpp>
#include <LedController.hpp>

/// @brief Params: RedMin RedMax GreenMin GreenMax BlueMin BlueMax
class BoundaryCommand : public RaveCubeCommand
{
private:
    const char*         _cmdStr = "boundary";
    const uint8_t       _eepromSection = 3;
    uint8_t             _numberOfParams = 6;

    LedController*             _ledController;       
    FrequencyColorBoundaries   _colorBoundaries = {0};
    bool                       _dataValid = false;

    // 5 times uint16_t -> 10 bytes (20 if there is space in between)
    uint8_t             _binaryData[40];

    bool                IsDataValid(FrequencyColorBoundaries brightnessFactors);
    void                SetDefaultValues();

public:
    BoundaryCommand(UartController* uartCtrl, I2CController* i2cCtrl, LedController* ledController) : RaveCubeCommand(uartCtrl, i2cCtrl)
    {
            _ledController = ledController;
	        SetDefaultValues();
    };
    
    ~BoundaryCommand();

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