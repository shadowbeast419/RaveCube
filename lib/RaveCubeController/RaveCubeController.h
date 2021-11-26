/*
 * RaveCubeControler.h
 *
 *  Created on: 23 Nov 2020
 *      Author: Christoph
 */

#ifndef SRC_RAVECUBECONTROLER_H_
#define SRC_RAVECUBECONTROLER_H_

#include <LedController.h>
#include <UartController.h>
#include <SettingsController.h>
#include <cstring>
#include <cstdio>

#define CTRL_TX_BUFFER_LENGTH 256

class RaveCubeController
{
private:
	LedController* 		_ledCtrlInstance;
	UartController* 	_uartCtrlInstance;
	SettingsController* _settingsCtrlInstance;
	uint8_t				_streamBrightnessValuesEnable = RESET;
	uint8_t				_streamAmplitudeValuesEnable = RESET;
	uint8_t				_dataStreamCounter = 0;
	uint8_t				_dataStreamDivider = 2;
	uint8_t*			_invalidString = (uint8_t*)"Invalid command! \n";

	void ChangeFilterOrder(uint8_t* cmdStr);
	void ChangeBoundaries(uint8_t* cmdStr);
	void ChangeLedStatus(uint8_t* cmdStr);
	void ChangeGamma(uint8_t* cmdStr);
	void ChangeBrightnessFactors(uint8_t* cmdStr);
	void ChangeAmplitude(uint8_t* cmdStr);
	void ChangeStreamingBrightnessValuesEnable(uint8_t* cmdStr);
	void ChangeStreamingAmplitudeValuesEnable(uint8_t* cmdStr);

	void SendData();

	void SetStreamingFilterValueStatus(uint8_t streamFilterStatus);
	uint8_t GetStreamingFilterValueStatus();

public:
	RaveCubeController(LedController* ledCtrlInstance, UartController* uartCtrlInstance, SettingsController* settingCtrl);
	void ExecuteCommand(uint8_t* cmdStr);
	void SendStreamingData();

	virtual ~RaveCubeController();
};

#endif /* SRC_RAVECUBECONTROLER_H_ */
