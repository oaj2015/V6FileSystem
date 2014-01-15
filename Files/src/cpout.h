#ifndef CPOUT
#define CPOUT

#include "P2.h"

#include <string>

void cpout(int fd, std::string v6_name, std::string external_name);
struct ByteAndBlock* copyBlock(int fd, int inum, unsigned int block);

struct ByteAndBlock {
	int bytesRead;
	int block;
};
#endif
