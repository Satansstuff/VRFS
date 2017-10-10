#pragma once

class Bitmap {
	int size;
	int num_bytes;
	char* map;
public:
	Bitmap(int size);
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
