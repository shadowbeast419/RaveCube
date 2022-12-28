
#ifndef SRC_COLORFILTERCOMMAND_H_
#define SRC_COLORFILTERCOMMAND_H_

#include <FactorCommand.hpp>

FactorCommand::FactorCommand(UartController* uartCtrl, I2CController* i2cCtrl, LedController* ledController) : RaveCubeCommand(uartCtrl, i2cCtrl)
{
    _ledController = ledController;
}

CommandStatus FactorCommand::Parse(char* str)
{
    // Wrong Command
    if( strstr((char*)str, _cmdStr) == NULL )
        return WrongCommandString;

	char* pSplittedStr = strtok((char*)str, " ");
	BrightnessFactors factors = BrightnessFactors();

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

    if(i == 1)
        return Read;

    if(i != _numberOfParams + 1)
        return WrongParamCount;

    // Check if values are valid
    if(factors.IsDataValid())
	{
        _brightnessFactors = factors;
        return Valid;
	}

    return WrongParamValue;
}

void FactorCommand::Save()
{
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

	// Data written/read from EEPROM is not valid
	_uartCtrl->Transmit((uint8_t*)"BrightnessFactors write to EEPROM not successfull");
    brightnessFactorsEEPROM.ToString(_toStringBuffer);
    _uartCtrl->Transmit((uint8_t*) _toStringBuffer);
}

void FactorCommand::Load()
{
	uint16_t dataLength = sizeof(BrightnessFactors);

	_i2cCtrl->ReadDataEEPROM(_binaryData, dataLength, _eepromSection);
	BrightnessFactors brightnessFactorsEEPROM = BinaryToData<BrightnessFactors>(_binaryData);

    if(brightnessFactorsEEPROM.IsDataValid())
    {
        _ledController->SetBrightnessFactors(brightnessFactorsEEPROM);
        _brightnessFactors = brightnessFactorsEEPROM;

        _uartCtrl->Transmit((uint8_t*)"BrightnessFactors loaded successfully from EEPROM");

        return;
    }

    // Data not valid
    _uartCtrl->Transmit((uint8_t*) "BrightnessFactors loaded from EEPROM not valid");
    _brightnessFactors.ToString(_toStringBuffer);
    _uartCtrl->Transmit((uint8_t*)_toStringBuffer);
}


void FactorCommand::SendCommandResponse(CommandStatus errorStatus)
{
    switch(errorStatus)
    {
        case Valid:
			sprintf(_messageBuffer, "Valid Factor Command \n");
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

char* FactorCommand::ToString()
{
    _brightnessFactors.ToString(_toStringBuffer);
    return _toStringBuffer;
}

FactorCommand::~FactorCommand()
{

}

#endif