#pragma once

#include <unordered_map>
#include "bitmap.h"
#include "memorydevice.h"

// number of blocks that contain inodes
#define NUM_INODE_BLOCKS 6

#define NUM_ADDRESSES 8
#define FILENAME_SIZE 40

#define FILE_INVALID 0
enum filecodes
{
	OPEN_OK = 5,
	DELETE_OK = 4,
	CREATE_OK = 3,
	READ_OK = 2,
	WRITE_OK = 1,
	NOT_FOUND = 0,
	FILE_NOT_OPEN = -1,
	ACCESS_DENIED = -2,
	DISK_FULL = -3,
	FILE_IS_OPEN = -4,
	NOT_EMPTY_FOLDER = -5,
	FILE_ERROR = -6,
	CANT_DELETE_ROOT = -7,
	ALREADY_EXISTS = -8,
	MAX_FILES = -9,
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

	Address current_directory;

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

	int addAddressToDir(Inode* parent, Address child);
	int removeAddressFromDir(Inode* parent, Address child);
	int removeFile(Inode* file);
	int removeDir(Inode* dir);

	// returns -1 if full
	int reserveFreeBlock();
	int reserveFreeInode();
	size_t numFreeBlocks();
	void freeBlock(Address block);
	void freeInode(Address inode);

	int writeInodeToBlock(Inode *node);
	File counter = 1;
	File getNewFileID();
public:

	FileSystem();

	~FileSystem();

	int create(const std::string& file);
	std::string getCurrentDirectory();
	int remove(const std::string& file);

	int chmod(const std::string &str, bool r, bool w);
	int mkdir(const std::string &dir);
	int cd(const std::string &dir);
	std::string ls(const std::string &dir = "");
	std::string currDirName();

	File open(const std::string& file);

	int write(File file, const std::string& data);

	int close(File file);
};
