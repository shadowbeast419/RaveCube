
#ifndef SRC_COLORFILTERCOMMAND_H_
#define SRC_COLORFILTERCOMMAND_H_

#include <FactorCommand.hpp>

bool FactorCommand::IsDataValid(BrightnessFactors brightnessFactors)
{
    float32_t factorMin = 0.01f;
    float32_t factorMax = 100.0f;

    if(brightnessFactors.Red >= factorMin && brightnessFactors.Green >= factorMin && 
        brightnessFactors.Blue >= factorMin && brightnessFactors.All >= factorMin && 
        brightnessFactors.Red <= factorMax && brightnessFactors.Green <= factorMax &&
        brightnessFactors.Blue <= factorMax && brightnessFactors.All <= factorMax)
    {
        return true;
    }

    return false;
}

void FactorCommand::SetDefaultValues()
{
    _brightnessFactors.Red = BRIGHTNESS_FACTOR_RED_INIT;
    _brightnessFactors.Green = BRIGHTNESS_FACTOR_GREEN_INIT;
    _brightnessFactors.Blue = BRIGHTNESS_FACTOR_BLUE_INIT;
}

CommandStatus FactorCommand::Parse(char* str)
{
    // Wrong Command
    if( strstr((char*)str, _cmdStr) == NULL )
        return WrongCommandString;

	char* pSplittedStr = strtok((char*)str, " ");
	BrightnessFactors factors = {0.0f , 0.0f , 0.0f};

    uint8_t i;

	for(i = 0; pSplittedStr != NULL; i++)
	{
		switch(i)
		{
		case 1:
			factors.Red = (float)atof(pSplittedStr);

			break;
		case 2:
			factors.Green = (float)atof(pSplittedStr);

			break;
		case 3:
			factors.Blue = (float)atof(pSplittedStr);
        
        case 4:
            factors.All = (float)atof(pSplittedStr);
		}

		pSplittedStr = strtok(NULL, " ");
	}

    if(i != _numberOfParams)
    {
        return WrongParamCount;
    }

    // Check if values are valid
    if(IsDataValid(factors))
	{
        _brightnessFactors = factors;
        _dataValid = true;
	}

    _dataValid = false;
    return WrongParamValue;
}

uint8_t* FactorCommand::ToBinary(uint16_t* dataLength)
{ 
    *dataLength = sizeof(BrightnessFactors);
    memcpy(_binaryData, &_brightnessFactors, *dataLength);

    return _binaryData;
}

void FactorCommand::ToStruct(uint8_t* dataArray)
{
    uint16_t dataLength = sizeof(FilterLevels);
	BrightnessFactors factors = {0};

	memcpy(&factors, dataArray, dataLength);

	// Error, fallback to default values
	if(!IsDataValid(factors))
	{
		SetDefaultValues();
		return;
	}

	_brightnessFactors = factors;
}

void FactorCommand::Save()
{
    uint16_t dataLength = 0;
    uint8_t* dataArray;

    if(_dataValid)
    {
        _ledController->SetBrightnessFactors(_brightnessFactors);
        dataArray = ToBinary(&dataLength);

        _i2cCtrl->WriteDataEEPROM(dataArray, dataLength, _eepromSection);
    }
}

void FactorCommand::Load()
{
	uint16_t dataLength = sizeof(BrightnessFactors);

	_i2cCtrl->ReadDataEEPROM(_binaryData, dataLength, _eepromSection);
	ToStruct(_binaryData);
}

FactorCommand::~FactorCommand()
{

}

#endif