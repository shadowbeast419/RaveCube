/*
 * MovingAvgFilter.hpp
 *
 *  Created on: 5 Oct 2020
 *      Author: Christoph
 */

#ifndef INC_MOVINGAVGFILTER_H_
#define INC_MOVINGAVGFILTER_H_

#include <RgbLedBrightness.hpp>

#define COLOR_FILTER_ORDER_MAX 150
#define COLOR_FILTER_ORDER_MIN 3

#define COLOR_FILTER_RED_ORDER_INIT 10
#define COLOR_FILTER_GREEN_ORDER_INIT 10
#define COLOR_FILTER_BLUE_ORDER_INIT 10

#define VOLTAGE_FILTER_ORDER_MAX 100
#define VOLTAGE_FILTER_ORDER_INIT 50

#define PEAK_VOLTAGE_FILTER_ORDER_MAX 125
#define PEAK_VOLTAGE_FILTER_ORDER_INIT PEAK_VOLTAGE_FILTER_ORDER_MAX

struct RingBufferNodeColorAmplitude
{
	struct RingBufferNodeColorAmplitude* pPrev;
	struct RingBufferNodeColorAmplitude* pNext;
	uint16_t brightness;
};

struct RingBufferNodeVoltage
{
	struct RingBufferNodeVoltage* pPrev;
	struct RingBufferNodeVoltage* pNext;
	float32_t voltage;
};

struct FilterLevels
{
	uint16_t RedBrightness;
	uint16_t GreenBrightness;
	uint16_t BlueBrightness;
	uint16_t Voltage;
	uint16_t PeakVoltage;
};

enum VoltageFilterSelection
{
	PeakVoltage,
	Voltage
};

class MovingAvgFilter
{
private:
	FilterLevels						_filterOrders = {0};

	RingBufferNodeColorAmplitude		_ringBufferRedBrightness[COLOR_FILTER_ORDER_MAX];
	RingBufferNodeColorAmplitude*		_currentNodeRedBrightness = NULL; // Current node of the color amplitudes in the ring buffer
	uint16_t							_elementCounterRed = 0;

	RingBufferNodeColorAmplitude		_ringBufferGreenBrightness[COLOR_FILTER_ORDER_MAX];
	RingBufferNodeColorAmplitude*		_currentNodeGreenBrightness = NULL;
	uint16_t							_elementCounterGreen = 0;

	RingBufferNodeColorAmplitude		_ringBufferBlueBrightness[COLOR_FILTER_ORDER_MAX];
	RingBufferNodeColorAmplitude*		_currentNodeBlueBrightness = NULL;
	uint16_t							_elementCounterBlue = 0;

	RingBufferNodeVoltage				_ringBufferVoltage[VOLTAGE_FILTER_ORDER_MAX];
	RingBufferNodeVoltage*				_currentNodeVoltage = NULL;

	RingBufferNodeVoltage				_ringBufferPeakVoltage[PEAK_VOLTAGE_FILTER_ORDER_MAX];
	RingBufferNodeVoltage*				_currentNodePeakVoltage = NULL;

	void InitColorRingBuffer(RingBufferNodeColorAmplitude* ringBuffer, uint16_t order, RingBufferNodeColorAmplitude** _currentNode, 
		bool clearRingBuffer);
	void InitVoltageRingBuffer(RingBufferNodeVoltage* ringBuffer, uint16_t order, RingBufferNodeVoltage** _currentNode, float32_t initValue);
	uint16_t CalculateAverageBrightness(RingBufferNodeColorAmplitude* _currentNode, uint16_t order);
	float32_t CalculateAverageVoltage(RingBufferNodeVoltage* _currentNode, uint16_t order);
	RingBufferNodeColorAmplitude* ChangeColorRingBufferOrder(uint16_t order, RingBufferNodeColorAmplitude* ringBuffer, RingBufferNodeColorAmplitude* currentNode, ColorSelection selection);
	RingBufferNodeVoltage* ChangeVoltageRingBufferOrder(uint16_t order, RingBufferNodeVoltage* ringBuffer, RingBufferNodeVoltage* currentNode, VoltageFilterSelection selection);

public:
	MovingAvgFilter();
	virtual ~MovingAvgFilter();
	void InitRingBuffer();
	void InitColorAmplitudesFilter(uint16_t colorFilterOrder);
	void InitVoltageFilter(uint16_t voltageFilterOrder);
	void InitPeakVoltageFilter(uint16_t peakVoltageFilterOrder);
	void AddBrightnessValue(RgbLedBrightness amplitudes);
	void AddVoltageValue(float32_t voltage);
	void AddPeakVoltageValue(float32_t voltage);
	RgbLedBrightness GetAverageBrightness();
	float32_t GetAverageVoltage();
	float32_t GetAveragePeakVoltage();
	FilterLevels GetFilterOrder();
	void SetFilterOrder(FilterLevels orders);
	bool IsRingBufferFull(ColorSelection color);
};

#endif /* INC_MOVINGAVGFILTER_H_ */
