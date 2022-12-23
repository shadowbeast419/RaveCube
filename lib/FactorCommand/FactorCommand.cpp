
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
    uint32_t dataLength = 0;

    _ledController->SetBrightnessFactors(_brightnessFactors);

    dataLength = DataToBinary<BrightnessFactors>(_brightnessFactors, _binaryData);
    _i2cCtrl->WriteDataEEPROM(_binaryData, dataLength, _eepromSection);

}

void FactorCommand::Load()
{
	uint16_t dataLength = sizeof(BrightnessFactors);

	_i2cCtrl->ReadDataEEPROM(_binaryData, dataLength, _eepromSection);
	_brightnessFactors = BinaryToData<BrightnessFactors>(_binaryData);

    _ledController->SetBrightnessFactors(_brightnessFactors);
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