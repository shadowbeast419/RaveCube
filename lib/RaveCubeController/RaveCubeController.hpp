// /*
//  * RaveCubeControler.h
//  *
//  *  Created on: 23 Nov 2020
//  *      Author: Christoph
//  */

// #ifndef SRC_RAVECUBECONTROLER_H_
// #define SRC_RAVECUBECONTROLER_H_

// #include <LedController.hpp>
// #include <UartController.hpp>
// #include <SettingsController.hpp>
// #include <cstring>
// #include <cstdio>

// #define CTRL_TX_BUFFER_LENGTH 256


// class RaveCubeController
// {
// private:
// 	LedController* 		_ledCtrlInstance;
// 	UartController* 	_uartCtrlInstance;
// 	SettingsController* _settingsCtrlInstance;
// 	bool				_streamBrightnessValuesEnable = false;
// 	bool				_streamAmplitudeValuesEnable = false;
// 	uint8_t				_dataStreamCounter = 0;
// 	uint8_t				_dataStreamDivider = 10;
// 	uint8_t* 			_invalidString = (uint8_t*)"Invalid command! \n";


// 	void ChangeFilterOrder(uint8_t* cmdStr);
// 	void ChangeBoundaries(uint8_t* cmdStr);
// 	void ChangeLedStatus(uint8_t* cmdStr);
// 	void ChangeGamma(uint8_t* cmdStr);
// 	void ChangeBrightnessFactors(uint8_t* cmdStr);
// 	void ChangeAmplitude(uint8_t* cmdStr);
// 	void ChangeStreamingBrightnessValuesEnable(uint8_t* cmdStr);
// 	void ChangeStreamingAmplitudeValuesEnable(uint8_t* cmdStr);
// 	void UpdateFilterOrders(FilterLevels orders, bool saveToEEPROM);
// 	// RaveCubeCommand GetCommandFromString(char* str);

// 	void SendData();

// 	void SetStreamingFilterValueStatus(uint8_t streamFilterStatus);
// 	uint8_t GetStreamingFilterValueStatus();

// public:
// 	RaveCubeController(LedController* ledCtrlInstance, UartController* uartCtrlInstance, SettingsController* settingCtrl);
// 	void ExecuteCommand(uint8_t* cmdStr);
// 	void SendStreamingData();
// 	void ChangeFilterOrder(FilterLevels newLevels, bool saveToEEPROM);

// 	virtual ~RaveCubeController();
// };

// #endif /* SRC_RAVECUBECONTROLER_H_ */
