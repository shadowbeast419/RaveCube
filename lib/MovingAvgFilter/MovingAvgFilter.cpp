/*
 * MovingAvgFilter.cpp
 *
 *  Created on: 5 Oct 2020
 *      Author: Christoph
 */

#include <MovingAvgFilter.h>

MovingAvgFilter::MovingAvgFilter()
{
	_filterOrders.RedBrightness = COLOR_FILTER_RED_ORDER_INIT;
	_filterOrders.GreenBrightness = COLOR_FILTER_GREEN_ORDER_INIT;
	_filterOrders.BlueBrightness = COLOR_FILTER_BLUE_ORDER_INIT;
	_filterOrders.Voltage = VOLTAGE_FILTER_ORDER_INIT;
	_filterOrders.PeakVoltage = PEAK_VOLTAGE_FILTER_ORDER_INIT;

	InitRingBuffer();
}

void MovingAvgFilter::InitRingBuffer()
{
	InitColorRingBuffer(_ringBufferRedBrightness, _filterOrders.RedBrightness, &_currentNodeRedBrightness, 0);
	InitColorRingBuffer(_ringBufferGreenBrightness, _filterOrders.GreenBrightness, &_currentNodeGreenBrightness, 0);
	InitColorRingBuffer(_ringBufferBlueBrightness, _filterOrders.BlueBrightness, &_currentNodeBlueBrightness, 0);
	InitVoltageRingBuffer(_ringBufferVoltage, _filterOrders.Voltage, &_currentNodeVoltage, 150.f);
	InitVoltageRingBuffer(_ringBufferPeakVoltage, _filterOrders.PeakVoltage, &_currentNodePeakVoltage, 150.0f);
}

void MovingAvgFilter::AddBrightnessValue(RgbLedBrightness brightness)
{
	_currentNodeRedBrightness->brightness = brightness.Red;
	_currentNodeRedBrightness = _currentNodeRedBrightness->pNext;

	_currentNodeGreenBrightness->brightness = brightness.Green;
	_currentNodeGreenBrightness = _currentNodeGreenBrightness->pNext;

	_currentNodeBlueBrightness->brightness = brightness.Blue;
	_currentNodeBlueBrightness = _currentNodeBlueBrightness->pNext;
}

void MovingAvgFilter::AddVoltageValue(float32_t voltage)
{
	_currentNodeVoltage->voltage = voltage;
	_currentNodeVoltage = _currentNodeVoltage->pNext;
}

void MovingAvgFilter::AddPeakVoltageValue(float32_t voltage)
{
	_currentNodePeakVoltage->voltage = voltage;
	_currentNodePeakVoltage = _currentNodePeakVoltage->pNext;
}

void MovingAvgFilter::InitColorRingBuffer(RingBufferNodeColorAmplitude* ringBuffer, uint16_t order, RingBufferNodeColorAmplitude** _currentNode, uint16_t initValue)
{
	if(order > COLOR_FILTER_ORDER_MAX)
	{
		order = COLOR_FILTER_ORDER_MAX;
	}

	// Init ring buffer
	for(uint16_t i = order - 1; i > 0; i--)
	{
		ringBuffer[i].pPrev = &ringBuffer[i-1];
	}

	for(uint16_t i = 0; i < (uint16_t)(order - 1); i++)
	{
		ringBuffer[i].pNext = &ringBuffer[i+1];
	}

	// First element, points to end
	ringBuffer[0].pPrev = &ringBuffer[order - 1];

	// Last element points to start
	ringBuffer[order - 1].pNext = &ringBuffer[0];

	// Initialize the brightness values
	for(uint16_t i = 0; i < order; i++)
	{
		ringBuffer[i].brightness = initValue;
	}

	*_currentNode = &ringBuffer[0];
}

void MovingAvgFilter::InitVoltageRingBuffer(RingBufferNodeVoltage* ringBuffer, uint16_t order, RingBufferNodeVoltage** _currentNode, float32_t initValue)
{
	// Same for amplitude voltage
	for(uint16_t i = order - 1; i > 0; i--)
	{
		ringBuffer[i].pPrev = &ringBuffer[i-1];
	}

	for(uint16_t i = 0; i < (uint16_t)(order - 1); i++)
	{
		ringBuffer[i].pNext = &ringBuffer[i+1];
	}

	ringBuffer[0].pPrev = &ringBuffer[order - 1];
	ringBuffer[order - 1].pNext = &ringBuffer[0];

	*_currentNode = &ringBuffer[0];

	for(uint16_t i = 0; i < order; i++)
	{
		ringBuffer[i].voltage = initValue;
	}
}


uint16_t MovingAvgFilter::CalculateAverageBrightness(RingBufferNodeColorAmplitude* _currentNode, uint16_t order)
{
	float32_t sum = 0.0f;
	float32_t weight = 2.0f / (((float32_t)order) * ((float32_t)(order) + 1.0f));

	// Stores the start index
	RingBufferNodeColorAmplitude* pBufferColor = _currentNode;

	// Move the pointer to a previous position depending on the order of the filter
	for(uint16_t i = 0; i < order; i++)
	{
		pBufferColor = pBufferColor->pPrev;
	}

	// Calculate the sum of the upcoming values (depending on the order of the filter)
	for(uint16_t i = 0; i < order; i++)
	{
		sum += pBufferColor->brightness * ((float32_t)i + 1.0f);
		pBufferColor = pBufferColor->pNext;
	}

	return (uint16_t)(sum * weight);
}


RgbLedBrightness MovingAvgFilter::GetAverageBrightness()
{
	RgbLedBrightness avgBrightness = {0};

	avgBrightness.Red = CalculateAverageBrightness(_currentNodeRedBrightness, _filterOrders.RedBrightness);
	avgBrightness.Green = CalculateAverageBrightness(_currentNodeGreenBrightness, _filterOrders.GreenBrightness);
	avgBrightness.Blue = CalculateAverageBrightness(_currentNodeBlueBrightness, _filterOrders.BlueBrightness);
	avgBrightness.Reset = 0;

	return avgBrightness;
}

float32_t MovingAvgFilter::CalculateAverageVoltage(RingBufferNodeVoltage* _currentNode, uint16_t order)
{
	float32_t sum = 0.0f;
	float32_t weight = 2.0f / (((float32_t)order) * ((float32_t)(order) + 1.0f));

	// Stores the start index
	RingBufferNodeVoltage* pBufferVoltage = _currentNode;

	// Move the pointer to a previous position depending on the order of the filter
	for(uint16_t i = 0; i < order; i++)
	{
		pBufferVoltage = pBufferVoltage->pPrev;
	}

	// Calculate the sum of the upcoming values (depending on the order of the filter)
	for(uint16_t i = 0; i < order; i++)
	{
		sum += pBufferVoltage->voltage * ((float32_t)i + 1.0f);
		pBufferVoltage = pBufferVoltage->pNext;
	}

	return (sum * weight);
}

float32_t MovingAvgFilter::GetAverageVoltage()
{
	return CalculateAverageVoltage(_currentNodeVoltage, _filterOrders.Voltage);
}

float32_t MovingAvgFilter::GetAveragePeakVoltage()
{
	return CalculateAverageVoltage(_currentNodePeakVoltage, _filterOrders.PeakVoltage);
}

MovingAvgFilterOrder MovingAvgFilter::GetFilterOrder()
{
	return _filterOrders;
}

void MovingAvgFilter::SetFilterOrder(MovingAvgFilterOrder orders)
{
	if(orders.RedBrightness != _filterOrders.RedBrightness)
	{
		_filterOrders.RedBrightness = orders.RedBrightness;
		InitColorRingBuffer(_ringBufferRedBrightness, _filterOrders.RedBrightness, &_currentNodeRedBrightness, 0);
	}

	if(orders.GreenBrightness != _filterOrders.GreenBrightness)
	{
		_filterOrders.GreenBrightness = orders.GreenBrightness;
		InitColorRingBuffer(_ringBufferGreenBrightness, _filterOrders.GreenBrightness, &_currentNodeGreenBrightness, 0);
	}

	if(orders.BlueBrightness != _filterOrders.BlueBrightness)
	{
		_filterOrders.BlueBrightness = orders.BlueBrightness;
		InitColorRingBuffer(_ringBufferBlueBrightness, _filterOrders.BlueBrightness, &_currentNodeBlueBrightness, 0);
	}

	if(orders.Voltage != _filterOrders.Voltage)
	{
		_filterOrders.Voltage = orders.Voltage;
		InitVoltageRingBuffer(_ringBufferVoltage, _filterOrders.Voltage, &_currentNodeVoltage, 150.0f);
	}

	if(orders.PeakVoltage != _filterOrders.PeakVoltage)
	{
		_filterOrders.PeakVoltage = orders.PeakVoltage;
		InitVoltageRingBuffer(_ringBufferPeakVoltage, _filterOrders.PeakVoltage, &_currentNodePeakVoltage, 200.0f);
	}
}

MovingAvgFilter::~MovingAvgFilter()
{
	// TODO Auto-generated destructor stub
}

