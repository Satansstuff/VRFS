#include "filesystem.h"
#include <iostream>
#include <unordered_map>
#include <sstream>

FileSystem::FileSystem()
{
	std::cout << "Bitmaps size:  " << AVAILABLE_BLOCKS + NUM_INODES << "\n";
	std::cout << "Bitmaps bytes: " << (AVAILABLE_BLOCKS + NUM_INODES)/8 << "\n";
	Inode root_dir;
	root_dir.attributes.set(0,true);
	root_dir.attributes.set(1,true);
	root_dir.attributes.set(2,true);
	root_dir.name = "/";
	root_dir.addresses[0] = 0;
	root_dir.addresses[1] = 0;
	inode_bitmap.set(0, true);
	writeInodeBitmap();
	writeInodeToBlock(&root_dir);
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
unsigned long FileSystem::getNewFileID()
{
	return counter++;
}
Inode* FileSystem::parsePath(const std::string& path)
{
	int last_slash = -1;
	for(unsigned i = 0; i < path.size(); i++)
		if(path[i] == '/')
			last_slash = i;

	if(last_slash == -1 || path.size() < 3)
		return nullptr;

	std::string to_find = path.substr(last_slash, path.size()-last_slash);

	std::string dir_path = path.substr(0, last_slash);

	if(path[0] == '/')
	{
		// absolute
	}
	else if(path[0] == '.' && path[1] == '/')
	{
		// relative current dir
		dir_path = getPathTo(current_directory) + dir_path.substr(2, last_slash - 2);
	}
	else if(path[0] == '.' && path[1] == '.' && path[2] == '/')
	{
		// relative parent
		Inode* parent = getInode(current_directory.addresses[0]);
		if(!parent)
			return nullptr;
		std::string left = getPathTo(*parent);
		dir_path = left + dir_path.substr(2, last_slash - 2);
		delete parent;
	}
	else
	{
		// relative current dir
		dir_path = getPathTo(current_directory) + dir_path;
	}

	Inode* result = nullptr;

	Inode* to_search = getDirectoryFromAbsolute(dir_path);

	if(!to_search)
	{
		// Directory not found
	}
	else
	{
		if(to_search->numBytes < NUM_ADDRESSES - 2)
		{
			// don't use indirect
			for(size_t i = 0; i < to_search->numBytes; i++)
			{
				Inode* to_check = getInode(to_search->addresses[i+2]);
				std::string name(to_check->name);
				if(name == to_find)
				{
					result = to_check;
					break;
				}
			}
		}
		else
		{
			// use indirect
			// TODO: implement
		}
	}
	return result;
}

Inode* FileSystem::getDirectoryFromAbsolute(const std::string& dir)
{
	std::stringstream stream(dir);
	std::string item;

	// start with root (0)
	Inode* cur_dir = getInode(0);
	while(std::getline(stream, item, '/'))
	{
		if(item.size() == 0)
			continue;

		// numBytes is number of addresses used here
		if(cur_dir->numBytes < NUM_ADDRESSES - 2)
		{
			// only in addresses
			for(size_t i = 0; i < cur_dir->numBytes;i++)
			{
				Inode* to_check = getInode(cur_dir->addresses[i+2]);
				std::string name(to_check->name);
				if(name == item && to_check->attributes[0])
				{
					delete cur_dir;
					cur_dir = to_check;
					break;
				}
				else
				{
					delete to_check;
				}
			}
			// can only get here if nothing found
			return nullptr;
		}
		else
		{
			// use indirect
			// TODO: implement pl0x
		}
	}
	return cur_dir;
}




int FileSystem::create(const std::string& file)
{
	return 0;
}
File FileSystem::open(const std::string& file)
{
	Inode *node = this->parsePath(file);
	int returncode = filecodes::FILE_ERROR;
	if(!node)
		returncode = filecodes::FILE_NOT_FOUND;
	else if(!node->attributes[1])
		returncode = filecodes::ACCESS_DENIED;
	if(open_files.emplace(getNewFileID(),*node))
		returncode = filecodes::OPEN_OK;
	else
		returncode = filecodes::FILE_ERROR;
	delete node;
	return returncode;
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
	size_t datasize = data.length();
	unsigned blocksNeeded = std::ceil(datasize / BLOCK_SIZE);
	if(blocksNeeded > AVAILABLE_BLOCKS)
	{
		return filecodes::DISK_FULL;
	}
	const char *writable = data.c_str();
	if(blocksNeeded == 1)
	{
		for(size_t i = current_block; i < AVAILABLE_BLOCKS; i++)
		{
			if(!block_bitmap[i])
			{
				block_bitmap.set(i, true);
				memory.write(i + NUM_INODE_BLOCKS + 1, writable, datasize, 0);
				current_block++;
				break;
			}
			else
			{
				current_block++;
			}
		}
	}
	else if(blocksNeeded < NUM_ADDRESSES)
	{
		size_t dataToWrite = datasize;
		for(size_t i = current_block; i < AVAILABLE_BLOCKS; i++)
		{
			if(dataToWrite > 0)
			{
				if(block_bitmap[i])
				{
					
				}
				else
				{
					current_block++;
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		//Indirect needed
	}
	writeInodeToBlock(&of);
	return filecodes::WRITE_OK;
}
int FileSystem::writeInodeToBlock(Inode *node)
{
		int block = node->addresses[1] / INODES_PER_BLOCK + 1;
		int inodes_out = node->addresses[1] % INODES_PER_BLOCK;
		memory.write(block, (char*)node, sizeof(Inode), inodes_out*sizeof(Inode));
		return 1;
}
int FileSystem::remove(const std::string& file)
{
	Inode *toRemove = parsePath(file);
	auto address = toRemove->addresses[1];
	if(!toRemove)
	{
		return filecodes::FILE_NOT_FOUND;
	}
	if(!toRemove->attributes[2])
	{
		return filecodes::ACCESS_DENIED;
	}
	if(toRemove->attributes[0] && toRemove->numBytes != 0)
	{
		return filecodes::NOT_EMPTY_FOLDER;
	}
	else if(toRemove->attributes[0] && toRemove->numBytes == 0)
	{
		//Det är en tom mapp, ta bort.
		return filecodes::DELETE_OK;
	}
	//Sök igenom openfiles så man inte tar bort en fil som är öppen
	for(auto &inode : open_files)
	{
		if(inode.second.addresses[1] == address)
		{
			return filecodes::FILE_IS_OPEN;
		}
	}
	/*
		Sätt bitmap både inode och block till false så de kan återanvändas
		måste veta om det är indirect och hur mycket jadijadijadi
	*/
	unsigned numBlocks = toRemove->numBytes / BLOCK_SIZE;
	if(numBlocks > NUM_ADDRESSES - 2)
	{
		//direct
	}
	else
	{
		//indirect
	}
	return filecodes::DELETE_OK;

}
int FileSystem::close(File file)
{
	return open_files.erase(file); 
}
