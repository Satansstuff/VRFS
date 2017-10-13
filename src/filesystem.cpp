#include "filesystem.h"

#include <iostream>

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

Inode* FileSystem::getInode(Address address)
{
	Inode* result = nullptr;
	if(inode_bitmap[address]) {
		int block = address / INODES_PER_BLOCK + 1;
		int inodes_in = address % INODES_PER_BLOCK;

		result = new Inode();
		memory.read(block, (char*)result, sizeof(Inode), inodes_in*sizeof(Inode));
	}
	return result;
}


std::string FileSystem::getPathTo(Inode inode)
{
	std::string result;
	Inode* curr_inode = new Inode(inode);
	while(true)
	{
		std::string current(curr_inode->name);
		result = current + "/" +  result;

		Inode* parent = getInode(inode.addresses[0]);
		if(parent)
		{
			std::string parent_name(parent->name);
			if(parent_name == "/")
			{
				result = "/" + result;
				return result;
			}
			delete curr_inode;
			curr_inode = parent;
		}
		else
		{
			// HORRIBLE ERROR, parent not found!
			return "ERROR";
		}
	}
}

Inode* FileSystem::parseFilePath(const std::string& path)
{
	int last_slash = -1;
	for(unsigned i = 0; i < path.size(); i++)
		if(path[i] == '/')
			last_slash = i;

	if(last_slash == -1 || path.size() < 3)
		return nullptr;

	std::string dir_path = path.substr(0, last_slash);

	if(path[0] == '/')
	{
		// absolute
	}
	else if(path[0] == '.' && path[1] == '/')
	{
		// relative current dir
		dir_path = getPathTo(current_directory) + dir_path.substr(1, last_slash - 1);
	}
	else if(path[0] == '.' && path[1] == '.' && path[2] == '/')
	{
		// relative parent
		std::string left 
		dir_path = dir_path.substr(2, last_slash - 2);
	}
	else
	{
		// relative current dir
	}
}


