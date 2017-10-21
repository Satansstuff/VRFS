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
FileSystem::FileSystem(const std::string &path)
{
	memory.restoreImage(path);
}
FileSystem::~FileSystem()
{

}
void FileSystem::saveToFile(const std::string &path)
{
	memory.createImage(path);
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
		if(curr_inode->addresses[1] == 0)
		{
			if(result.size() == 0)
				result += "/";
			delete curr_inode;
			return result;
		}
		else
		{
			std::string name(curr_inode->name);
			result = "/" + name + result;
			Inode* parent = getInode(curr_inode->addresses[0]);
			delete curr_inode;
			curr_inode = parent;
		}
	}
}

File FileSystem::getNewFileID()
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
	if(path.size() == 0)
	{
		current = getInode(current_directory);
	}
	else if(path[0] == '/')
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
	//std::cout << "Reserving inode\n";
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
	Inode *testnode = parsePath(file);
	if(testnode)
	{
		delete testnode;
		//Filen finns redan
		return ALREADY_EXISTS;
	}
	int inode_addr = reserveFreeInode();
	if(!inode_addr)
	{
		//Fullt
		return MAX_FILES;
	}
	Inode node;
	node.attributes.set(0, false);
	node.addresses[1] = inode_addr;
	std::size_t last = file.find_last_of("/\\");

	Inode* parent = nullptr;

	if(last > file.size())
	{
		last = 0;
		parent = parsePath(file.substr(0,last));
	}
	else
	{
		parent = parsePath(file.substr(0,last));
	}

	if(!parent)
	{
		//Föräldern finns inte
		delete parent;
		return FILE_ERROR;
	}

	std::string to_create = file.substr(last);
	node.addresses[0] = parent->addresses[1];
	strcpy(node.name, to_create.c_str());

	addAddressToDir(parent, node.addresses[1]);
	writeInodeToBlock(parent);
	delete parent;
	writeInodeToBlock(&node);
	return CREATE_OK;
}


File FileSystem::open(const std::string& file)
{
	Inode *node = this->parsePath(file);
	if(!node)
		return FILE_INVALID;
	else if(!node->attributes[1])
		return FILE_INVALID;
	File newID = getNewFileID();
	open_files.emplace(newID,*node);
	delete node;
	return newID;
}
int FileSystem::write(File file, const std::string& data)
{
	if(!file)
	{
		return filecodes::NOT_FOUND;
	}
	auto fptr = open_files.find(file);
	if(fptr == open_files.end())
	{
		return filecodes::FILE_NOT_OPEN;
	}
	Inode of = fptr->second;
	if(!of.attributes[2])
	{
		return filecodes::ACCESS_DENIED;
	}
	size_t datasize = data.length() + 1;
	of.numBytes = datasize;
	unsigned blocksNeeded = std::ceil((double)datasize / BLOCK_SIZE);
	if(blocksNeeded > numFreeBlocks())
	{
		return filecodes::DISK_FULL;
	}
	const char *writable = data.c_str();

	size_t blocks_in = 0;
	for(size_t i = 0; i < blocksNeeded && i < NUM_ADDRESSES - 2; i++)
	{
		int new_block = reserveFreeBlock();
		if(new_block < 0)
			return DISK_FULL;

		int index = i + 2;
		of.addresses[index] = new_block;
		int block = new_block + NUM_INODE_BLOCKS + 1;
		int num_bytes = 0;
		if(blocks_in < blocksNeeded-1)
		{
			num_bytes = BLOCK_SIZE;
		}
		else
		{
			num_bytes = datasize % BLOCK_SIZE;
		}
		memory.write(block, &writable[blocks_in*BLOCK_SIZE], num_bytes);
		blocks_in++;
	}
	if(blocksNeeded > NUM_ADDRESSES - 2)
	{
		//indirect needed
		int indirect_block = reserveFreeBlock();
		if(indirect_block < 0)
			return DISK_FULL;
		of.indirect_address = indirect_block;
		for(size_t i = 0; i < blocksNeeded - (NUM_ADDRESSES - 2); i++)
		{
			int new_block = reserveFreeBlock();
			if(new_block < 0)
				return DISK_FULL;

			Address address = new_block;
			memory.write(indirect_block, (char*)&address, sizeof(Address), sizeof(Address)*i);

			int block = address + NUM_INODE_BLOCKS + 1;
			int num_bytes = 0;
			if(blocks_in < blocksNeeded-1)
				num_bytes = BLOCK_SIZE;
			else
				num_bytes = datasize % BLOCK_SIZE;

			memory.write(block, &writable[blocks_in*BLOCK_SIZE], num_bytes);
			blocks_in++;
		}
	}
	writeInodeToBlock(&of);
	return filecodes::WRITE_OK;
}

std::string FileSystem::read(File file)
{
	std::string result;
	if(!file)
	{
		return std::to_string(filecodes::NOT_FOUND);
	}
	auto fptr = open_files.find(file);
	if(fptr == open_files.end())
	{
		return std::to_string(filecodes::FILE_NOT_OPEN);
	}
	Inode of = fptr->second;
	if(!of.attributes[1])
	{
		return std::to_string(filecodes::ACCESS_DENIED);
	}
	size_t datasize = of.numBytes;
	unsigned blocksNeeded = std::ceil((double)datasize / BLOCK_SIZE);
	size_t blocks_in = 0;

	char* buffer = new char[BLOCK_SIZE];
	for(size_t i = 0; i < blocksNeeded && i < NUM_ADDRESSES - 2; i++)
	{
		int index = i + 2;
		int block = of.addresses[index] + NUM_INODE_BLOCKS + 1;
		int num_bytes = 0;
		if(blocks_in < blocksNeeded-1)
			num_bytes = BLOCK_SIZE;
		else
			num_bytes = datasize % BLOCK_SIZE;

		memory.read(block, buffer, num_bytes);
		std::string str(buffer);
		result += str;
		blocks_in++;
	}
	if(blocksNeeded > NUM_ADDRESSES - 2)
	{
		//indirect
		Address indirect_block = of.indirect_address + NUM_INODE_BLOCKS + 1;
		for(size_t i = 0; i < blocksNeeded - (NUM_ADDRESSES - 2); i++)
		{
			Address address;
			memory.read(indirect_block, (char*)&address, sizeof(Address), sizeof(Address)*i);

			int block = address + NUM_INODE_BLOCKS + 1;
			int num_bytes = 0;
			if(blocks_in < blocksNeeded-1)
				num_bytes = BLOCK_SIZE;
			else
				num_bytes = datasize % BLOCK_SIZE;

			memory.read(block, buffer, num_bytes);
			std::string str(buffer);
			result += str;
			blocks_in++;
		}
	}
	return result;
}


size_t FileSystem::numFreeBlocks()
{
	readBlockBitmap();
	size_t result = 0;
	for(int i = 0; i < AVAILABLE_BLOCKS; i++)
	{
		if(!block_bitmap[i])
			result++;
	}
	return result;
}

int FileSystem::writeInodeToBlock(Inode *node)
{
	int block = node->addresses[1] / INODES_PER_BLOCK + 1;
	int inodes_out = node->addresses[1] % INODES_PER_BLOCK;
	memory.write(block, (char*)node, sizeof(Inode), inodes_out*sizeof(Inode));
	return 1;
}

void FileSystem::freeBlock(Address block)
{
	readBlockBitmap();
	block_bitmap.set(block, false);
	writeBlockBitmap();
}
void FileSystem::freeInode(Address inode)
{
	readInodeBitmap();
	inode_bitmap.set(inode, false);
	writeInodeBitmap();
}

int FileSystem::removeAddressFromDir(Inode* parent, Address child)
{
	if(!parent)
		return NOT_FOUND;
	if(parent->numBytes == 0)
		return DELETE_OK;


	size_t num_dir_addr = NUM_ADDRESSES - 2;

	bool found = false;
	bool indirect = false;
	size_t index = 0;
	for(size_t i = 0; i < parent->numBytes && i < num_dir_addr; i++)
	{
		if(parent->addresses[i+2] == child)
		{
			index = i+2;
			found = true;
			break;
		}
	}
	if(!found && parent->numBytes > num_dir_addr)
	{
		// search in indirect
		size_t num_in_block = parent->numBytes - NUM_ADDRESSES + 2;
		int block = parent->indirect_address + NUM_INODE_BLOCKS + 1;
		Address* buffer = new Address[num_in_block];
		memory.read(block, (char*)buffer, sizeof(Address)*num_in_block);
		for(size_t i = 0; i < num_in_block; i++)
		{
			if(buffer[i] == child)
			{
				index = i;
				found = true;
				indirect = true;
				break;
			}
		}
		delete buffer;
	}

	if(found)
	{
		if(parent->numBytes <= num_dir_addr)
		{
			//std::cout << "REMOVE 1, index: "<< index << std::endl;
			//std::cout << parent->addresses[index] << " = " << parent->addresses[parent->numBytes-1 + 2] << std::endl;
			parent->addresses[index] = parent->addresses[parent->numBytes-1 + 2];
		}
		else if(parent->numBytes == num_dir_addr + 1)
		{
			if(!indirect)
			{
				//std::cout << "REMOVE 2" << std::endl;
				Address last;
				int block = parent->indirect_address + NUM_INODE_BLOCKS + 1;
				memory.read(block, (char*)&last, sizeof(Address));
				parent->addresses[index] = last;
			}
			//std::cout << "REMOVE 3" << std::endl;

			freeBlock(parent->indirect_address);
		}
		else
		{
			//std::cout << "REMOVE 4" << std::endl;
			Address last;
			size_t num_in_block = parent->numBytes - NUM_ADDRESSES + 2;
			int block = parent->indirect_address + NUM_INODE_BLOCKS + 1;
			memory.read(block, (char*)&last, sizeof(Address), sizeof(Address)*(num_in_block-1));

			if(indirect)
			{
				//std::cout << "REMOVE 5" << std::endl;
				memory.write(block, (char*)&last, sizeof(Address), sizeof(Address)*index);
			}
			else
			{
				//std::cout << "REMOVE 6" << std::endl;
				parent->addresses[index] = last;
			}
		}
		parent->numBytes--;
		writeInodeToBlock(parent);
	}
	else
	{
		return NOT_FOUND;
	}

	return DELETE_OK;
}

int FileSystem::removeDir(Inode* dir)
{
	if(dir->numBytes > 0)
		return NOT_EMPTY_FOLDER;
	if(dir->addresses[1] == 0)
		return CANT_DELETE_ROOT;


	freeInode(dir->addresses[1]);
	Inode* parent = getInode(dir->addresses[0]);
	removeAddressFromDir(parent, dir->addresses[1]);
	delete parent;
	return DELETE_OK;
}

int FileSystem::removeFile(Inode* toRemove)
{
	auto address = toRemove->addresses[1];
	if(!toRemove->attributes[2])
	{
		return filecodes::ACCESS_DENIED;
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

int FileSystem::remove(const std::string& file)
{
	Inode *toRemove = parsePath(file);
	if(!toRemove)
	{
		return filecodes::NOT_FOUND;
	}

	if(toRemove->attributes[0])
	{
		return removeDir(toRemove);
	}
	else
	{
		return removeFile(toRemove);
	}
}

int FileSystem::close(File file)
{
	return open_files.erase(file); 
}

int FileSystem::addAddressToDir(Inode* parent, Address child)
{
	if(parent->numBytes <= NUM_ADDRESSES - 2 - 1)
	{
		parent->addresses[parent->numBytes + 2] = child;
	}
	else if(parent->numBytes == NUM_ADDRESSES - 2)
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
	Inode* same_name = parsePath(dir);
	if(same_name)
	{
		delete same_name;
		return ALREADY_EXISTS;
	}

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
		return FILE_ERROR;
	}

	int address = reserveFreeInode();
	if(address < 0)
	{
		//std::cout << "Could not reserve inode " << address << " \n";
		return MAX_FILES; // max number of files and directories
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

	return CREATE_OK;
}

int FileSystem::cd(const std::string& dir)
{
	std::string directory = dir;
	if(dir.size() == 0)
		directory = "/";
	Inode* to_enter = parsePath(directory);
	if(!to_enter)
		return 0; // nothing found

	if(!to_enter->attributes[0])
		return 0; // not a directory

	current_directory = to_enter->addresses[1];
	delete to_enter;
	return 1;
}

std::string FileSystem::ls(const std::string &dir)
{
	std::string dirs;
	std::string files;

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
		if(child->attributes[0])
			dirs += "\n\t" + name;
		else
			files += "\n\t" + name;

		delete child;
	}


	// check in indirect
	if(to_view->numBytes > NUM_ADDRESSES - 2)
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

			if(child->attributes[0])
				dirs += "\n\t" + name;
			else
				files += "\n\t" + name;

			delete child;
		}
		delete buffer;
	}

	delete to_view;
	std::string result;
	if(dirs.size() != 0)
		dirs = "Directories:" + dirs + "\n";
	if(files.size() != 0)
		files = "Files:" + files + "\n";

	return dirs + files;
}

std::string FileSystem::currDirName()
{
	Inode* current = getInode(current_directory);
	std::string result(current->name);
	delete current;
	return result;
}

int FileSystem::chmod(const std::string &str, bool r, bool w)
{
	Inode *node = parsePath(str);
	if(node->attributes[0])
	{
		delete node;
		return -1;
	}
	else
	{
		node->attributes.set(1,r);
		node->attributes.set(2,w);
	}
	writeInodeToBlock(node);
	delete node;
	return 1;
}
