#pragma once

#include <bitset>
#include "memorydevice.h"

#define NUM_ADDRESSES 8
#define FILENAME_SIZE 8

#define Address unsigned short;

struct INode {
	// [0]: is_directory
	// [1]: can read
	// [2]: can write
	std::bitset<3> attributes;            // 1 byte
	char name[FILENAME_SIZE]              // FILENAME_SIZE bytes
	unsigned short num_blocks;            // 2 bytes
	// if file then block-addresses
	// if directory then inode-addresses
	Address addresses[NUM_ADDRESSES];     // NUM_ADDRESSES * 2 bytes
	// always block-address
	Address indirect_address;             // 2 bytes
};


class FileSystem
{
	MemoryDevice memory;

public:

	FileSystem();

	~FileSystem();


};
