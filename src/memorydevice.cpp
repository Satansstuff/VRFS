#include "memorydevice.h"

#include <cstring>

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

int MemoryDevice::write(int block, char* to_write, int num_bytes, int start)
{
	if(block < 0 || blocks >= NUM_BLOCKS)
		return 1;
	if(start < 0 || num_bytes < 0)
		return 1;
	if(num_bytes + start > BLOCK_SIZE)
		return 1;

	char* dest = blocks[block] + start;
	std::memcpy(dest, to_write, num_bytes);

	return 0;
}
