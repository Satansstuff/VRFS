#pragma once

#include <bitset>
#include "memorydevice.h"

// number of blocks that contain inodes
#define NUM_INODE_BLOCKS 4

#define NUM_ADDRESSES 8
#define FILENAME_SIZE 32

#define Address unsigned short



struct INode {
	// [0]: is_directory
	// [1]: can read
	// [2]: can write
	std::bitset<3> attributes;                              // 1 byte
	char name[FILENAME_SIZE];                               // FILENAME_SIZE bytes
	// if num_blocks > NUM_ADDRESSES then indirect is used
	unsigned short num_blocks;                              // 2 bytes
	// if file then block-addresses
	// if directory then inode-addresses
	Address addresses[NUM_ADDRESSES];                       // NUM_ADDRESSES * 2 bytes
	// always block-address
	Address indirect_address;                               // 2 bytes
};


constexpr int AVAILABLE_BLOCKS = NUM_BLOCKS - 1 - NUM_INODE_BLOCKS;
constexpr int NUM_INODES = NUM_INODE_BLOCKS * BLOCK_SIZE/sizeof(INode);

class FileSystem
{
	MemoryDevice memory;

	std::bitset<AVAILABLE_BLOCKS> block_bitmap;

	std::bitset<NUM_INODES> inode_bitmap;
public:

	FileSystem();

	~FileSystem();


};
