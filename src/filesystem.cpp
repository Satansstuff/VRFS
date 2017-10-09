#include "filesystem.h"

#include <iostream>

FileSystem::FileSystem()
{
	std::cout << "NUM_INODES: " << NUM_INODES << "\n";
	std::cout << "sizeof(INode): " << sizeof(INode) << "\n";
}


FileSystem::~FileSystem()
{

}
