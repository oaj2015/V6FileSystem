#include "P2.h"

#include <iostream>
#include <string>

unsigned long long read_long(char* array, int n) {
	unsigned long long x = 0;
	for(int i = 0; i < n; i++) {
		x = (x << 8) | (unsigned char)  array[i];
		//dout << std::hex << x << std::endl;
	}
	return x;
}

void write_long(unsigned long long x, int n, char* array) {
	for(int i = n - 1; i >= 0; i--) {
		array[i] = x & 0xFF;
		x = x >> 8;
		//dout << std::hex << array[i] << std::endl;
	}
}

bool stringEquals(std::string a, std::string b) {
	int min = a.length() < b.length()? a.length() : b.length();
	int max = a.length() > b.length()? a.length() : b.length();
	const char* ca = a.c_str();
	const char* cb = b.c_str();
	for(int i = 0; i < min; i++) {
		if(ca[i] != cb[i])
			return false;
	}

	if(a.length() == max) {
		for(int i = min; i < max; i++) {
			if(ca[i] != 0)
				return false;
		}
	}
	else {
		for(int i = min; i < max; i++) {
			if(cb[i] != 0)
				return false;
		}
	}
	return true;
}
