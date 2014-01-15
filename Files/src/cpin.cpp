#include "P2.h"

#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <cstring>
void cpin(int fd, std::string external_file, std::string v6_file) {
	//Parse v6_file to get parent path
	std::vector<std::string> path = split(v6_file, '/');
	std::stringstream ss;
	for(int i = 0; i < path.size() - 1; i++) {
		ss << "/" << path[i];
	}
	unsigned int par_inum = path_to_inum(fd, ss.str());
	int xfd = open(external_file.c_str(), O_RDONLY);

	SuperBlock sb(fd);
	dout << sb.nfree << std::endl;
	unsigned int child_inum = sb.allocate_inode(fd);
	INode node(child_inum);
	
	char buffer[BLOCK_SIZE] = {};
	int bytesRead;
	int size = 0;
	while((bytesRead = read(xfd, buffer, BLOCK_SIZE))) {
		size += bytesRead;
		int block = node.addBlock(fd, sb);

		lseek(fd, block * BLOCK_SIZE, SEEK_SET);
		write(fd, buffer, bytesRead);
	}
	node.flags |= 0x8000; //mark inode as allocated
	node.size = size;
	node.actime = node.modtime = time(NULL);
	
	//Add child inode to parent directory entry
	char dir_entry[DIR_SIZE] = {};
	write_long(child_inum, sizeof(child_inum), dir_entry);
	//memset(dir_entry, 0, DIR_SIZE());
	memcpy(dir_entry + sizeof(child_inum), path[path.size() - 1].c_str(), path[path.size() - 1].size());
	addDirEntry(fd, par_inum, dir_entry);

	node.flush(fd);
	sb.flush(fd);
}

void addDirEntry(int fd, unsigned int inum, char* dir_entry) {
	//Read the inode until an empty directory space is found(inode = 0)
	//If no such space is found then allocate a new block
	//Write the entry to the block, flush
	
	char buffer[BLOCK_SIZE] = {};
	int block = 0;
	int bytesRead;
	bool found = false;
	ByteAndBlock* rtnVal;
	rtnVal = copyBlock(fd, inum, block);
	lseek(fd, rtnVal->block * BLOCK_SIZE, SEEK_SET);
	read(fd, buffer, BLOCK_SIZE);
	while(!found && rtnVal->bytesRead) {
		//SuperBlock sb0(fd);
		for(int offset = 0; offset < rtnVal->bytesRead && !found; offset += DIR_SIZE) {
			unsigned int entry_inum = read_long(buffer + offset, sizeof(unsigned int));
		//SuperBlock sb1(fd);
			if(entry_inum == 0) {
				memcpy(buffer + offset, dir_entry, DIR_SIZE);

				lseek(fd, rtnVal->block * BLOCK_SIZE, SEEK_SET);
				write(fd, buffer, BLOCK_SIZE);
				found = true;
		//SuperBlock sb2(fd);
		//dout << "Hello world";
			}
		}
		delete rtnVal;
		rtnVal = copyBlock(fd, inum, block);
		lseek(fd, rtnVal->block * BLOCK_SIZE, SEEK_SET);
		read(fd, buffer, BLOCK_SIZE);
	}

	delete rtnVal;

	if(!found) {
		SuperBlock sb(fd);
		INode inode(fd, inum);
		int block = inode.addBlock(fd, sb);
		memset(buffer, 0, BLOCK_SIZE);
		memcpy(buffer, dir_entry, DIR_SIZE);
		
		lseek(fd, block * BLOCK_SIZE, SEEK_SET);
		write(fd, buffer, BLOCK_SIZE);

		inode.size += DIR_SIZE;

		inode.flush(fd);
		sb.flush(fd);
		dout << "Hello world";
	}
}


