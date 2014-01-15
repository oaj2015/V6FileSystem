#include "P2.h"

#include <string>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>

int bytesRead(unsigned int blockNumber, unsigned int size) {
	if (blockNumber < size / BLOCK_SIZE)
		return BLOCK_SIZE;
	else if (blockNumber == size / BLOCK_SIZE)
		//return BLOCK_SIZE - (blockNumber * BLOCK_SIZE - size);
		return size % BLOCK_SIZE;
	else 
		return 0;
}

void cpout(int fd, std::string v6_file, std::string external_file) {
	int outFD = 0;
	const char* externalNameAsCharArray = external_file.c_str();
	outFD = open(externalNameAsCharArray, O_CREAT | O_RDWR | O_TRUNC, 0777);

	unsigned int iNumber = path_to_inum(fd, v6_file);
	INode node(fd, iNumber);
	
	int size = node.size;
	char buffer[BLOCK_SIZE];
	unsigned int lgclBlkNmbr = 0;

	while (size > 0) {
		ByteAndBlock* rtn = copyBlock(fd, iNumber, lgclBlkNmbr++);

		dout << size;
		lseek(fd, rtn->block*BLOCK_SIZE, SEEK_SET);
		read(fd, buffer, rtn->bytesRead);

		size -= rtn -> bytesRead;
		write(outFD, buffer, rtn->bytesRead);
	}
}

ByteAndBlock* copyBlock(int fd, int inum, unsigned int logicalBlockNumber) {
	INode node(fd, inum);
	ByteAndBlock* rtnVal = new ByteAndBlock();
	unsigned int physicalBlockNumber = 0;
	char buffer[BLOCK_SIZE];

	bool isSmall = true;
	if (node.flags & 0x1000) {
		isSmall = false;
	}

	if (logicalBlockNumber > node.size / BLOCK_SIZE) {
		rtnVal -> bytesRead = 0;
		rtnVal -> block = 0;
	}

	if (isSmall) { //The file is small.
		physicalBlockNumber = node.addr[logicalBlockNumber];
		if (physicalBlockNumber == 0) {
			rtnVal -> bytesRead = bytesRead(logicalBlockNumber, node.size);
			rtnVal -> block = physicalBlockNumber;
			return rtnVal;
		}
	}
	else { //The file is large or huge.
		unsigned int indirectBlockLogical = logicalBlockNumber / (BLOCK_SIZE / sizeof(unsigned int));
		unsigned int offsetByte = logicalBlockNumber % (BLOCK_SIZE / sizeof(unsigned int));	
		
		if (indirectBlockLogical < I_LIST_SIZE - 1) { //The block is in a single indirect section.
			unsigned int indirectBlockNumber = node.addr[indirectBlockLogical];
			lseek(fd, indirectBlockNumber * BLOCK_SIZE, SEEK_SET);
			read(fd, buffer, sizeof(BLOCK_SIZE));

			physicalBlockNumber = read_long(buffer + offsetByte * sizeof(unsigned int), sizeof(unsigned int));
			if (physicalBlockNumber == 0) {
				memset(buffer, 0, BLOCK_SIZE);
				rtnVal -> bytesRead = bytesRead(logicalBlockNumber, node.size);
				rtnVal -> block = physicalBlockNumber;
				return rtnVal;
			}
		}	
		else { //The file is huge and the block is in a double indirect section.
			unsigned int indexWithinPhysicalBlocks = logicalBlockNumber - (I_LIST_SIZE - 1) * (BLOCK_SIZE / sizeof(unsigned int));
			unsigned int indexWithinIndirectLevelOne = indexWithinPhysicalBlocks / (BLOCK_SIZE / sizeof(unsigned int));
			unsigned int wordWithinLevelOneIndirectBlock = indexWithinPhysicalBlocks % (BLOCK_SIZE / sizeof(unsigned int));
			unsigned int wordWithinLevelTwoIndirectBlock = indexWithinIndirectLevelOne /  (BLOCK_SIZE / sizeof(unsigned int));

			unsigned int levelTwoPhysicalNumber = node.addr[I_LIST_SIZE - 1];
			if (physicalBlockNumber == 0) {
				memset(buffer, 0, BLOCK_SIZE);
				rtnVal -> bytesRead = bytesRead(logicalBlockNumber, node.size);
				rtnVal -> block = physicalBlockNumber;
				return rtnVal;
			}
			lseek(fd, levelTwoPhysicalNumber * BLOCK_SIZE, SEEK_SET);
			read(fd, buffer, sizeof(BLOCK_SIZE));

			unsigned int LevelOnePhysicalNumber = read_long(buffer + wordWithinLevelTwoIndirectBlock * sizeof(unsigned int), sizeof(unsigned int));
			if (physicalBlockNumber == 0) {
				memset(buffer, 0, BLOCK_SIZE);
				rtnVal -> bytesRead = bytesRead(logicalBlockNumber, node.size);
				rtnVal -> block = physicalBlockNumber;
				return rtnVal;
			}
			lseek(fd, LevelOnePhysicalNumber * BLOCK_SIZE, SEEK_SET);
			read(fd, buffer, sizeof(BLOCK_SIZE));

			physicalBlockNumber = read_long(buffer + wordWithinLevelOneIndirectBlock * sizeof(unsigned int), sizeof(unsigned int));
			if (physicalBlockNumber == 0) {
				memset(buffer, 0, BLOCK_SIZE);
				rtnVal -> bytesRead = bytesRead(logicalBlockNumber, node.size);
				rtnVal -> block = physicalBlockNumber;
				return rtnVal;
			}
		}
	}
	rtnVal -> bytesRead = bytesRead(logicalBlockNumber, node.size);
	rtnVal -> block = physicalBlockNumber;
	return rtnVal;
}
