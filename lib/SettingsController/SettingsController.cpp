/*
 * SettingsController.cpp
 *
 *  Created on: Apr 11, 2021
 *      Author: Christoph
 */

#include <SettingsController.hpp>

SettingsController::SettingsController(I2CController* i2cInstance)
{
	SettingsFrame settingsData;
	SettingsFrame settingsFromEEPROM;

	_i2c = i2cInstance;

	_i2c->ReadDataEEPROM(&settingsData, 0);

	// First boot, no data is stored yet -> default Values
	if(settingsData.controlFlag != SET || settingsData.filterOrders.RedBrightness > COLOR_FILTER_ORDER_MAX ||
			settingsData.filterOrders.Voltage > VOLTAGE_FILTER_ORDER_MAX || settingsData.amplitude < 30.0f)
	{
		SetDefaultValues(&settingsData);
		_i2c->WriteDataEEPROM(&settingsData, 0);
		_i2c->ReadDataEEPROM(&settingsFromEEPROM, 0);

		// if settings don't match, try again
		if(CompareSettings(settingsData, settingsFromEEPROM) == RESET)
		{
			_i2c->WriteDataEEPROM(&settingsData, 0);
		}
	}

	this->settingsData = settingsData;
}

SettingsController::~SettingsController()
{

}

void SettingsController::SetDefaultValues(SettingsFrame* settingsData)
{
	settingsData->amplitude = 800.0f;
	settingsData->boundaries = {{30,150}, {150,750}, {750, 6000}};
	settingsData->filterOrders.RedBrightness = COLOR_FILTER_RED_ORDER_INIT;
	settingsData->filterOrders.GreenBrightness = COLOR_FILTER_GREEN_ORDER_INIT;
	settingsData->filterOrders.BlueBrightness = COLOR_FILTER_BLUE_ORDER_INIT;
	settingsData->filterOrders.Voltage = VOLTAGE_FILTER_ORDER_INIT;
	settingsData->filterOrders.PeakVoltage = PEAK_VOLTAGE_FILTER_ORDER_INIT;
	settingsData->controlFlag = SET;
	settingsData->factors = { 2.5f, 1.0f, 1.3f };
	settingsData->gammaStatus = SET;
	settingsData->ledStatus = SET;
}

uint8_t SettingsController::CompareSettings(SettingsFrame data1, SettingsFrame data2)
{
	if(data1.amplitude != data2.amplitude ||
			data1.filterOrders.RedBrightness != data2.filterOrders.RedBrightness ||
			data1.filterOrders.GreenBrightness != data2.filterOrders.GreenBrightness ||
			data1.filterOrders.BlueBrightness != data2.filterOrders.BlueBrightness ||
			data1.filterOrders.Voltage != data2.filterOrders.Voltage ||
			data1.filterOrders.PeakVoltage != data2.filterOrders.PeakVoltage ||
			data1.gammaStatus != data2.gammaStatus ||
			data1.ledStatus != data2.ledStatus)
	{
		return RESET;
	}

	if(data1.boundaries.Red.Min != data2.boundaries.Red.Min || data1.boundaries.Red.Max != data2.boundaries.Red.Max ||
			data1.boundaries.Green.Min != data2.boundaries.Green.Min || data1.boundaries.Green.Max != data2.boundaries.Green.Max ||
			data1.boundaries.Blue.Min != data2.boundaries.Blue.Min || data1.boundaries.Blue.Max != data2.boundaries.Blue.Max)
	{
		return RESET;
	}

	if(data1.factors.Red != data2.factors.Red || data1.factors.Green != data2.factors.Green || data1.factors.Blue != data2.factors.Blue)
	{
		return RESET;
	}

	return SET;
}

void SettingsController::SaveSettings()
{
	SettingsFrame settingsFromEEPROM;

	_i2c->WriteDataEEPROM(&settingsData, 0);
	_i2c->ReadDataEEPROM(&settingsFromEEPROM, 0);

	// if settings don't match, try again
	if(CompareSettings(settingsData, settingsFromEEPROM) == RESET)
	{
		_i2c->WriteDataEEPROM(&settingsData, 0);
	}
}
