#pragma once
#include <string>

namespace Base64 {
    constexpr char digits[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/',
    };

	inline std::string encode(const std::string& data)
	{
		const size_t inputSize = data.length();
		const size_t triplets = inputSize / 3;
		const size_t remainingCharacters = inputSize % 3;

		std::string output;
		output.reserve(triplets * 4 + 4);

		const char* ptr = data.data();
		const char* lastTriplet = ptr + 3 * triplets;
		while (ptr <= lastTriplet)
		{
			const char& a = *ptr++, & b = *ptr++, & c = *ptr++;
			const char& A = a >> 2, B = (a & 0b11) | (b >> 4), C = (b & 0b1111) | (c >> 6), D = c & 0b111111;

			output += digits[A];
			output += digits[B];
			output += digits[C];
			output += digits[D];
		}

		switch (remainingCharacters)
		{
		default:
		case 0:
			break;
		case 1:
		{
			const char& a = *ptr++;
			const char& A = a >> 2, B = a & 0b11;
			output += digits[A];
			output += digits[B];
			output += "==";
			break;
		}
		case 2:
		{
			const char& a = *ptr++, & b = *ptr++;
			const char& A = a >> 2, B = (a & 0b11) | (b >> 4), C = (b & 0b1111);
			output += digits[A];
			output += digits[B];
			output += digits[C];
			output += '=';
		}
		}

		return output;
	}
}