#include "filesystem.h"

#include <iostream>

FileSystem::FileSystem()
{
	std::cout << "NUM_INODES: " << NUM_INODES << "\n";
	std::cout << "sizeof(Inode): " << sizeof(Inode) << "\n";
}


FileSystem::~FileSystem()
{

}

void FileSystem::writeBlockBitmap()
{
	//memory.write(
}
void FileSystem::readBlockBitmap()
{

}
void FileSystem::writeInodeBitmap()
{

}
void FileSystem::readInodeBitmap()
{

}


