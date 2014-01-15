#include "P2.h"

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
SuperBlock::SuperBlock() {
	memset(free, 0, sizeof(free));
	memset(inode, 0, sizeof(inode));
}

SuperBlock::SuperBlock(int fd) {
	memset(free, 0, sizeof(free));
	memset(inode, 0, sizeof(inode));
	//Seek to Block 1
	int rtnVal = lseek(fd, BLOCK_SIZE, SEEK_SET);
	if(rtnVal == -1)
		perror(NULL);
	if(rtnVal != BLOCK_SIZE)
		dout << rtnVal;
	char buffer[BLOCK_SIZE] = {};

	rtnVal = read(fd, buffer, BLOCK_SIZE);
	if(rtnVal == -1)
		perror(NULL);
	unsigned int offset = 0;
	isize  = read_long(buffer + offset, sizeof(isize )); offset += sizeof(isize );
	fsize  = read_long(buffer + offset, sizeof(fsize )); offset += sizeof(fsize );
	nfree  = read_long(buffer + offset, sizeof(nfree )); offset += sizeof(nfree );
	offset += 2;
	for(int i = 0; i < SB_LIST_SIZE; i++) {
		free[i] = read_long(buffer + offset, sizeof(free[i]));
		offset += sizeof(free[i]);
	}
	ninode = read_long(buffer + offset, sizeof(ninode)); offset += sizeof(ninode);
	offset += 2;
	for(int i = 0; i < SB_LIST_SIZE; i++) {
		inode[i] = read_long(buffer + offset, sizeof(inode[i]));
		offset += sizeof(inode[i]);
	}
	flock  = read_long(buffer + offset, sizeof(flock )); offset += sizeof(flock );
	ilock  = read_long(buffer + offset, sizeof(ilock )); offset += sizeof(ilock );
	fmod   = read_long(buffer + offset, sizeof(fmod  )); offset += sizeof(fmod  );
	time   = read_long(buffer + offset, sizeof(time  )); offset += sizeof(time  );
} 

void SuperBlock::flush(int fd) {
	int rtnVal = lseek(fd, BLOCK_SIZE, SEEK_SET);
	if(rtnVal == -1)
		perror(NULL);
	char buffer[BLOCK_SIZE] = {};
	
	unsigned int offset = 0;
	write_long(isize , sizeof(isize ), buffer + offset); offset += sizeof(isize );
	write_long(fsize , sizeof(fsize ), buffer + offset); offset += sizeof(fsize );
	write_long(nfree , sizeof(nfree ), buffer + offset); offset += sizeof(nfree );
	offset += 2;
	for(int i = 0; i < SB_LIST_SIZE; i++) {
		write_long(free[i], sizeof(free[i]), buffer + offset);
		//long long x = read_long(buffer + offset, sizeof(free[i]));
		//dout << free[i] << " " << x << std::endl;
		offset += sizeof(free[i]);
	}
	write_long(ninode, sizeof(ninode), buffer + offset); offset += sizeof(ninode);
	offset += 2;
	for(int i = 0; i < SB_LIST_SIZE; i++) {
		write_long(inode[i], sizeof(inode[i]), buffer + offset);
		offset += sizeof(inode[i]);
	}
	write_long(flock , sizeof(flock ), buffer + offset); offset += sizeof(flock );
	write_long(ilock , sizeof(ilock ), buffer + offset); offset += sizeof(ilock );
	write_long(fmod  , sizeof(fmod  ), buffer + offset); offset += sizeof(fmod  );
	write_long(time  , sizeof(time  ), buffer + offset); offset += sizeof(time );

	rtnVal = write(fd, buffer, BLOCK_SIZE);
	if(rtnVal == -1)
		perror(NULL);
}

unsigned int SuperBlock::allocate_block(int fd) {
	nfree--;

	//If there are no blocks left return an error
	
	dout << nfree << std::endl;
	if(free[nfree] == 0)
		return -1;
	
	unsigned int block = free[nfree];

	if(nfree == 0) {
		char buffer[BLOCK_SIZE] = {};
		lseek(fd, block * BLOCK_SIZE, SEEK_SET);
		
		read(fd, buffer, BLOCK_SIZE);
		unsigned int offset = 0;
		nfree = read_long(buffer, sizeof(nfree));
		offset += 2;
		for(int i = 0; i < SB_LIST_SIZE; i++) {
			free[i] = read_long(buffer + offset, sizeof(free[i]));
			offset += sizeof(free[i]);
		}
	}
	return block;
}

void SuperBlock::free_block(int fd, int block) {
	if(nfree == SB_LIST_SIZE) {
		char buffer[BLOCK_SIZE] = {};
		lseek(fd, block * BLOCK_SIZE, SEEK_SET);
		
		unsigned int offset = 0;
		write_long(nfree, sizeof(nfree), buffer + offset);
		offset += 2;
		for(int i = 0; i < SB_LIST_SIZE; i++) {
			write_long(free[i], sizeof(free[i]), buffer + offset);
			offset += sizeof(free[i]);
		}
		write(fd, buffer, BLOCK_SIZE);
	}
	free[nfree] = block;
	nfree++;
}

unsigned int SuperBlock::allocate_inode(int fd) {
	if(ninode > 0)
		return inode[--ninode];
	
	//No more inodes are available in the inode list
	//So we need to read through the ilist to find free inodes
	unsigned int count = 0;
	unsigned int iblock = 0;
	
	while(count < SB_LIST_SIZE && iblock < isize) {
		char buffer[BLOCK_SIZE] = {};
		lseek(fd, BLOCK_SIZE * (iblock + 2), SEEK_SET);
		read(fd, buffer, BLOCK_SIZE);

		//Test if each inode is free.
		//If it is, increment count, compute the inumber and add it
		//to the inode list
		for(int i = 0; i < BLOCK_SIZE / INODE_SIZE; i++) {
			unsigned short flags = read_long(buffer + i * INODE_SIZE, sizeof(unsigned short));
			if(flags & 0x8000) {
				inode[count++] = iblock * BLOCK_SIZE / INODE_SIZE + i + 1;
			}
		}
		iblock++;
	}

	if(count > 0) {
		ninode = count;
		return inode[--ninode];
	} else {
		return -1;
	}
}

void SuperBlock::free_inode(int num) {
	if(ninode < SB_LIST_SIZE)
		inode[ninode] = num;
} 

