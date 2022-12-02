

#include <FilterCommand.hpp>

void FilterCommand::SetDefaultValues()
{
	_filterLevels.RedBrightness = COLOR_FILTER_RED_ORDER_INIT;
	_filterLevels.GreenBrightness = COLOR_FILTER_GREEN_ORDER_INIT;
	_filterLevels.BlueBrightness = COLOR_FILTER_BLUE_ORDER_INIT;
	_filterLevels.Voltage = VOLTAGE_FILTER_ORDER_INIT;
	_filterLevels.PeakVoltage = PEAK_VOLTAGE_FILTER_ORDER_INIT;
}

bool FilterCommand::IsDataValid(FilterLevels filterLevels)
{
	    // Check if values are valid
    if((filterLevels.RedBrightness >= COLOR_FILTER_ORDER_MIN) && (filterLevels.RedBrightness < COLOR_FILTER_ORDER_MAX) &&
			(filterLevels.GreenBrightness >= COLOR_FILTER_ORDER_MIN) && (filterLevels.GreenBrightness < COLOR_FILTER_ORDER_MAX) &&
			(filterLevels.BlueBrightness >= COLOR_FILTER_ORDER_MIN) && (filterLevels.BlueBrightness < COLOR_FILTER_ORDER_MAX) &&
			(filterLevels.Voltage >= 3) && (filterLevels.Voltage <= VOLTAGE_FILTER_ORDER_MAX) &&
			(filterLevels.PeakVoltage >= 3) && (filterLevels.PeakVoltage <= PEAK_VOLTAGE_FILTER_ORDER_MAX))
	{
		return true;
	}

	return false;
}

CommandStatus FilterCommand::Parse(char* str)
{
    // Wrong Command
    if( strstr((char*)str, _cmdStr) == NULL )
        return WrongCommandString;

    FilterLevels filterLevels = {0};
    char* pSplittedStr = strtok((char*)str, " ");

    uint8_t i;

    // Split up the cmd string
    for(i = 0; pSplittedStr != NULL; i++)
	{
		if(i == 1)
		{
			filterLevels.RedBrightness = atoi(pSplittedStr);
		}

		if(i == 2)
		{
			filterLevels.GreenBrightness = atoi(pSplittedStr);
		}

		if(i == 3)
		{
			filterLevels.BlueBrightness = atoi(pSplittedStr);
		}

		if(i == 4)
		{
			filterLevels.Voltage = atoi(pSplittedStr);
		}

		if(i == 5)
		{
			filterLevels.PeakVoltage = atoi(pSplittedStr);
		}

		pSplittedStr = strtok(NULL, " ");
	}

    if(i != _numberOfParams)
    {
        return WrongParamCount;
    }

	if(IsDataValid(filterLevels))
	{
		_filterLevels = filterLevels;
        _dataValid = true;

        return Valid;
	}

    _dataValid = false;
    return WrongParamValue;
}

uint8_t* FilterCommand::ToBinary(uint16_t* dataLength)
{ 
    *dataLength = sizeof(FilterLevels);
    memcpy(_binaryData, &_filterLevels, *dataLength);

    return _binaryData;
}

void FilterCommand::ToStruct(uint8_t* dataArray)
{
	uint16_t dataLength = sizeof(FilterLevels);
	FilterLevels filterLevels = {0};

	memcpy(&filterLevels, dataArray, dataLength);

	// Error, fallback to default values
	if(!IsDataValid(filterLevels))
	{
		SetDefaultValues();
		return;
	}

	_filterLevels = filterLevels;
}

void FilterCommand::Save()
{
    uint16_t dataLength = 0;
    uint8_t* dataArray;

    if(_dataValid)
    {
        _movingAvgFilter->SetFilterOrder(_filterLevels);
        dataArray = ToBinary(&dataLength);

        _i2cCtrl->WriteDataEEPROM(dataArray, dataLength, _eepromSection);
    }
}

void FilterCommand::Load()
{
	uint16_t dataLength = sizeof(FilterLevels);

	_i2cCtrl->ReadDataEEPROM(_binaryData, dataLength, _eepromSection);
	ToStruct(_binaryData);
}

FilterCommand::~FilterCommand()
{
	
}