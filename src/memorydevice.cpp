#include <cstring>
#include <iostream>
#include <fstream>
#include "memorydevice.h"

MemoryDevice::MemoryDevice()
{
	blocks = new char*[NUM_BLOCKS];
	for(int i = 0; i < NUM_BLOCKS; i++)
	{
		blocks[i] = new char[BLOCK_SIZE];
	}
}

MemoryDevice::~MemoryDevice()
{
	for(int i = 0; i < NUM_BLOCKS; i++)
	{
		delete[] blocks[i];
	}
	delete[] blocks;
}

int MemoryDevice::write(int block,const char* data, int num_bytes, int start)
{
	if(block < 0 || block >= NUM_BLOCKS)
		return 0;
	if(start < 0 || num_bytes < 0)
		return 0;
	if(num_bytes + start > BLOCK_SIZE)
		return 0;

	char* dest = blocks[block] + start;
	std::memcpy(dest, data, num_bytes);

	return 1;
}


int MemoryDevice::read(int block, char* dest, int num_bytes, int start)
{
	if(block < 0 || block >= NUM_BLOCKS)
		return 0;
	if(start < 0 || num_bytes < 0)
		return 0;
	if(num_bytes + start > BLOCK_SIZE)
		return 0;

	char* to_read = blocks[block] + start;
	std::memcpy(dest, to_read, num_bytes);
	return 1;
}


void createImage(const std::string& filepath)
{
	std::ofstream file;
	file.open(filepath);
	if(!file.is_open())
	{
		for(int i = 0; i < NUM_BLOCKS; i++)
		{
			file << blocks[i];
		}
		file.close();
	}
}

void restoreImage(const std::string& filepath)
{
	std::ifstream file;
	file.open(filepath);
	if(file.is_open())
	{
		
	}
}
