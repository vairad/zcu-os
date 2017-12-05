#pragma once
#include <cstdint>

#undef stdin
#undef stderr
#undef stdout


typedef size_t bitmap_t;

class Bitmap
{	
	const static uint16_t partSize = sizeof(bitmap_t) * 8;

	uint32_t mapPartCount;
	bitmap_t *map;


public:
	Bitmap(size_t mapSize);
	~Bitmap();

	bool isSet(bitmap_t b);
	void set(bitmap_t b, bool val = true);
	void clear(bitmap_t b);
};

