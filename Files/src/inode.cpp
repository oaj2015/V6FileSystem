#include "P2.h"

#include <iostream>
#include <time.h>
#include <unistd.h>
#include <cstring>
INode::INode(int fd, int inumber) {
	memset(addr, 0, sizeof(addr));
	char buffer[INODE_SIZE];
	inum = inumber;
	if(inum > 0)
		lseek(fd, 2 * BLOCK_SIZE + (inum - 1) * INODE_SIZE, SEEK_SET);
	read(fd, buffer, INODE_SIZE);
	
	int offset = 0;
	flags   = read_long(buffer + offset, sizeof(flags  )); offset += sizeof(flags  );
	nlinks  = read_long(buffer + offset, sizeof(nlinks )); offset += sizeof(nlinks );
	offset += 1;
	uid     = read_long(buffer + offset, sizeof(uid    )); offset += sizeof(uid    );
	gid     = read_long(buffer + offset, sizeof(gid    )); offset += sizeof(gid    );
	size    = read_long(buffer + offset, sizeof(size   )); offset += sizeof(size   );
	for(int i = 0; i < I_LIST_SIZE; i++) {
		addr[i] = read_long(buffer + offset, sizeof(addr[i]));
		offset += sizeof(addr[i]);

	}
	actime  = read_long(buffer + offset, sizeof(actime )); offset += sizeof(actime );
	modtime = read_long(buffer + offset, sizeof(modtime)); offset += sizeof(modtime);
}

void INode::flush(int fd) {
	char buffer[INODE_SIZE];
	if(inum > 0)
		lseek(fd, 2 * BLOCK_SIZE + (inum - 1) * INODE_SIZE, SEEK_SET);
	int offset = 0;
	write_long(flags  , sizeof(flags  ), buffer + offset); offset += sizeof(flags  );
	write_long(nlinks , sizeof(nlinks ), buffer + offset); offset += sizeof(nlinks );
	offset += 1;
	write_long(uid    , sizeof(uid    ), buffer + offset); offset += sizeof(uid    );
	write_long(gid    , sizeof(gid    ), buffer + offset); offset += sizeof(gid    );
	write_long(size   , sizeof(size   ), buffer + offset); offset += sizeof(size   );
	for(int i = 0; i < I_LIST_SIZE; i++) {
		write_long(addr[i], sizeof(addr[i]), buffer + offset);
		offset += sizeof(addr[i]);
	}
	write_long(actime , sizeof(actime ), buffer + offset); offset += sizeof(actime );
	write_long(modtime, sizeof(modtime), buffer + offset); offset += sizeof(modtime);

	write(fd, buffer, INODE_SIZE);
}

INode::INode(int inumber) {
	memset(addr, 0, sizeof(addr));
	inum = inumber;
	actime = modtime = time(NULL);
}

int INode::addBlock(int fd, SuperBlock& sb) {
	unsigned int block = sb.allocate_block(fd);
	bool isLarge = flags & 0x1000;

	//Find the position of the last nonzero entry
	int pos = -1;
	for(int i = I_LIST_SIZE - 1; i >= 0 && pos < 0; i--) {
		if(addr[i] > 0) {
			pos = i;
		}
	}
	
	//Find the place where the next block goes
	if(isLarge && pos == I_LIST_SIZE - 1) {
		//Huge file

		lseek(fd, addr[pos] * BLOCK_SIZE, SEEK_SET);
		char buffer[BLOCK_SIZE] = {};
		read(fd, buffer, BLOCK_SIZE);

		//ipos is the index of the last nonzero second-level indirect block
		//ipos is guaranteed to be >= 0, since addr[I_LIST_SIZE - 1] > 0
		//so we can read the first level indirect block pointed to by ipos
		//and check if it has free space
		unsigned int ipos = getLastUsedPos(buffer);
		unsigned int iblock = read_long(buffer + ipos, sizeof(unsigned int));
		char ibuffer[BLOCK_SIZE] = {};

		lseek(fd, iblock * BLOCK_SIZE, SEEK_SET);
		read(fd, ibuffer, BLOCK_SIZE);

		//either iipos is at the end (so there is no room in
		//this first level indirect block (ibuffer)),
		//or iipos is not at the end and we can insert block into ibuffer
		unsigned int iipos = getLastUsedPos(ibuffer);

		if(iipos < BLOCK_SIZE / sizeof(unsigned int) - 1) {
			//There is room
			write_long(block, sizeof(block), ibuffer + iipos + sizeof(block));

			lseek(fd, iblock * BLOCK_SIZE, SEEK_SET);
			write(fd, ibuffer, BLOCK_SIZE);
		}
		else {
			//There is no room
			//Allocate new first level indirect block
			//Add references in 2nd indirect block and in new block

			//Check that there is room for a new 1st indirect block
			//If not then print an error message
			if(ipos == BLOCK_SIZE / sizeof(unsigned int) - 1) {
				std::cerr << "File has reached maximum size. Cannot increase size." << std::endl;
				return -1;
			}

			unsigned int intermediate_block = sb.allocate_block(fd);
			memset(ibuffer, 0, BLOCK_SIZE);
			write_long(block, sizeof(block), ibuffer);
			
			lseek(fd, intermediate_block * BLOCK_SIZE, SEEK_SET);
			write(fd, ibuffer, BLOCK_SIZE);

			write_long(intermediate_block, sizeof(intermediate_block), buffer + ipos + sizeof(unsigned int));

			lseek(fd, addr[pos] * BLOCK_SIZE, SEEK_SET);
			write(fd, buffer, BLOCK_SIZE);
			
		}

		
	}
	else if(isLarge) {
		//Large file

		//Read in the block pointed to by pos
		//Either it has room for one more entry or it does not
		lseek(fd, addr[pos] * BLOCK_SIZE, SEEK_SET);
		char buffer[BLOCK_SIZE] = {};
		read(fd, buffer, BLOCK_SIZE);

		int ipos = getLastUsedPos(buffer);

		if(ipos < BLOCK_SIZE / sizeof(block) - 1) {
			//There is room in the block
			write_long(block, sizeof(block), buffer + ipos + sizeof(block));
			lseek(fd, addr[pos] * BLOCK_SIZE, SEEK_SET);
			write(fd, buffer, BLOCK_SIZE);
		}
		else {
			//There is no room in the block
			if(pos < I_LIST_SIZE - 2) {
				//Allocate new intermediate block, first entry will be block
				unsigned int intermediate_block = sb.allocate_block(fd);
				char buffer[BLOCK_SIZE] = {};
				write_long(block, sizeof(block), buffer);
				lseek(fd, intermediate_block * BLOCK_SIZE, SEEK_SET);
				write(fd, buffer, BLOCK_SIZE);

				//Assign intermediate block into inode
				addr[pos + 1] = intermediate_block;
			}
			else {
				//INode becomes huge
				
				//Allocate 2 intermediate blocks
				unsigned int intermediate_block1 = sb.allocate_block(fd);
				unsigned int intermediate_block2 = sb.allocate_block(fd);

				char buffer[BLOCK_SIZE] = {};

				//Assign proper references
				//1st level indirect block points to block
				write_long(block, sizeof(block), buffer);

				lseek(fd, intermediate_block1 * BLOCK_SIZE, SEEK_SET);
				write(fd, buffer, BLOCK_SIZE);

				//2nd level indirect block points to 1st level indirect
				write_long(intermediate_block1, sizeof(intermediate_block1), buffer);

				lseek(fd, intermediate_block2 * BLOCK_SIZE, SEEK_SET);
				write(fd, buffer, BLOCK_SIZE);
				
				//Assign 2nd intermediate block into inode
				addr[pos] = intermediate_block2;
			}
		}
	}
	else {
		//Small file

		if(pos < I_LIST_SIZE - 2)
			addr[pos+1] = block;
		else {
			//The small file is at capacity
			//Change the file to large, allocate a new intermediate block
			flags = flags | 0x1000;
			unsigned int intermediate_block = sb.allocate_block(fd);
			char buffer[BLOCK_SIZE] = {};

			unsigned int offset = 0;
			for(int i = 0; i < I_LIST_SIZE - 1; i++) {
				write_long(addr[i], sizeof(addr[i]), buffer + offset);
				offset += sizeof(addr[i]);
			}
			write_long(block, sizeof(block), buffer + offset);

			lseek(fd, intermediate_block * BLOCK_SIZE, SEEK_SET);
			write(fd, buffer, BLOCK_SIZE);

			//Assign intermediate block into inode, overwrite other fields
			for(int i = 1; i < I_LIST_SIZE; i++)
				addr[i] = 0;
			addr[0] = intermediate_block;
		}
	}

	size += BLOCK_SIZE;
	
	return block;
}


int getLastUsedPos(char * buffer) {
	int ipos = -1;
	for(unsigned int offset = BLOCK_SIZE / sizeof(unsigned int) - 1; offset >= 0 && ipos < 0; offset -= sizeof(unsigned int)) {
		if(read_long(buffer + offset, sizeof(unsigned int)) > 0)
			ipos = offset;
	}
	return ipos;
}
