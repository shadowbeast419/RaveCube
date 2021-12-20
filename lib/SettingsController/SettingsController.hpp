/*
 * SettingsController.h
 *
 *  Created on: Apr 11, 2021
 *      Author: Christoph
 */

#ifndef INC_SETTINGSCONTROLLER_H_
#define INC_SETTINGSCONTROLLER_H_

#include <I2CController.hpp>
#include <string.h>
#include <SettingsStructs.hpp>

class I2CController;

class SettingsController
{
private:
	I2CController* _i2c = NULL;

	void SetDefaultValues(SettingsFrame* settingsData);
	uint8_t CompareSettings(SettingsFrame data1, SettingsFrame data2);

public:
	SettingsFrame settingsData;

	SettingsController(I2CController* i2cInstance);
	virtual ~SettingsController();
	void SaveSettings();
};

#endif /* INC_SETTINGSCONTROLLER_H_ */
