#ifndef CPIN
#define CPIN

#include "P2.h"

#include <string>

void cpin(int fd, std::string external_name, std::string v6_name);
void cpin(int xfd, int ifd, int inum);

void addBlockToInode(int fd, int inum, int block);
void addDirEntry(int fd, unsigned int inum, char* dir_entry);

#endif
