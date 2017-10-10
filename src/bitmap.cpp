#include "bitmap.h"
#include "math.h"

Bitmap::Bitmap(int _size)
{
	size = _size;
	num_bytes = std::ceil(size/8.0);
	map = new char[num_bytes];
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
