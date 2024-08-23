#pragma once

#include "vectormath.h"
#include "sdfobject.h"
#include "ray.h"
#include "Scene.h"
#include "bitmaps.h"
#include <mutex>
#include <string>
#include <algorithm>
#include <iostream>
#include "MinWindow.h"

constexpr int BYTES_PER_PIXEL = 3;
typedef unsigned char byte;

class ImageData {

public:
	ImageData(int w, int h)
		: width(w), height(h), data(NULL)
	{
		int N = width * height * BYTES_PER_PIXEL;
		data = new byte[N];
		if (!data)
			throw "Image data allocation failed";
		std::fill(data, data + N, 0);
	}
	~ImageData()
	{
		if (data)
			delete[] data;
	}
	
	ImageData(const ImageData& other)
	{
		width = other.width;
		height = other.height;
		int N = width * height * BYTES_PER_PIXEL;
		data = new byte[N];
		memcpy(data, other.data, N);
	}
	
	void storeValues(int wi, int hi, const std::vector<byte>& values) {
		//int index = wi * height + hi;
		int index = hi * width + wi;
		storeValues(index, values);
	}
	void storeValues(int index, const std::vector<byte>& values)
	{
		//std::unique_lock<std::mutex> lock(this->mutex);
		int imindex = index * 3;
		for (int k = 2; k >= 0; k--, imindex++)
		{
			data[imindex] = values[k];
		}
	}
	void storeValues(const std::vector<int>& indices, const std::vector<std::vector<byte>>& values)
	{
		//std::unique_lock<std::mutex> lock(this->mutex);
		auto i = indices.begin();
		auto v = values.begin();
		for (; i < indices.end(); i++, v++)
		{
			int imindex = *i * 3;
			for (int k = 2; k >= 0; k--, imindex++)
			{
				data[imindex] = (*v)[k];
			}
		}
	}


	void writeImage(std::string filename)
	{
		//std::unique_lock<std::mutex> lock(this->mutex);
		WriteImage(filename.c_str(), data, width, height, BYTES_PER_PIXEL);
	}

	void drawImage(MinWindow& window) const;


private:
	byte* data;
	int width, height;
	// std::mutex mutex; // mutex not needed since each thread writes to different location in the data array
};




void renderImage(const Scene&, const int);

void tileTask(int, ImageData&, const Scene&, const std::vector<int>&);

std::vector<std::vector<int>> createTiles(int, int, int);








