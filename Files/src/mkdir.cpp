#include "P2.h"

#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <cstring>
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while(std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

void mkdir(int fd, std::string name) {
	std::vector<std::string> dirs = split(name, '/');

	int inum = 1; //1 represents the root

	SuperBlock sb(fd);
	//Get the i-number for each directory along the path,
	//Until the first directory along the path isn't found
	//Create that directory
	for(std::vector<std::string>::iterator it = dirs.begin(); it != dirs.end(); ++it) {
		dout << *it << std::endl;
		int child_inum = path_to_inum(fd, *it, inum);

		//if the directory is not present
		if(child_inum < 1) {
			child_inum = sb.allocate_inode(fd);
			if(child_inum < 1) {
				std::cerr << "I-nodes depleted. Cannot allocate directory entry." << std::endl;
				return;
			}

			INode node(child_inum);
			node.flags = 0x8000 | 0x4000; //allocated and directory

			//We've allocated an inode, now we need to allocate a data block
			//Populate that data block with the entries for . and ..
			//and associate it with the inode.
			int block = sb.allocate_block(fd);
			if(block < 0) {
				std::cerr << "Data blocks depleted. Cannot allocate directory entry." << std::endl;
				return;
			}

			char buffer[BLOCK_SIZE] = {};
			write_long(child_inum, sizeof(child_inum), buffer);
			buffer[sizeof(child_inum)] = '.';
			write_long(inum, sizeof(inum), buffer + DIR_SIZE);
			buffer[sizeof(inum) + DIR_SIZE] = '.';
			buffer[sizeof(inum) + DIR_SIZE + 1] = '.';

			lseek(fd, block, SEEK_SET);
			write(fd, buffer, BLOCK_SIZE);

			char dir_entry[DIR_SIZE];
			write_long(child_inum, sizeof(child_inum), dir_entry);
			strncpy(dir_entry + sizeof(child_inum), (*it).c_str(), DIR_SIZE - sizeof(child_inum));

			//Add child inode to parent's directory listing
			addDirEntry(fd, inum, dir_entry);					

			node.flush(fd);
		}

		inum = child_inum;
	}

	sb.flush(fd);
}

unsigned int path_to_inum(int fd, std::string path) {
	return path_to_inum(fd, path, 1);
}

unsigned int path_to_inum(int fd, std::string path, int inum) {
	std::vector<std::string> pathVector = split(path, '/');
	return path_to_inum(fd, pathVector, inum, 0);
}

unsigned int path_to_inum(int fd, std::vector<std::string>& path, int inum, int pathPos) {
	//dout << path << " " << inum << std::endl;
	//Get the first directory name in the path
	char delim = '/';
	std::string dir;
	if(pathPos < path.size()) {
		//Read the inode which represents a directory
		//and look for a file whose name matches dir
		dir = path[pathPos];
		char buffer[BLOCK_SIZE] = {};
		unsigned int block = 0;
		ByteAndBlock* rtnVal;
		rtnVal = copyBlock(fd, inum, block); 
		lseek(fd, rtnVal->block * BLOCK_SIZE, SEEK_SET);
		read(fd, buffer, BLOCK_SIZE);

		while(rtnVal->bytesRead > 0) {
			for(int offset = 0; offset < rtnVal->bytesRead; offset += DIR_SIZE) {
				std::string name(buffer + offset + sizeof(unsigned int), DIR_SIZE - sizeof(unsigned int));
				if(stringEquals(dir, name)) {
					delete rtnVal;
					return path_to_inum(fd, path, read_long(buffer + offset, sizeof(int)), pathPos + 1);
				}

			}
			delete rtnVal;

			rtnVal = copyBlock(fd, inum, block++); 
			lseek(fd, rtnVal->block * BLOCK_SIZE, SEEK_SET);
			read(fd, buffer, BLOCK_SIZE);
		}
		delete rtnVal;
		return -1;
	} else {
		return inum;
	}

}
