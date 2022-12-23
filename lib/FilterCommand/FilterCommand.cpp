

#include <FilterCommand.hpp>

FilterCommand::FilterCommand(UartController* uartCtrl, I2CController* i2cCtrl, MovingAvgFilter* movingAvgFilter) : RaveCubeCommand(uartCtrl, i2cCtrl)
{
	this->_movingAvgFilter = movingAvgFilter;
}

CommandStatus FilterCommand::Parse(char* str)
{
    // Wrong Command
    if( strstr((char*)str, _cmdStr) == NULL )
        return WrongCommandString;

    FilterLevels filterLevels = FilterLevels();
    char* pSplittedStr = strtok((char*)str, " ");

    uint8_t i;

    // Split up the cmd string
    for(i = 0; pSplittedStr != NULL; i++)
	{
		if(i == 1)
		{
			filterLevels.Color.RedBrightness = atoi(pSplittedStr);
		}

		if(i == 2)
		{
			filterLevels.Color.GreenBrightness = atoi(pSplittedStr);
		}

		if(i == 3)
		{
			filterLevels.Color.BlueBrightness = atoi(pSplittedStr);
		}

		if(i == 4)
		{
			filterLevels.Voltage.Voltage = atoi(pSplittedStr);
		}

		if(i == 5)
		{
			filterLevels.Voltage.PeakVoltage = atoi(pSplittedStr);
		}

		pSplittedStr = strtok(NULL, " ");
	}

	// No Parameters -> Print command output
	if(i == 1)
		return Read;

    if(i != _numberOfParams + 1)
        return WrongParamCount;

	if(filterLevels.IsDataValid())
	{
		_filterLevels = filterLevels;
        return Valid;
	}

    return WrongParamValue;
}

void FilterCommand::Save()
{
    uint16_t dataLength = 0;
	uint8_t receiveBuffer[dataLength];

	_movingAvgFilter->SetFilterOrder(_filterLevels);

	// Convert to binary and save
	dataLength = DataToBinary<FilterLevels>(_filterLevels, _binaryData);
	_i2cCtrl->WriteDataEEPROM(_binaryData, dataLength, _eepromSection);

	// Check if data has been written properly
	_i2cCtrl->ReadDataEEPROM(receiveBuffer, dataLength, _eepromSection);
	FilterLevels filterLevelsEEPROM = BinaryToData<FilterLevels>(_binaryData);

	if(filterLevelsEEPROM == _filterLevels)
	{
		_uartCtrl->Transmit((uint8_t*)"FilterLevels successfully written to EEPROM");
		_filterLevels.ToString(_toStringBuffer);
		_uartCtrl->Transmit((uint8_t*) _toStringBuffer); 
	}
	else
	{
		// Data written/read from EEPROM is not valid
		
	}
}

void FilterCommand::Load()
{
	uint16_t dataLength = sizeof(FilterLevels);

	_i2cCtrl->ReadDataEEPROM(_binaryData, dataLength, _eepromSection);
	FilterLevels filterLevels = BinaryToData<FilterLevels>(_binaryData);

	if(filterLevels.IsDataValid())
	{
		_movingAvgFilter->SetFilterOrder(filterLevels);
		_filterLevels = filterLevels;

		_uartCtrl->Transmit((uint8_t*)"FilterLevels loaded successfully from EEPROM");

		return;
	}

	// Data is not valid -> Write default values to EEPROM
	_uartCtrl->Transmit((uint8_t*)"FilterLevels loaded from EEPROM not Valid");

	filterLevels.ToString(_toStringBuffer);
	_uartCtrl->Transmit((uint8_t*) _toStringBuffer); 
}

void FilterCommand::SendCommandResponse(CommandStatus errorStatus)
{
    switch(errorStatus)
    {
        case Valid:
			sprintf(_messageBuffer, "Valid Filter Command \n");
            break;

        // case WrongCommandString:
		// 	sprintf(messageBuffer, "Valid Filter Command");
        //     _uartCtrl->Transmit((uint8_t*)"Wrong Command String");
        //     break;

        case WrongParamCount:
            sprintf(_messageBuffer, "Wrong Parameter Count, Expected %i \n", _numberOfParams);
            break;

        case WrongParamValue:
			sprintf(_messageBuffer, "Wrong Parameter Value \n");
            break;
		
		case Read:
			Print();
			return;

		case WrongCommandString:
            return;
    }

	_uartCtrl->Transmit((uint8_t*)_messageBuffer);
}

char* FilterCommand::ToString()
{
	_filterLevels.ToString(_toStringBuffer);
	return _toStringBuffer;
}

FilterCommand::~FilterCommand()
{
	
}