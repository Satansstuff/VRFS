#pragma once

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
	*	0 if success
	*	1 if trying to write outside of device in some way
	 */
	int write(int block, char* data, int num_bytes, int start = 0);

	int read(int block, char* dest, int num_bytes, int start = 0);
};
