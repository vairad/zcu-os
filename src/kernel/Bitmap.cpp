#include "Bitmap.h"

Bitmap::Bitmap(size_t mapSize)
{
	this->mapPartCount = static_cast<uint16_t>(mapSize / this->partSize + 1);
	this->map = new bitmap_t[this->mapPartCount]{0};
}


Bitmap::~Bitmap()
{
	delete[] this->map;
	this->map = nullptr;
}

void calcBitmapBit(bitmap_t b, uint8_t partSize, uint16_t *part, uint64_t *bit) {
	*part = static_cast<uint16_t>(b / partSize);
	uint8_t partOffset = b % partSize;
	*bit = static_cast<uint64_t>(1LL << partOffset);
}

bool Bitmap::isSet(bitmap_t b) {
	uint16_t part; uint64_t bit;
	calcBitmapBit(b, this->partSize, &part, &bit);

	return (this->map[part] & bit) != 0;
}
void Bitmap::set(bitmap_t b, bool val) {
	uint16_t part; uint64_t bit;
	calcBitmapBit(b, this->partSize, &part, &bit);

	if (val) {
		this->map[part] |= bit;
	}
	else {
		this->map[part] &= ~bit;
	}
}

void Bitmap::clear(bitmap_t b) {
	this->set(b, false);
}
