#include "P2.h"

#include <fcntl.h>
#include <iostream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cstring>

int initfs(std::string name, int n1, int n2) {
	//Create the file.
	dout << "Creating file " << name << " " << n1 << " " << n2 << std::endl;
	int fd;
	const char* nameAsCharArray = name.c_str();
	fd = open(nameAsCharArray, O_CREAT | O_RDWR | O_TRUNC, 0777);
	
	{
		char buffer[BLOCK_SIZE];
		memset(buffer, 0, BLOCK_SIZE);
		for(int i = 0; i < n1; i++) {
			int rtnVal = write(fd, buffer, BLOCK_SIZE);

			if(rtnVal == -1)
				perror(NULL);
		}
	}
	//Assign general variables.
	unsigned int inodeBlocks = (n2 + BLOCK_SIZE / INODE_SIZE - 1) / (BLOCK_SIZE / INODE_SIZE); //Number of blocks required for inodes.
	unsigned int freeBlockStart = inodeBlocks + 2; //+2 because of the 0th unused block, super block
	
	//Assign SuperBlock variables and flush to file.
	SuperBlock super;
	super.isize = n2;
	super.fsize = n1;
	
	if (super.fsize - freeBlockStart - 1 > SB_LIST_SIZE)
		super.nfree = SB_LIST_SIZE;
	else 
		super.nfree = super.fsize - freeBlockStart - 1;

	unsigned int freeBlockNumber = freeBlockStart;
	for (int freeCount = super.nfree - 1; freeCount >= 0; freeCount--) {
		super.free[freeCount] = freeBlockNumber;
		freeBlockNumber++;
	}

	if (super.isize - 1 > SB_LIST_SIZE)
		super.ninode = SB_LIST_SIZE;
	else
		super.ninode = super.isize - 1;
	
	unsigned int iNumber = 2;
	dout << super.ninode;
	for (unsigned int iNumberCount = 0; iNumberCount < super.ninode; iNumberCount++) {
		super.inode[iNumberCount] = iNumber;
		iNumber++;
	}

	super.flock = '0';
	super.ilock = '0';
	super.fmod = '0';
	super.time = time(NULL);
	
	super.flush(fd);
	//SuperBlock sb(fd);
	
	//Create the home directory's inode and flush it to file.
	INode home(fd, 1);
	home.flags = 0x8000 | 0x4000; //I know what to do here. allocated and directory
	home.nlinks = 1;
	//These commands only supported on Linux.
	//home.uid = getuid();
	//home.gid = getgid();
	home.uid = 0;
	home.gid = 0;
	//home.size = 2 * DIR_SIZE;
	home.actime = home.modtime = time(NULL);
	
	home.flush(fd);

	//Write the directory entries for the home directory "." and ".."
	//JUST WRITE DIRECTLY TO THE DIRENTRYINFO
	char dirEntryInfo[DIR_SIZE];
	std::memset(dirEntryInfo, '\0', DIR_SIZE);
	write_long(1, sizeof(unsigned int), dirEntryInfo);
	dirEntryInfo[4] = '.';
	
	addDirEntry(fd, 1, dirEntryInfo);

	//SuperBlock sb(fd);
	
	dirEntryInfo[5] = '.';
	addDirEntry(fd, 1, dirEntryInfo);
	
	//SuperBlock sb2(fd);
	{
		INode nd1(fd, 1);
		char buffer[BLOCK_SIZE];
		ByteAndBlock* rtn = copyBlock(fd, 1, 0);

		lseek(fd, rtn->block * BLOCK_SIZE, SEEK_SET);
		read(fd, buffer, BLOCK_SIZE);

	dout << "Hello World!";
	}
	//Skip to start of freeblocks, create freeBlock chains, and flush to file.
	for (freeBlockNumber = freeBlockStart + super.nfree; freeBlockNumber < super.fsize; freeBlockNumber++) {
		int rtnVal = lseek(fd, freeBlockNumber * BLOCK_SIZE, SEEK_SET); //Skip to the head of the chain
		
		if(rtnVal == -1)
			perror(NULL);
		char buffer[BLOCK_SIZE] = {};

		int freeLeft;
		if (super.fsize - freeBlockNumber > SB_LIST_SIZE)
			freeLeft = SB_LIST_SIZE;
		else
			freeLeft = super.fsize - freeBlockNumber;
		
		unsigned int headList[freeLeft];

		for (freeLeft--; freeLeft >= 0; freeLeft--) {
			freeBlockNumber++;
			headList[freeLeft] = freeBlockNumber;
			//write(fd, &freeBlock, sizeof(freeBlock)); 
		}

		int offset = 0;
		write_long(freeLeft, sizeof(freeLeft), buffer + offset);
		offset += sizeof(freeLeft);

		for (unsigned int index = 0; index < (sizeof(headList)/sizeof(*headList)); index++) {
			write_long(headList[index], sizeof(headList[index]), buffer + offset);
			offset += sizeof(headList[index]);
		}

		rtnVal = write(fd, buffer, BLOCK_SIZE);
	
		if(rtnVal == -1)
			perror(NULL);
	}		
	//SuperBlock sb(fd);
	{
		INode nd1(fd, 1);
		char buffer[BLOCK_SIZE];
		ByteAndBlock* rtn = copyBlock(fd, 1, 0);

		lseek(fd, rtn->block * BLOCK_SIZE, SEEK_SET);
		read(fd, buffer, BLOCK_SIZE);

	}
	dout << "Hello World!";

	return fd;
}
