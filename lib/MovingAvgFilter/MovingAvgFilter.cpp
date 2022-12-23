/*
 * MovingAvgFilter.cpp
 *
 *  Created on: 5 Oct 2020
 *      Author: Christoph
 */

#include <MovingAvgFilter.hpp>

MovingAvgFilter::MovingAvgFilter()
{
	InitRingBuffer();
}

void MovingAvgFilter::InitRingBuffer()
{
	InitColorRingBuffer(_ringBufferRedBrightness, _filterOrders.Color.RedBrightness, &_currentNodeRedBrightness, true);
	InitColorRingBuffer(_ringBufferGreenBrightness, _filterOrders.Color.GreenBrightness, &_currentNodeGreenBrightness, true);
	InitColorRingBuffer(_ringBufferBlueBrightness, _filterOrders.Color.BlueBrightness, &_currentNodeBlueBrightness, true);
	InitVoltageRingBuffer(_ringBufferVoltage, _filterOrders.Voltage.Voltage, &_currentNodeVoltage, 20.f);
	InitVoltageRingBuffer(_ringBufferPeakVoltage, _filterOrders.Voltage.PeakVoltage, &_currentNodePeakVoltage, 50.0f);
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

void MovingAvgFilter::InitColorRingBuffer(RingBufferNodeColorAmplitude* ringBuffer, uint16_t order, 
	RingBufferNodeColorAmplitude** _currentNode, bool clearRingBuffer)
{
	// Watch out for the maximum limit
	order = order > FilterLevelsColor::FilterLevelsMax ? FilterLevelsColor::FilterLevelsMax : order;

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

	// If clearing is not necessary, set all the values to the last value
	if(clearRingBuffer)
	{
		// Initialize the brightness values
		for(uint16_t i = 0; i < order; i++)
		{
			ringBuffer[i].brightness = 0;
		}
	}

	*_currentNode = &ringBuffer[0];
}


// Increases/decreases the filter order with adding or removing nodes of the ringbuffer. The Current Node stays the same if order value has decreased
RingBufferNodeVoltage* MovingAvgFilter::ChangeVoltageRingBufferOrder(uint16_t order, RingBufferNodeVoltage* ringBuffer, RingBufferNodeVoltage* currentNode, VoltageFilterSelection selection)
{
	// Watch out for the upper limit
	uint16_t newOrder = order > FilterLevelsVoltage::FilterLevelsMax ? FilterLevelsVoltage::FilterLevelsMax : order;

	uint16_t currentOrder = 0;
	int16_t orderDiff = 0;

	switch(selection)
	{
		case Voltage:
			orderDiff = _filterOrders.Voltage.Voltage - newOrder;
			currentOrder = _filterOrders.Voltage.Voltage;

			break;

		case PeakVoltage:
			orderDiff = _filterOrders.Voltage.PeakVoltage - newOrder;
			currentOrder = _filterOrders.Voltage.PeakVoltage;

			break;
	}

	// No Change
	if(orderDiff == 0)
		return currentNode;

	RingBufferNodeVoltage* newEnd = currentNode;

	// New order is smaller than the current one
	if(orderDiff > 0)
	{
		// Cut out the last nodes and connect the ends again

		// Go to the new end
		for(uint16_t i = 0; i < newOrder; i++)
		{
			newEnd = newEnd->pPrev;
		}

		// Now go further until we reach the previous end
		RingBufferNodeVoltage* prevEnd = newEnd;

		for(uint16_t i = 0; i < orderDiff; i++)
		{
			prevEnd = prevEnd->pPrev;
		}

		// Connect the two "ends" together
		// NewEnd pPrev points to pPrev from pNew
		newEnd->pPrev = prevEnd->pPrev;

		return currentNode;
	}

	// New order is larger
	if(orderDiff < 0)
	{
		uint16_t newElementCnt = abs(orderDiff);

		// Append new nodes at the current node
		for(uint16_t offset = 0; offset < newElementCnt; offset++)
		{
			RingBufferNodeVoltage* nextAvailableNode = &ringBuffer[currentOrder - 1 + offset];
			nextAvailableNode->voltage = currentNode->voltage;
			
			currentNode->pNext = nextAvailableNode;

			// Move one new node forward
			currentNode = currentNode->pNext; 
		}

		return currentNode;
	}

	return currentNode;
}

RingBufferNodeColorAmplitude* MovingAvgFilter::ChangeColorRingBufferOrder(uint16_t order, RingBufferNodeColorAmplitude* ringBuffer, RingBufferNodeColorAmplitude* currentNode, ColorSelection selection)
{
	// Watch out for the maximum limit
	uint16_t newOrder = order > FilterLevelsColor::FilterLevelsMax ? FilterLevelsColor::FilterLevelsMax : order;
	uint16_t currentOrder = 0;
	int16_t orderDiff = 0;

	switch(selection)
	{
		case Red:
			orderDiff = _filterOrders.Color.RedBrightness - newOrder;
			currentOrder = _filterOrders.Color.RedBrightness;

			break;

		case Green:
			orderDiff = _filterOrders.Color.GreenBrightness - newOrder;
			currentOrder = _filterOrders.Color.GreenBrightness;

			break;

		case Blue:
			orderDiff = _filterOrders.Color.BlueBrightness - newOrder;
			currentOrder = _filterOrders.Color.BlueBrightness;

			break;
	}

	// No Change
	if(orderDiff == 0)
		return currentNode;

	RingBufferNodeColorAmplitude* newEnd = currentNode;

	// New order is smaller than the current one
	if(orderDiff > 0)
	{
		// Cut out the last nodes and connect the ends again

		// Go to the new end
		for(uint16_t i = 0; i < newOrder; i++)
		{
			newEnd = newEnd->pPrev;
		}

		// Now go further until we reach the previous end
		RingBufferNodeColorAmplitude* prevEnd = newEnd;

		for(uint16_t i = 0; i < orderDiff; i++)
		{
			prevEnd = prevEnd->pPrev;
		}

		// Connect the two "ends" together
		// NewEnd pPrev points to pPrev from pNew
		newEnd->pPrev = prevEnd->pPrev;

		return currentNode;
	}

	// New order is larger
	if(orderDiff < 0)
	{
		uint16_t newElementCnt = abs(orderDiff);

		// Append new nodes at the current node
		for(uint16_t offset = 0; offset < newElementCnt; offset++)
		{
			RingBufferNodeColorAmplitude* nextAvailableNode = &ringBuffer[currentOrder - 1 + offset];

			// Assign the current brightness value to the new element
			nextAvailableNode->brightness = currentNode->brightness;
			currentNode->pNext = nextAvailableNode;

			// Move one new node forward
			currentNode = currentNode->pNext; 
		}

		return currentNode;
	}

	return currentNode;
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

	avgBrightness.Red = CalculateAverageBrightness(_currentNodeRedBrightness, _filterOrders.Color.RedBrightness);
	avgBrightness.Green = CalculateAverageBrightness(_currentNodeGreenBrightness, _filterOrders.Color.GreenBrightness);
	avgBrightness.Blue = CalculateAverageBrightness(_currentNodeBlueBrightness, _filterOrders.Color.BlueBrightness);
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
	return CalculateAverageVoltage(_currentNodeVoltage, _filterOrders.Voltage.Voltage);
}

float32_t MovingAvgFilter::GetAveragePeakVoltage()
{
	return CalculateAverageVoltage(_currentNodePeakVoltage, _filterOrders.Voltage.PeakVoltage);
}

FilterLevels MovingAvgFilter::GetFilterOrder()
{
	return _filterOrders;
}

void MovingAvgFilter::SetFilterOrder(FilterLevels orders)
{
	FilterLevelsColor colorOrders;
	colorOrders.RedBrightness = orders.Color.RedBrightness;
	colorOrders.GreenBrightness = orders.Color.GreenBrightness;
	colorOrders.BlueBrightness = orders.Color.BlueBrightness;

	SetColorFilterOrder(colorOrders);

	if(orders.Voltage.Voltage != _filterOrders.Voltage.Voltage)
	{
		_filterOrders.Voltage = orders.Voltage;
		_currentNodeVoltage = ChangeVoltageRingBufferOrder(_filterOrders.Voltage.Voltage,_ringBufferVoltage, _currentNodeVoltage, Voltage);
	}

	if(orders.Voltage.PeakVoltage != _filterOrders.Voltage.PeakVoltage)
	{
		_filterOrders.Voltage.PeakVoltage = orders.Voltage.PeakVoltage;
		_currentNodeVoltage = ChangeVoltageRingBufferOrder(_filterOrders.Voltage.Voltage,_ringBufferVoltage, _currentNodeVoltage, PeakVoltage);
	}
}


void MovingAvgFilter::SetColorFilterOrder(FilterLevelsColor orders)
{
	if(orders.RedBrightness != _filterOrders.Color.RedBrightness)
	{
		_filterOrders.Color.RedBrightness = orders.RedBrightness;
		_currentNodeRedBrightness = ChangeColorRingBufferOrder(_filterOrders.Color.RedBrightness, _ringBufferRedBrightness, _currentNodeRedBrightness, Red);
	}

	if(orders.GreenBrightness != _filterOrders.Color.GreenBrightness)
	{
		_filterOrders.Color.GreenBrightness = orders.GreenBrightness;
		_currentNodeGreenBrightness = ChangeColorRingBufferOrder(_filterOrders.Color.GreenBrightness, _ringBufferGreenBrightness, _currentNodeGreenBrightness, Green);
	}

	if(orders.BlueBrightness != _filterOrders.Color.BlueBrightness)
	{
		_filterOrders.Color.BlueBrightness = orders.BlueBrightness;
		_currentNodeBlueBrightness = ChangeColorRingBufferOrder(_filterOrders.Color.BlueBrightness, _ringBufferBlueBrightness, _currentNodeBlueBrightness, Blue);
	}
}

MovingAvgFilter::~MovingAvgFilter()
{
	// TODO Auto-generated destructor stub
}

