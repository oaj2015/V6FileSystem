#ifndef P2
#define P2

#define SB_LIST_SIZE 253
#define I_LIST_SIZE 27

#define BLOCK_SIZE 2048
#define INODE_SIZE 128
#define DIR_SIZE 64

#ifdef DEBUG
#define dout std::cout << std::endl << __FILE__ << "(" << __LINE__ << ") DEBUG: "
#else
#define dout 0 && std::cout
#endif

#include "initfs.h"
#include "cpin.h"
#include "cpout.h"
#include "mkdir.h"
#include "superblock.h"
#include "inode.h"

unsigned long long read_long(char* array, int size);
void write_long(unsigned long long num, int size, char* array);

bool stringEquals(std::string a, std::string b);


#endif
