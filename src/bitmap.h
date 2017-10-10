#pragma once

class Bitmap {
	int size;
	char* map;
public:
	Bitmap(int size);
	~Bitmap();

	int getSize();
	char* getMap();

	// returns value at i:th bit
	// returns false if outside range
	bool operator[](int i);

	void set(int i, bool val);
};
