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
	//char name[FILENAME_SIZE] = "/";
	strcpy(root_dir.name, "/");
	root_dir.addresses[0] = 0;
	root_dir.addresses[1] = 0;
	inode_bitmap.set(0, true);
	writeInodeBitmap();
	writeInodeToBlock(&root_dir);

	current_directory = 0;
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
std::string FileSystem::getCurrentDirectory()
{
	Inode *node = getInode(current_directory);
	std::string path = getPathTo(*node);
	delete node;
	return path;
}
std::string FileSystem::getPathTo(Inode inode)
{
	std::string result;
	Inode* curr_inode = new Inode(inode);
	while(true)
	{
		std::string current(curr_inode->name);
		result = current + "/" +  result;

		Inode* parent = getParent(&inode);
		if(parent)
		{
			if(parent->addresses[1] == 0)
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

Inode* FileSystem::getParent(Inode* node)
{
	return getInode(node->addresses[0]);
}

Inode* FileSystem::findChild(Inode* node, const std::string& child_name)
{
	Inode* result = nullptr;


	for(size_t i = 0; i < node->numBytes && i < NUM_ADDRESSES - 2; i++)
	{
		Inode* child = getInode(node->addresses[i + 2]);
		if(!child)
			continue; // FILESYSTEM BROKEN
		std::string name(child->name);
		if(name == child_name)
		{
			result = child;
			break;
		}
		delete child;
	}


	// check in indirect
	if(node->numBytes >= NUM_ADDRESSES - 2)
	{
		size_t num_in_block = node->numBytes - NUM_ADDRESSES + 2;
		int block = node->indirect_address + NUM_INODE_BLOCKS + 1;
		Address* buffer = new Address[num_in_block];
		memory.read(block, (char*)buffer, sizeof(Address)*num_in_block);
		for(size_t i = 0; i < num_in_block; i++)
		{
			Inode* child = getInode(buffer[i]);
			if(!child)
				continue; // FILESYSTEM BROKEN
			std::string name(child->name);
			if(name == child_name)
			{
				result = child;
				break;
			}
			delete child;
		}
	}


	return result;
}

Inode* FileSystem::parsePath(const std::string& path)
{
	Inode* current = nullptr;
	if(path[0] == '/')
	{
		// start with root
		current = getInode(0);
	}
	else
	{
		// start with current directory
		current = getInode(current_directory);
	}

	std::stringstream stream(path);
	std::string item;
	while(std::getline(stream, item, '/'))
	{
		if(item.size() == 0)
			continue;

		if(item.size() == 1 && item[0] == '.')
		{
			// current dir, dont move
		}
		else if(item.size() == 2 && item[0] == '.' && item[1] == '.')
		{
			// move to parent
			Inode* parent = getParent(current);

			if(!parent)
				return nullptr; // CRITICAL ERROR NO PARENT FOUND, FILESYSTEM BROKEN!!

			delete current;
			current = parent;
		}
		else
		{
			// search for item in current node, move to it
			Inode* child = findChild(current, item);

			if(!child)
				return nullptr; // CHILD NOT FOUND!

			delete current;
			current = child;
		}
	}
	return current;
}

int FileSystem::reserveFreeBlock()
{
	readBlockBitmap();
	int address = -1;
	for(size_t i = 0; i < AVAILABLE_BLOCKS; i++)
	{
		if(!block_bitmap[i])
		{
			address = i;
			block_bitmap.set(i, true);
			break;
		}
	}
	writeBlockBitmap();
	return address;
}

int FileSystem::reserveFreeInode()
{
	readInodeBitmap();
	std::cout << "Reserving inode\n";
	int address = -1;
	for(size_t i = 0; i < NUM_INODES; i++)
	{
		if(!inode_bitmap[i])
		{
			address = i;
			inode_bitmap.set(i, true);
			break;
		}
	}
	writeInodeBitmap();
	return address;
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
	/*if(open_files.emplace(getNewFileID(),*node))
		returncode = filecodes::OPEN_OK;
	else
		returncode = filecodes::FILE_ERROR;*/
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

int FileSystem::addAddressToDir(Inode* parent, Address child)
{
	if(parent->numBytes < NUM_ADDRESSES - 2 - 1)
	{
		parent->addresses[parent->numBytes + 2] = child;
	}
	else if(parent->numBytes == NUM_ADDRESSES - 2 - 1)
	{
		int block_adr = reserveFreeBlock();
		if(block_adr < 0)
			return DISK_FULL;

		parent->indirect_address = block_adr;

		int block = block_adr + NUM_INODE_BLOCKS + 1;
		memory.write(block, (char*)&child, sizeof(Address));
	}
	else
	{
		size_t num_in_block = parent->numBytes - NUM_ADDRESSES + 2;
		if(num_in_block >= BLOCK_SIZE)
			return FILE_ERROR;

		int block = parent->indirect_address + NUM_INODE_BLOCKS + 1;
		memory.write(block, (char*)&child, sizeof(Address), num_in_block*sizeof(Address));
	}
	parent->numBytes++;
	return 0;
}

int FileSystem::mkdir(const std::string& dir)
{
	int last_slash = -1;
	for(unsigned i = 0; i < dir.size(); i++)
		if(dir[i] == '/')
			last_slash = i;


	std::string to_create;
	std::string dir_path;


	Inode* parent = nullptr;

	if(last_slash == -1)
	{
		to_create = dir;
		parent = getInode(current_directory);
	}
	else
	{
		to_create = dir.substr(last_slash, dir.size()-last_slash);
		dir_path = dir.substr(0, last_slash);
		parent = parsePath(dir_path);
	}

	//std::cout << "DIR: " << dir << "\n";
	//std::cout << "TO_CREATE: " << to_create << "\n";
	//std::cout << "DIR_PATH: " << dir_path << "\n";

	if(!parent)
	{
		// could not find parent
		//std::cout << "Could not find parent\n";
		return 0;
	}

	int address = reserveFreeInode();
	if(address < 0)
	{
		//std::cout << "Could not reserve inode " << address << " \n";
		return 0; // max number of files and directories
	}

	Address adr = address;

	Inode new_dir;
	new_dir.attributes.set(0, true);
	new_dir.attributes.set(1, true);
	new_dir.attributes.set(2, true);
	strcpy(new_dir.name, to_create.c_str());
	new_dir.addresses[0] = parent->addresses[1];
	new_dir.addresses[1] = adr;

	addAddressToDir(parent, adr);


	writeInodeToBlock(parent);
	writeInodeToBlock(&new_dir);

	delete parent;

	return 1;
}

int FileSystem::cd(const std::string& dir)
{
	Inode* to_enter = parsePath(dir);
	if(!to_enter)
		return 0; // nothing found

	if(!to_enter->attributes[0])
		return 0; // not a directory

	current_directory = to_enter->addresses[1];
	return 1;
}

std::string FileSystem::ls(const std::string &dir)
{
	std::string result;

	Inode* to_view = nullptr;

	if(dir.size() == 0)
	{
		to_view = getInode(current_directory);
	}
	else
	{
		to_view = parsePath(dir);
		if(!to_view)
			return "ERROR: '" + dir + "' not found";
		std::string path = getPathTo(*to_view);
		if(!to_view->attributes[0])
			return "ERROR: '" + path + "' not a directory";
	}


	for(size_t i = 0; i < to_view->numBytes && i < NUM_ADDRESSES - 2; i++)
	{
		Inode* child = getInode(to_view->addresses[i + 2]);
		if(!child)
			continue; // FILESYSTEM BROKEN
		std::string name(child->name);

		result += name + "\n";

		delete child;
	}


	// check in indirect
	if(to_view->numBytes >= NUM_ADDRESSES - 2)
	{
		size_t num_in_block = to_view->numBytes - NUM_ADDRESSES + 2;
		int block = to_view->indirect_address + NUM_INODE_BLOCKS + 1;
		Address* buffer = new Address[num_in_block];
		memory.read(block, (char*)buffer, sizeof(Address)*num_in_block);
		for(size_t i = 0; i < num_in_block; i++)
		{
			Inode* child = getInode(buffer[i]);
			if(!child)
				continue; // FILESYSTEM BROKEN
			std::string name(child->name);

			result += name + "\n";

			delete child;
		}
	}
	return result;
}







