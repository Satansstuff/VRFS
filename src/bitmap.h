#pragma once

#include <cstring>
#include <math.h>

template<int T>
class Bitmap {
	char map[(int)std::ceil(T/8.0)];
public:
	Bitmap();
	~Bitmap();

	int getSize();
	int getNumBytes();
	char* getMap();

	void clear();

	// returns value at i:th bit
	// returns false if outside range
	bool operator[](int i);

	void set(int i, bool val);
};

template<int T>
Bitmap<T>::Bitmap()
{
	clear();
}

template<int T>
Bitmap<T>::~Bitmap()
{
}

template<int T>
int Bitmap<T>::getSize()
{
	return T;
}

template<int T>
int Bitmap<T>::getNumBytes()
{
	return std::ceil(T/8.0);
}

template<int T>
char* Bitmap<T>::getMap()
{
	return map;
}

template<int T>
void Bitmap<T>::clear()
{
	memset(map, 0, getNumBytes());
}

template<int T>
bool Bitmap<T>::operator[](int i)
{
	if(i >= T || i < 0)
		return false;
	char cell = map[i/8];
	char masked = cell & 1 << i%8;
	return masked != 0;
}

template<int T>
void Bitmap<T>::set(int i, bool val)
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
