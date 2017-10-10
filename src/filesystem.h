'#pragma once

#include "bitmap.h"
#include "memorydevice.h"

// number of blocks that contain inodes
#define NUM_INODE_BLOCKS 6

#define NUM_ADDRESSES 8
#define FILENAME_SIZE 42

#define Address unsigned short

struct Inode {
	// [0]: is directory
	// [1]: can read
	// [2]: can write
	Bitmap<3> attributes;

	char name[FILENAME_SIZE];

	// if num_blocks > NUM_ADDRESSES then indirect is used
	unsigned short num_blocks;

	// if file then block-addresses
	// if directory then inode-addresses
	Address addresses[NUM_ADDRESSES];

	// always block-address
	Address indirect_address;
};

// -1 since first block is reserved for bitmaps
constexpr int AVAILABLE_BLOCKS = NUM_BLOCKS - 1 - NUM_INODE_BLOCKS;
constexpr int INODES_PER_BLOCK = BLOCK_SIZE/sizeof(Inode);
constexpr int NUM_INODES = NUM_INODE_BLOCKS * INODES_PER_BLOCK;

class FileSystem
{
	MemoryDevice memory;

	Bitmap<AVAILABLE_BLOCKS> block_bitmap;

	Bitmap<NUM_INODES> inode_bitmap;

	void writeBlockBitmap();
	void readBlockBitmap();

	void writeInodeBitmap();
	void readInodeBitmap();
public:

	FileSystem();

	~FileSystem();

	

};
