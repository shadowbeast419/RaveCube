
#include <BoundaryCommand.hpp>

BoundaryCommand::BoundaryCommand(UartController* uartCtrl, I2CController* i2cCtrl, LedController* ledController) : RaveCubeCommand(uartCtrl, i2cCtrl)
{
	_ledController = ledController;
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

	if(i == 1)
		return Read;

    if(i != _numberOfParams + 1)
        return WrongParamCount;

    // Check if values are valid
    if(colorBoundaries.IsDataValid())
	{
        _colorBoundaries = colorBoundaries;
	}

    return WrongParamValue;
}

/*

    uint32_t dataLength = sizeof(BrightnessFactors);
    uint8_t receiveBuffer[dataLength];

    _ledController->SetBrightnessFactors(_brightnessFactors);

    dataLength = DataToBinary<BrightnessFactors>(_brightnessFactors, _binaryData);
    _i2cCtrl->WriteDataEEPROM(_binaryData, dataLength, _eepromSection);

	// Check if data has been written properly
	_i2cCtrl->ReadDataEEPROM(receiveBuffer, dataLength, _eepromSection);
	BrightnessFactors brightnessFactorsEEPROM = BinaryToData<BrightnessFactors>(_binaryData);

	if(brightnessFactorsEEPROM == _brightnessFactors)
	{
		_uartCtrl->Transmit((uint8_t*)"BrightnessFactors successfully written to EEPROM");
		_brightnessFactors.ToString(_toStringBuffer);
		_uartCtrl->Transmit((uint8_t*) _toStringBuffer);

		return; 
	}

*/

void BoundaryCommand::Save()
{
    uint16_t dataLength = sizeof(FrequencyColorBoundaries);
	uint8_t receiveBuffer[dataLength];

	_ledController->SetColorBoundaries(_colorBoundaries);

	dataLength = DataToBinary<FrequencyColorBoundaries>(_colorBoundaries, _binaryData);
	_i2cCtrl->WriteDataEEPROM(_binaryData, dataLength, _eepromSection);

	_i2cCtrl->ReadDataEEPROM(receiveBuffer, dataLength, _eepromSection);
	FrequencyColorBoundaries boundariesEEPROM = BinaryToData<FrequencyColorBoundaries>(_binaryData);

	if(boundariesEEPROM == _colorBoundaries)
	{
		_uartCtrl->Transmit((uint8_t*)"BrightnessFactors successfully written to EEPROM");
		boundariesEEPROM.ToString(_toStringBuffer);
		_uartCtrl->Transmit((uint8_t*) _toStringBuffer);

		return; 
	}
}

void BoundaryCommand::Load()
{
	uint16_t dataLength = sizeof(FrequencyColorBoundaries);

	_i2cCtrl->ReadDataEEPROM(_binaryData, dataLength, _eepromSection);
	FrequencyColorBoundaries boundariesEEPROM = BinaryToData<FrequencyColorBoundaries>(_binaryData);

	if(boundariesEEPROM.IsDataValid())
	{
		_colorBoundaries = boundariesEEPROM;
		_ledController->SetColorBoundaries(boundariesEEPROM);

		_uartCtrl->Transmit((uint8_t*)"ColorBoundaries loaded successfully from EEPROM");
		
		return;
	}

    _uartCtrl->Transmit((uint8_t*) "ColorBoundaries loaded from EEPROM not valid");
    boundariesEEPROM.ToString(_toStringBuffer);
    _uartCtrl->Transmit((uint8_t*)_toStringBuffer);
}

void BoundaryCommand::SendCommandResponse(CommandStatus errorStatus)
{
    switch(errorStatus)
    {
        case Valid:
			sprintf(_messageBuffer, "Valid Boundary Command \n");
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

char* BoundaryCommand::ToString()
{
	_colorBoundaries.ToString(_toStringBuffer);
	return _toStringBuffer;
}

BoundaryCommand::~BoundaryCommand()
{
	
}