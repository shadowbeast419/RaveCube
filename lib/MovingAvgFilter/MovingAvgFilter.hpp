/*
 * MovingAvgFilter.hpp
 *
 *  Created on: 5 Oct 2020
 *      Author: Christoph
 */

#ifndef INC_MOVINGAVGFILTER_H_
#define INC_MOVINGAVGFILTER_H_

#include <RgbLedBrightness.hpp>
#include <FilterLevels.hpp>

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

enum VoltageFilterSelection
{
	PeakVoltage,
	Voltage
};

class MovingAvgFilter
{
private:
	FilterLevels						_filterOrders;

	RingBufferNodeColorAmplitude		_ringBufferRedBrightness[FilterLevelsColor::FilterLevelsMax];
	RingBufferNodeColorAmplitude*		_currentNodeRedBrightness = NULL; // Current node of the color amplitudes in the ring buffer

	RingBufferNodeColorAmplitude		_ringBufferGreenBrightness[FilterLevelsColor::FilterLevelsMax];
	RingBufferNodeColorAmplitude*		_currentNodeGreenBrightness = NULL;

	RingBufferNodeColorAmplitude		_ringBufferBlueBrightness[FilterLevelsColor::FilterLevelsMax];
	RingBufferNodeColorAmplitude*		_currentNodeBlueBrightness = NULL;

	RingBufferNodeVoltage				_ringBufferVoltage[FilterLevelsVoltage::FilterLevelsMax];
	RingBufferNodeVoltage*				_currentNodeVoltage = NULL;

	RingBufferNodeVoltage				_ringBufferPeakVoltage[FilterLevelsVoltage::FilterLevelsMax];
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
	void SetColorFilterOrder(FilterLevelsColor orders);
};

#endif /* INC_MOVINGAVGFILTER_H_ */
