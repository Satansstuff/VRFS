#include "filesystem.h"
#include <iostream>
#include <unordered_map>

FileSystem::FileSystem()
{
	std::cout << "Bitmaps size:  " << AVAILABLE_BLOCKS + NUM_INODES << "\n";
	std::cout << "Bitmaps bytes: " << (AVAILABLE_BLOCKS + NUM_INODES)/8 << "\n";
}


FileSystem::~FileSystem()
{

}

void FileSystem::writeBlockBitmap()
{
	memory.write(0, block_bitmap.getMap(), block_bitmap.getNumBytes());
}
void FileSystem::readBlockBitmap()
{
	memory.read(0, block_bitmap.getMap(), block_bitmap.getNumBytes());
}
void FileSystem::writeInodeBitmap()
{
	memory.write(0, inode_bitmap.getMap(), inode_bitmap.getNumBytes(), block_bitmap.getNumBytes());
}
void FileSystem::readInodeBitmap()
{
	memory.read(0, inode_bitmap.getMap(), inode_bitmap.getNumBytes(), block_bitmap.getNumBytes());
}
int FileSystem::create(const std::string& file)
{
	return 0;
}
File FileSystem::open(const std::string& file)
{
	return 0;
}
int FileSystem::write(File file, const std::string& data)
{
	if(!file)
	{
		return filecodes::FILE_NOT_FOUND;
	}
	auto fptr = open_files.find(file);
	if(fptr == open_files.end())
	{
		return filecodes::FILE_NOT_OPEN;
	}
	Inode of = fptr->second;
	if(!of.attributes[1] || !of.attributes[2])
	{
		return filecodes::ACCESS_DENIED;
	}

}
int FileSystem::remove(const std::string& file)
{
	return 0;
}
int FileSystem::close(File file)
{
	return open_files.erase(file); 
}