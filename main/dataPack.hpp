#pragma once

class DataPack
{
public:
	DataPack() = default;
	DataPack(size_t size, void* data) : size{ size }, data{ data } {}
	DataPack(void* data, size_t size) : size{ size }, data{ data } {}

	DataPack(const DataPack&) = default;
	DataPack& operator=(const DataPack&) = default;

	DataPack(DataPack&&) = default;
	DataPack& operator=(DataPack&&) = default;

	~DataPack() {}

	size_t size = 0;
	void* data = 0;
};
