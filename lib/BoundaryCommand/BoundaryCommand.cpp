
#include <BoundaryCommand.hpp>

bool BoundaryCommand::IsDataValid(FrequencyColorBoundaries colorBoundaries)
{
	// Check if values are valid
    if((colorBoundaries.Red.Min < colorBoundaries.Red.Max) && (colorBoundaries.Green.Min < colorBoundaries.Green.Max) 
            && (colorBoundaries.Blue.Min < colorBoundaries.Blue.Max))
	{
		return true;
	}

	return false;
}

void BoundaryCommand::SetDefaultValues()
{
	_colorBoundaries.Red.Min = BOUNDARY_RED_MIN_INIT;
    _colorBoundaries.Red.Max = BOUNDARY_RED_MAX_INIT;
    _colorBoundaries.Green.Min = BOUNDARY_GREEN_MIN_INIT;
    _colorBoundaries.Green.Max = BOUNDARY_GREEN_MAX_INIT;
    _colorBoundaries.Blue.Min = BOUNDARY_BLUE_MIN_INIT;
    _colorBoundaries.Blue.Max = BOUNDARY_BLUE_MAX_INIT;
}

CommandStatus BoundaryCommand::Parse(char* str)
{
    // Wrong Command
    if( strstr((char*)str, _cmdStr) == NULL )
        return WrongCommandString;

	char* pSplittedStr = strtok(str, " ");
	FrequencyColorBoundaries colorBoundaries;

    uint8_t i;

	for(i = 0; pSplittedStr != NULL; i++)
	{
		switch(i)
		{
		case 1:
			colorBoundaries.Red.Min = atoi(pSplittedStr);

			break;
		case 2:
			colorBoundaries.Red.Max = atoi(pSplittedStr);

			break;
		case 3:
			colorBoundaries.Green.Min = atoi(pSplittedStr);

			break;
		case 4:
			colorBoundaries.Green.Max = atoi(pSplittedStr);

			break;
		case 5:
			colorBoundaries.Blue.Min = atoi(pSplittedStr);

			break;
		case 6:
			colorBoundaries.Blue.Max = atoi(pSplittedStr);

			break;
		}

		pSplittedStr = strtok(NULL, " ");
	}

    if(i != _numberOfParams)
    {
        return WrongParamCount;
    }

    // Check if values are valid
    if(IsDataValid(colorBoundaries))
	{
        _colorBoundaries = colorBoundaries;
        _dataValid = true;
	}

    _dataValid = false;

    return WrongParamValue;
}

uint8_t* BoundaryCommand::ToBinary(uint16_t* dataLength)
{ 
    *dataLength = sizeof(FrequencyColorBoundaries);
    memcpy(_binaryData, &_colorBoundaries, *dataLength);

    return _binaryData;
}

void BoundaryCommand::ToStruct(uint8_t* dataArray)
{
    uint16_t dataLength = sizeof(FrequencyColorBoundaries);
	FrequencyColorBoundaries boundaries = {0};

	memcpy(&boundaries, dataArray, dataLength);

	// Error, fallback to default values
	if(!IsDataValid(boundaries))
	{
		SetDefaultValues();
		return;
	}

	_colorBoundaries = boundaries;
}


void BoundaryCommand::Save()
{
    uint16_t dataLength = 0;
    uint8_t* dataArray;

    if(_dataValid)
    {
        _ledController->SetColorBoundaries(_colorBoundaries);
        dataArray = ToBinary(&dataLength);

        _i2cCtrl->WriteDataEEPROM(dataArray, dataLength, _eepromSection);
    }
}

void BoundaryCommand::Load()
{
	uint16_t dataLength = sizeof(FrequencyColorBoundaries);

	_i2cCtrl->ReadDataEEPROM(_binaryData, dataLength, _eepromSection);
	ToStruct(_binaryData);
}

BoundaryCommand::~BoundaryCommand()
{
	
}