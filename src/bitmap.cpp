#include "bitmap.h"

#include <cstring>
#include <math.h>

Bitmap::Bitmap(int _size)
{
	size = _size;
	num_bytes = std::ceil(size/8.0);
	map = new char[num_bytes];
	clear();
}

Bitmap::~Bitmap()
{
	delete[] map;
}

int Bitmap::getSize()
{
	return size;
}

int Bitmap::getNumBytes()
{
	return num_bytes;
}

char* Bitmap::getMap()
{
	return map;
}

void Bitmap::clear()
{
	memset(map, 0, num_bytes);
}

bool Bitmap::operator[](int i)
{
	if(i >= size || i < 0)
		return false;
	char cell = map[i/8];
	char masked = cell & 1 << i%8;
	return masked != 0;
}

void Bitmap::set(int i, bool val)
{
	if(val)
	{
		map[i/8] |= 1 << i%8;
	}
	else
	{
		map[i/8] &= ~(1 << i%8);
	}
}
