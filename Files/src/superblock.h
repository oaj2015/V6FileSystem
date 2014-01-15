#ifndef SUPERBLOCK
#define SUPERBLOCK

class SuperBlock {
public:
	//Read in data elements from the given file,
	//seeks to the superblock's location if needed
	SuperBlock(int fd);
	
	SuperBlock();

	//Flush data elements to the file
	//seeks to the correct starting position
	void flush(int fd);
	
	//Allocates a data block and returns the block number
	//Uses fd in case free list needs to be repopulated
	unsigned int allocate_block(int fd);

	//Frees the given data block
	//Uses fd in case free list needs to be flushed first
	void free_block(int fd, int block);

	//Allocate an inode and return the i-number
	//Uses fd in case inode list needs to be read from ilist
	unsigned int allocate_inode(int fd);

	//Frees the inode
	void free_inode(int num);

	unsigned int isize;
	unsigned int fsize;
	unsigned short nfree;
	unsigned int free[SB_LIST_SIZE];
	unsigned short ninode;
	unsigned int inode[SB_LIST_SIZE];
	char flock;
	char ilock;
	char fmod;
	unsigned int time;
};

#endif
