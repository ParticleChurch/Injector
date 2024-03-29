#pragma once
#include <string>

namespace Utils
{
	inline bool stringStartsWith(std::wstring str, std::wstring prefix)
	{
		return str.substr(0, prefix.length()) == prefix;
	}

	inline void mapXOR(unsigned char* data, size_t dataSize)
	{
		constexpr unsigned char bruh[256] = {
			0xA3, 0x33, 0xD9, 0x8C, 0x74, 0x3D, 0xF6, 0x66, 0x49, 0x07, 0xB8, 0x29, 0xDC, 0xA9, 0x90, 0xB6,
			0x2E, 0xFE, 0x9C, 0xE4, 0x9F, 0x55, 0x71, 0x86, 0x66, 0xD7, 0xD9, 0x42, 0x7A, 0x12, 0xAB, 0xD9,
			0x72, 0xDA, 0xB8, 0xEF, 0x59, 0xA1, 0xC9, 0x6F, 0x86, 0x45, 0x12, 0xE7, 0xB7, 0x6D, 0x19, 0x57,
			0xBC, 0xC5, 0xA9, 0x4C, 0x1C, 0xC4, 0x7B, 0xB7, 0x3E, 0x71, 0x08, 0x25, 0x32, 0xD8, 0xA9, 0x9E,
			0xA8, 0x4A, 0x03, 0x49, 0x7E, 0x3E, 0x6D, 0xA7, 0x21, 0x58, 0xA3, 0x34, 0x26, 0xB3, 0x6A, 0x75,
			0xA9, 0xF6, 0xC2, 0xB0, 0x38, 0xFD, 0xBB, 0xD0, 0xC8, 0x8D, 0x49, 0x6C, 0x03, 0x64, 0x69, 0xD1,
			0x44, 0xDB, 0xF7, 0x34, 0xB5, 0xB1, 0x1A, 0x42, 0x94, 0xA8, 0x24, 0xC9, 0x45, 0xCA, 0xFF, 0x1B,
			0xF0, 0x4A, 0xB2, 0xD5, 0xD0, 0x78, 0xF7, 0xF8, 0x77, 0x7B, 0x46, 0x59, 0x8E, 0x9A, 0x74, 0x20,
			0x6E, 0xE5, 0x12, 0xD3, 0x75, 0x27, 0x09, 0x45, 0x12, 0xB8, 0xA0, 0x98, 0xB6, 0xA5, 0x00, 0xC3,
			0xE6, 0x9F, 0x4E, 0xDD, 0xD0, 0x0B, 0xFC, 0x02, 0x4C, 0x8D, 0x27, 0x63, 0x63, 0x3F, 0x6B, 0x57,
			0xAA, 0x50, 0x12, 0xD1, 0x9B, 0x7C, 0x3F, 0xA1, 0x00, 0xBB, 0xFF, 0xA1, 0xC8, 0x5E, 0xDC, 0x10,
			0xF3, 0x07, 0x98, 0x34, 0x66, 0x20, 0x39, 0xF8, 0xFC, 0x0D, 0xB3, 0x09, 0x5F, 0xAA, 0x77, 0x00,
			0xCF, 0x65, 0xA9, 0x9E, 0xAC, 0x44, 0x03, 0x13, 0x36, 0x4C, 0xA7, 0xF1, 0x0C, 0x1A, 0x69, 0x51,
			0x42, 0x9A, 0x6A, 0x74, 0xD8, 0xF7, 0x2E, 0x3C, 0xE3, 0xA9, 0x02, 0xF5, 0x03, 0x68, 0x69, 0x25,
			0xAC, 0xD1, 0x94, 0x1C, 0xFF, 0x43, 0x05, 0xB8, 0x2D, 0x56, 0x3D, 0x1A, 0xF5, 0x8B, 0x3A, 0x0D,
			0xEA, 0xA7, 0x04, 0x1C, 0x29, 0x03, 0x6F, 0x5E, 0x3A, 0x6D, 0xB6, 0xB5, 0x09, 0x31, 0x8F, 0xC3,
		};

		for (size_t offset = 0; offset < dataSize; offset++)
			data[offset] ^= bruh[offset % 256];
	}
}