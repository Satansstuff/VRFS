#include "bitmap.h"

Bitmap::Bitmap(int _size)
{
	size = _size;
	map = new char[size];
}

Bitmap::~Bitmap()
{
	delete[] map;
}

int Bitmap::getSize()
{
	return size;
}

char* Bitmap::getMap()
{
	return map;
}

bool Bitmap::operator[](int i)
{
	char cell = map[i/8];
	char masked = cell & 1 << 1%8;
	return masked != 0;
}

void Bitmap::set(int i, bool val)
{
	if(val)
	{
		map[i%8] |= 1 << i%8;
	}
	else
	{
		map[i%8] &= ~(1 << i%8);
	}
}
