#ifndef INODE
#define INODE
class INode {
public:
	//Read in this INode from the given file
	//Assumed to be at the correct position
	INode(int fd, int inum);

	//Make an empty inode
	//Initializes actime and modtime
	INode(int inum);
	void flush(int fd);
	int addBlock(int fd, SuperBlock& sb);

	unsigned short flags;
	char nlinks;
	short uid;
	short gid;
	unsigned int size;
	unsigned int addr[I_LIST_SIZE];
	unsigned int actime;
	unsigned int modtime;
	
	//stored in memory but not saved to disk
	int inum;
};

int getLastUsedPos(char* buffer);

#endif
