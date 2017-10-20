#pragma once

#include <string>

#define NUM_BLOCKS 250
#define BLOCK_SIZE 512

class MemoryDevice
{
	// array of blocks
	// each block is an array of bytes
	char** blocks;

public:

	MemoryDevice();

	~MemoryDevice();

	/*
	* writes "num_bytes" bytes to the device starting at "start"
	* returns error code:
	*	1 if success
	*	0 if trying to write outside of device in some way
	 */
	int write(int block, const char* data, int num_bytes, int start = 0);

	int read(int block, char* dest, int num_bytes, int start = 0);

	void createImage(const std::string& dir);
	void restoreImage(const std::string& dir);
};
