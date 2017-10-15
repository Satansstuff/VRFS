#pragma once

#include <unordered_map>
#include "bitmap.h"
#include "memorydevice.h"

// number of blocks that contain inodes
#define NUM_INODE_BLOCKS 6

#define NUM_ADDRESSES 8
#define FILENAME_SIZE 40

enum filecodes
{
	DELETE_OK = 5,
	CREATE_OK = 4,
	OPEN_OK = 3,
	READ_OK = 2,
	WRITE_OK = 1,
	FILE_NOT_FOUND = 0,
	FILE_NOT_OPEN = -1,
	ACCESS_DENIED = -2,
	DISK_FULL = -3,
	FILE_IS_OPEN = -4,
	NOT_EMPTY_FOLDER = -5
};

#define Address unsigned short

struct Inode
{
	// [0]: is directory
	// [1]: can read
	// [2]: can write
	Bitmap<3> attributes;

	char name[FILENAME_SIZE];

	//if numBytes / BLOCK_SIZE > NUM_ADDRESSES, indirect is used
	unsigned int numBytes = 0;

	// if file then block-addresses
	// if directory then inode-addresses, [0]: is parent, [1] is itself
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
	MemoryDevice memory;

	Inode current_directory;

	// true if block is used
	Bitmap<AVAILABLE_BLOCKS> block_bitmap;
	unsigned int current_block;

	Bitmap<NUM_INODES> inode_bitmap;
	unsigned int current_inode = 1;

	std::unordered_map<File, Inode> open_files;

	void writeBlockBitmap();
	void readBlockBitmap();

	void writeInodeBitmap();
	void readInodeBitmap();

	std::string getPathTo(Inode inode);
	Inode* getInode(Address address);
	Inode* getParent(Inode* node);
	Inode* findChild(Inode* node, const std::string& child);

	Inode* parsePath(const std::string& path);

	int writeInodeToBlock(Inode *node);
public:

	FileSystem();

	~FileSystem();

	int create(const std::string& file);

	int remove(const std::string& file);

	int chmod(const std::string &str, bool r, bool w);
	int mkdir(const std::string &dir);

	File open(const std::string& file);

	int write(File file, const std::string& data);

	int close(File file);
};
