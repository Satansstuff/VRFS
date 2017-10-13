#pragma once

#include <unordered_map>
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


#define File unsigned long

class FileSystem
{
	struct OpenFile
	{
		
		char* buffer;
		Inode inode;
	};
	MemoryDevice memory;

	Bitmap<AVAILABLE_BLOCKS> block_bitmap;
	unsigned int current_block;

	Bitmap<NUM_INODES> inode_bitmap;
	unsigned int current_inode;

	std::unordered_map<File, OpenFile> open_files;

	void writeBlockBitmap();
	void readBlockBitmap();

	void writeInodeBitmap();
	void readInodeBitmap();
public:

	FileSystem();

	~FileSystem();

	int create(const std::string& file);

	File open(const std::string& file);

	int write(File file, const std::string& data);

	

	int close(File file);

};
