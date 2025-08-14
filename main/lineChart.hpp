#pragma once
#include "lcd.hpp"

template <class T>
class LineChart
{
public:
	using Color = ILI9341::Color;

	LineChart(unsigned dataSize);
	LineChart(LineChart& copy);
	LineChart& operator=(LineChart& copy);
	~LineChart();

	void drawTo(ILI9341& lcd, Color color = 0xFFFF, Color AxisColor = 0xFFFF, Color backGroundColor = 0x0000);
	void addData(T data);
	void setPosition(Vector2uc start, Vector2uc size);
	void setAutoMinMax(bool autoMin, bool autoMax);
	void setMinMax(T min, T max);
	T getMin();
	T getMax();

private:
	Vector2uc position[2];
	float deltaX = 0.0f;
	float zoomY = 0.0f;
	unsigned dataSize = 0;
	unsigned startIndex = 0;
	unsigned endIndex = 0;
	unsigned lastDrawStartIndex = 0;
	Vector2uc* lastDrawPositions = nullptr;
	T* data = nullptr;
	T dataMax = 0;
	T dataMin = 0;
	bool autoMinMax[2] = { true, true };

	void updateMinMax();
};

template<class T>
LineChart<T>::LineChart(unsigned dataSize)
{
	this->dataSize = dataSize;

	delete[] data;
	data = new T[dataSize];
	for (unsigned i = 0; i < dataSize; i++) data[i] = 0;

	delete[] lastDrawPositions;
	lastDrawPositions = new Vector2uc[dataSize];
	for (unsigned i = 0; i < dataSize; i++) lastDrawPositions[i] = { 0, 0 };

	startIndex = 0;
	endIndex = dataSize - 1;
}

template<class T>
inline LineChart<T>::LineChart(LineChart& copy)
{
	position[0] = copy.position[0];
	position[1] = copy.position[1];
	deltaX = copy.deltaX;
	zoomY = copy.zoomY;
	dataSize = copy.dataSize;
	startIndex = copy.startIndex;
	endIndex = copy.endIndex;
	lastDrawStartIndex = copy.lastDrawStartIndex;
	dataSize = copy.dataSize;

	delete[] data;
	data = new T[dataSize];
	for (unsigned i = 0; i < dataSize; i++) data[i] = copy.data[i];

	delete[] lastDrawPositions;
	lastDrawPositions = new Vector2uc[dataSize];
	for (unsigned i = 0; i < dataSize; i++) lastDrawPositions[i] = copy.lastDrawPositions[i];

	dataMax = copy.dataMax;
	dataMin = copy.dataMin;
	autoMinMax[0] = copy.autoMinMax[0];
	autoMinMax[1] = copy.autoMinMax[1];
}

template<class T>
inline LineChart<T>& LineChart<T>::operator=(LineChart& copy)
{
	position[0] = copy.position[0];
	position[1] = copy.position[1];
	deltaX = copy.deltaX;
	zoomY = copy.zoomY;
	dataSize = copy.dataSize;
	startIndex = copy.startIndex;
	endIndex = copy.endIndex;
	lastDrawStartIndex = copy.lastDrawStartIndex;

	delete[] data;
	data = new T[dataSize];
	for (unsigned i = 0; i < dataSize; i++) data[i] = copy.data[i];

	delete[] lastDrawPositions;
	lastDrawPositions = new Vector2uc[dataSize];
	for (unsigned i = 0; i < dataSize; i++) lastDrawPositions[i] = copy.lastDrawPositions[i];

	dataMax = copy.dataMax;
	dataMin = copy.dataMin;
	autoMinMax[0] = copy.autoMinMax[0];
	autoMinMax[1] = copy.autoMinMax[1];

	return *this;
}

template<class T>
LineChart<T>::~LineChart()
{
	dataSize = 0;
	startIndex = 0;
	endIndex = 0;
	delete[] data;
	delete[] lastDrawPositions;
}

template<class T>
void LineChart<T>::drawTo(ILI9341& lcd, Color color, Color AxisColor, Color backGroundColor)
{
	auto startIndex = this->startIndex; // 维护线程安全
	Vector2f lastClearPosition = lastDrawPositions[lastDrawStartIndex];
	Vector2f lastPosition = { (float)position[0].x, position[1].y - (data[startIndex] - dataMin) * zoomY };
	if (lastPosition.y < position[0].y) lastPosition.y = position[0].y;
	if (lastPosition.y > position[1].y) lastPosition.y = position[1].y;
	lastDrawPositions[0] = lastPosition;

	Vector2f nowPosition = lastPosition;
	for (unsigned i = startIndex + 1, j = 1; i != startIndex; i++, j++)
	{
		if (i == dataSize)
		{
			i = 0;
			if (i == startIndex) break;
		}

		// clear
		lcd.drawLine(lastClearPosition, lastDrawPositions[j], backGroundColor);
		lastClearPosition = lastDrawPositions[j];

		// draw
		nowPosition.x += deltaX;
		nowPosition.y = position[1].y - (data[i] - dataMin) * zoomY;
		if (nowPosition.y < position[0].y) nowPosition.y = position[0].y;
		if (nowPosition.y > position[1].y) nowPosition.y = position[1].y;

		lcd.drawLine((Vector2uc)lastPosition, (Vector2uc)nowPosition, color);
		lastDrawPositions[j] = (Vector2uc)nowPosition;
		lastPosition = nowPosition;
	}

	lcd.drawRectangle(position[0], { position[0].x, position[1].y }, AxisColor);
	lcd.drawRectangle({ position[0].x, position[1].y }, position[1], AxisColor);
}

template<class T>
void LineChart<T>::addData(T data)
{
	T last = this->data[startIndex];

	this->data[startIndex] = data;
	endIndex = startIndex;
	startIndex++;
	if (startIndex == dataSize) startIndex = 0;

	if (data < dataMin || dataMax < data || last == dataMax || last == dataMin) updateMinMax();
}

template <class T>
void LineChart<T>::setPosition(Vector2uc start, Vector2uc size)
{
	position[0] = start;
	position[1] = start + size;
	deltaX = (float)size.x / (float)(dataSize - 1);

	for (unsigned i = 0; i < dataSize; i++) lastDrawPositions[i] = position[1];
}

template<class T>
inline void LineChart<T>::setAutoMinMax(bool autoMin, bool autoMax)
{
	autoMinMax[0] = autoMin;
	autoMinMax[1] = autoMax;
	updateMinMax();
}

template<class T>
inline void LineChart<T>::setMinMax(T min, T max)
{
	if (!autoMinMax[0]) dataMin = min;
	if (!autoMinMax[1]) dataMax = max;
	if (!autoMinMax[0] && !autoMinMax[1])
		zoomY = (float)(position[1].y - position[0].y) / (float)(dataMax - dataMin);
}

template<class T>
inline T LineChart<T>::getMin()
{
	return dataMin;
}

template<class T>
inline T LineChart<T>::getMax()
{
	return dataMax;
}

template<class T>
void LineChart<T>::updateMinMax()
{
	if (autoMinMax[0])
	{
		dataMin = data[0];
		for (unsigned i = 1; i < dataSize; i++)
			if (data[i] < dataMin) dataMin = data[i];
		zoomY = (float)(position[1].y - position[0].y) / (float)(dataMax - dataMin);
	}

	if (autoMinMax[1])
	{
		dataMax = data[0];
		for (unsigned i = 1; i < dataSize; i++)
			if (data[i] > dataMax) dataMax = data[i];
		zoomY = (float)(position[1].y - position[0].y) / (float)(dataMax - dataMin);
	}
	if (dataMax == dataMin) zoomY = 1.0f;
}
