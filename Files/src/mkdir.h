#ifndef MKDIR
#define MKDIR

#include "P2.h"

#include <string>
#include <vector>

void mkdir(int fd, std::string name);
unsigned int path_to_inum(int fd, std::string path);
unsigned int path_to_inum(int fd, std::string path, int inum);
unsigned int path_to_inum(int fd, std::vector<std::string>& path, int inode, int pathPos);

std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
#endif
