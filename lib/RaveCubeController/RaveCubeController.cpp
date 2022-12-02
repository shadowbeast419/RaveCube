/*
 * RaveCubeControler.cpp
 *
 *  Created on: 23 Nov 2020
 *      Author: Christoph
 */

#include <RaveCubeController.hpp>

RaveCubeController::RaveCubeController(LedController* ledCtrlInstance, UartController* uartCtrlInstance, SettingsController* settingCtrl)
{
	_ledCtrlInstance = ledCtrlInstance;
	_uartCtrlInstance = uartCtrlInstance;
	_settingsCtrlInstance = settingCtrl;
}

void RaveCubeController::ExecuteCommand(uint8_t* cmdStr)
{
	if( strstr((char*)cmdStr,"filter") != NULL )
	{
		// SYNTAX: filter [redBrightnessOrder greenBrightnessOrder blueBrightnessOrder voltageOrder peakVoltageOrder]
		ChangeFilterOrder(cmdStr);
	}
	else if( strstr((char*)cmdStr, "boundaries") != NULL )
	{
		// SYNTAX: boundaries [red_lower] [red_upper] [green_lower] [green_upper] [blue_lower] [blue_upper]
		ChangeBoundaries(cmdStr);
	}
	else if( strstr((char*)cmdStr, "leds") != NULL )
	{
		// SYNTAX: leds [1] or [0]
		ChangeLedStatus(cmdStr);
	}
	else if( strstr((char*)cmdStr, "gamma") != NULL )
	{
		// SYNTAX: gamma [1] or [0]
		ChangeGamma(cmdStr);
	}
	else if( strstr((char*)cmdStr, "factor") != NULL )
	{
		ChangeBrightnessFactors(cmdStr);
	}
	else if( strstr((char*)cmdStr, "data") != NULL )
	{
		// SYNTAX: brightnessOrder amplitudeOrder RedMin RedMax GreenMin GreenMax BlueMin BlueMax LedStatus GammaStatus
		SendData();
	}
	else if( strstr((char*)cmdStr, "streamb") != NULL )
	{
		// SYNTAX: streamf [1] or [0]
		ChangeStreamingBrightnessValuesEnable(cmdStr);
	}
	else if( strstr((char*)cmdStr, "streama") != NULL)
	{
		ChangeStreamingAmplitudeValuesEnable(cmdStr);
	}
	else if( strstr((char*)cmdStr, "amplitude") != NULL)
	{
		ChangeAmplitude(cmdStr);
	}
}

void RaveCubeController::ChangeStreamingBrightnessValuesEnable(uint8_t* cmdStr)
{
	char* pSplittedStr = strtok((char*)cmdStr, " ");
	bool streamingStatus = false;

	for(uint8_t i = 0; pSplittedStr != NULL; i++)
	{
		if(i == 1)
		{
			streamingStatus = atoi(pSplittedStr);
		}

		pSplittedStr = strtok(NULL, " ");
	}

	if(streamingStatus == SET || streamingStatus == RESET)
	{
		_streamBrightnessValuesEnable = streamingStatus;
	}
	else
	{
		_uartCtrlInstance->Transmit(_invalidString);
	}
}

void RaveCubeController::ChangeStreamingAmplitudeValuesEnable(uint8_t* cmdStr)
{
	char* pSplittedStr = strtok((char*)cmdStr, " ");
	uint8_t streamingStatus = RESET;

	for(uint8_t i = 0; pSplittedStr != NULL; i++)
	{
		if(i == 1)
		{
			streamingStatus = atoi(pSplittedStr);
		}

		pSplittedStr = strtok(NULL, " ");
	}

	if(streamingStatus == SET || streamingStatus == RESET)
	{
		_streamAmplitudeValuesEnable = streamingStatus;
	}
	else
	{
		_uartCtrlInstance->Transmit(_invalidString);
	}
}

void RaveCubeController::UpdateFilterOrders(FilterLevels orders, bool saveToEEPROM)
{
	if((orders.RedBrightness >= COLOR_FILTER_ORDER_MIN) && (orders.RedBrightness < COLOR_FILTER_ORDER_MAX) &&
			(orders.GreenBrightness >= COLOR_FILTER_ORDER_MIN) && (orders.GreenBrightness < COLOR_FILTER_ORDER_MAX) &&
			(orders.BlueBrightness >= COLOR_FILTER_ORDER_MIN) && (orders.BlueBrightness < COLOR_FILTER_ORDER_MAX) &&
			(orders.Voltage >= 3) && (orders.Voltage <= VOLTAGE_FILTER_ORDER_MAX) &&
			(orders.PeakVoltage >= 3) && (orders.PeakVoltage <= PEAK_VOLTAGE_FILTER_ORDER_MAX))
	{
		_settingsCtrlInstance->settingsData.filterOrders = orders;
		_ledCtrlInstance->SetFilterOrder(orders);

		if(saveToEEPROM)
			_settingsCtrlInstance->SaveSettings();
	}
	else
	{
		_uartCtrlInstance->Transmit(_invalidString);
	}
}

void RaveCubeController::ChangeFilterOrder(uint8_t* cmdStr)
{
	char* pSplittedStr = strtok((char*)cmdStr, " ");
	FilterLevels orders = {0};

	for(uint8_t i = 0; pSplittedStr != NULL; i++)
	{
		if(i == 1)
		{
			orders.RedBrightness = atoi(pSplittedStr);
		}

		if(i == 2)
		{
			orders.GreenBrightness = atoi(pSplittedStr);
		}

		if(i == 3)
		{
			orders.BlueBrightness = atoi(pSplittedStr);
		}

		if(i == 4)
		{
			orders.Voltage = atoi(pSplittedStr);
		}

		if(i == 5)
		{
			orders.PeakVoltage = atoi(pSplittedStr);
		}

		pSplittedStr = strtok(NULL, " ");
	}

	UpdateFilterOrders(orders, true);
}

void RaveCubeController::ChangeFilterOrder(FilterLevels newLevels, bool saveToEEPROM)
{
	char transmitBuffer[CTRL_TX_BUFFER_LENGTH];

	// For now, only update the color filters and use "old" voltage filter orders
	newLevels.PeakVoltage = _settingsCtrlInstance->settingsData.filterOrders.PeakVoltage;
	newLevels.Voltage = _settingsCtrlInstance->settingsData.filterOrders.Voltage;

	sprintf(transmitBuffer, "Filters adjusted to: R: %u, G: %u, B: %u \n", 
		newLevels.RedBrightness, newLevels.GreenBrightness, newLevels.BlueBrightness);
	_uartCtrlInstance->Transmit((uint8_t*) transmitBuffer);
	while(_uartCtrlInstance->IsTxBusy());

	UpdateFilterOrders(newLevels, saveToEEPROM);
}

void RaveCubeController::ChangeBoundaries(uint8_t* cmdStr)
{
	char* pSplittedStr = strtok((char*)cmdStr, " ");
	FrequencyBoundary red = {1,0}, green = {1,0}, blue = {1,0};

	for(uint8_t i = 0; pSplittedStr != NULL; i++)
	{
		switch(i)
		{
		case 1:
			red.Min = atoi(pSplittedStr);

			break;
		case 2:
			red.Max = atoi(pSplittedStr);

			break;
		case 3:
			green.Min = atoi(pSplittedStr);

			break;
		case 4:
			green.Max = atoi(pSplittedStr);

			break;
		case 5:
			blue.Min = atoi(pSplittedStr);

			break;
		case 6:
			blue.Max = atoi(pSplittedStr);

			break;
		}

		pSplittedStr = strtok(NULL, " ");
	}

	if((red.Min >= 0) && (red.Max >= 0) && (green.Min >= 0) &&
			(green.Max >= 0) && (blue.Min >= 0) && (blue.Max >= 0) &&
			(red.Min < red.Max) && (green.Min < green.Max) && (blue.Min < blue.Max))
	{
		_settingsCtrlInstance->settingsData.boundaries = {red, green, blue};
		_settingsCtrlInstance->SaveSettings();

//		uartCtrlInstance->Transmit("Boundaries set: "
//				"Red {" + to_string(red.Min) + "," + to_string(red.Max) + "}; "
//				"Green {" + to_string(green.Min) + "," + to_string(green.Max) + "}; "
//				"Blue {" + to_string(blue.Min) + "," + to_string(blue.Max) + "}");
	}
	else
	{
		_uartCtrlInstance->Transmit(_invalidString);
	}

	while(_uartCtrlInstance->IsTxBusy());
}

void RaveCubeController::ChangeLedStatus(uint8_t* cmdStr)
{
	char* pSplittedStr = strtok((char*)cmdStr, " ");
	uint8_t ledStatus = SET;

	for(uint8_t i = 0; pSplittedStr != NULL; i++)
	{
		switch(i)
		{
		case 1:
			ledStatus = atoi(pSplittedStr);

			break;
		}

		pSplittedStr = strtok(NULL, " ");
	}

	if(ledStatus == SET || ledStatus == RESET)
	{
		char transmitBuffer[CTRL_TX_BUFFER_LENGTH];

		_settingsCtrlInstance->settingsData.ledStatus = ledStatus;
		_ledCtrlInstance->ResetLedMatrix();
		_settingsCtrlInstance->SaveSettings();

		sprintf(transmitBuffer, "LedStatus set: %u \n", ledStatus);
		_uartCtrlInstance->Transmit((uint8_t*) transmitBuffer);
	}
	else
	{
		_uartCtrlInstance->Transmit(_invalidString);
	}

	while(_uartCtrlInstance->IsTxBusy());
}

void RaveCubeController::ChangeGamma(uint8_t* cmdStr)
{
	char* pSplittedStr = strtok((char*)cmdStr, " ");
	uint8_t gammaStatus = SET;

	for(uint8_t i = 0; pSplittedStr != NULL; i++)
	{
		switch(i)
		{
		case 1:
			gammaStatus = atoi(pSplittedStr);

			break;
		}

		pSplittedStr = strtok(NULL, " ");
	}

	if(gammaStatus == SET || gammaStatus == RESET)
	{
		_settingsCtrlInstance->settingsData.gammaStatus = gammaStatus;
		_settingsCtrlInstance->SaveSettings();

		char transmitBuffer[CTRL_TX_BUFFER_LENGTH];

		sprintf(transmitBuffer, "GammaStatus set: %u \n", gammaStatus);
		_uartCtrlInstance->Transmit((uint8_t*) transmitBuffer);
	}
	else
	{
		_uartCtrlInstance->Transmit(_invalidString);
	}

	while(_uartCtrlInstance->IsTxBusy());
}

void RaveCubeController::ChangeBrightnessFactors(uint8_t* cmdStr)
{
	char* pSplittedStr = strtok((char*)cmdStr, " ");
	BrightnessFactors factors = {0.0f , 0.0f , 0.0f};

	for(uint8_t i = 0; pSplittedStr != NULL; i++)
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
		}

		pSplittedStr = strtok(NULL, " ");
	}

	if(factors.Red >= 0.0f && factors.Green >= 0.0f && factors.Blue >= 0.0f)
	{
		char transmitBuffer[CTRL_TX_BUFFER_LENGTH];

		_settingsCtrlInstance->settingsData.factors = factors;
		_settingsCtrlInstance->SaveSettings();

		sprintf(transmitBuffer, "BrightnessFactors set: Red: %f Green: %f Blue: %f \n", factors.Red, factors.Green, factors.Blue);
		_uartCtrlInstance->Transmit((uint8_t*)transmitBuffer);
	}
	else
	{
		_uartCtrlInstance->Transmit(_invalidString);
	}

	while(_uartCtrlInstance->IsTxBusy());
}

void RaveCubeController::ChangeAmplitude(uint8_t* cmdStr)
{
	char* pSplittedStr = strtok((char*)cmdStr, " ");
	float32_t amplitude = 0.0f;

	for(uint8_t i = 0; pSplittedStr != NULL; i++)
	{
		if(i == 1)
		{
			amplitude = (float)atof(pSplittedStr);
		}

		pSplittedStr = strtok(NULL, " ");
	}

	if(amplitude > 0.0f && amplitude < 1500.0f)
	{
		_settingsCtrlInstance->settingsData.amplitude = amplitude;
		_settingsCtrlInstance->SaveSettings();
	}
	else
	{
		_uartCtrlInstance->Transmit(_invalidString);
	}

	while(_uartCtrlInstance->IsTxBusy());
}

void RaveCubeController::SendData()
{
	char transmitBuffer[CTRL_TX_BUFFER_LENGTH];

	sprintf(transmitBuffer, "%u %u %u %u %u %u %u %u %u %u %u %u %u %f %f %f\n",
			_settingsCtrlInstance->settingsData.filterOrders.RedBrightness,
			_settingsCtrlInstance->settingsData.filterOrders.GreenBrightness,
			_settingsCtrlInstance->settingsData.filterOrders.BlueBrightness,
			_settingsCtrlInstance->settingsData.filterOrders.Voltage,
			_settingsCtrlInstance->settingsData.filterOrders.PeakVoltage,
			_settingsCtrlInstance->settingsData.boundaries.Red.Min,
			_settingsCtrlInstance->settingsData.boundaries.Red.Max,
			_settingsCtrlInstance->settingsData.boundaries.Green.Min,
			_settingsCtrlInstance->settingsData.boundaries.Green.Max,
			_settingsCtrlInstance->settingsData.boundaries.Blue.Min,
			_settingsCtrlInstance->settingsData.boundaries.Blue.Max,
			_settingsCtrlInstance->settingsData.ledStatus,
			_settingsCtrlInstance->settingsData.gammaStatus,
			_settingsCtrlInstance->settingsData.factors.Red,
			_settingsCtrlInstance->settingsData.factors.Green,
			_settingsCtrlInstance->settingsData.factors.Blue);

	_uartCtrlInstance->Transmit((uint8_t*)transmitBuffer);
	while(_uartCtrlInstance->IsTxBusy());
}

void RaveCubeController::SetStreamingFilterValueStatus(uint8_t streamFilterStatus)
{
	_streamBrightnessValuesEnable = streamFilterStatus;
}

uint8_t RaveCubeController::GetStreamingFilterValueStatus()
{
	return _streamBrightnessValuesEnable;
}

void RaveCubeController::SendStreamingData()
{
	uint8_t uartIsBusy = _uartCtrlInstance->IsTxBusy();

	if(uartIsBusy != SET && _streamBrightnessValuesEnable == SET)
	{
		// Send only every nth value to prevent data loss
		if((++_dataStreamCounter % _dataStreamDivider) == 0)
		{
			_dataStreamCounter = 0;

//			if(_streamAmplitudeValuesEnable == SET)
//			{
//				ColorAmplitudes avgAmplitudes = ledCtrlInstance->GetAvgAmplitude();
//
//	//			string dataStr = "_" + to_string(avgAmplitudes.red) + " " +
//	//					to_string(avgAmplitudes.green) + " " +
//	//					to_string(avgAmplitudes.blue) + " " +
//	//					to_string(avgAmplitudes.sum) + "_\n";
//	//
//	//			uartCtrlInstance->Transmit(dataStr);
//			}

			RgbLedBrightness brightness = _ledCtrlInstance->GetCurrentBrightness();

			char transmitBuffer[CTRL_TX_BUFFER_LENGTH];
			sprintf(transmitBuffer, "_%u %u %u %f %f_\n", brightness.Red, brightness.Green, brightness.Blue,
					_ledCtrlInstance->GetRMSVoltage(), _ledCtrlInstance->GetPeakRMSVoltage());
			_uartCtrlInstance->Transmit((uint8_t*)transmitBuffer);

			while(_uartCtrlInstance->IsTxBusy());
		}
	}
}

RaveCubeController::~RaveCubeController()
{
	// TODO Auto-generated destructor stub
}

