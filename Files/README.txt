The program must be compiled and run in a Unix environment.  The program does not work in a simulated Unix environment (MinGW).  Launch the program by running fsaccess.  The program creates a mountable v6 file system.  The file system can be accessed using fsaccess.

Fsaccess accepts the following commands:
"initfs" - initializes the V6 file system.  Parameters of the form (int numBlocks, int numInodes).  Example: "initfs 1000 300"

"cpin" - copies in a file from an exteral source to the v6 file system.  Parameters of the form (string externalFile, string V6File). Example: "cpin myRegFile.txt myV6File.txt"

"cpout"- copies out a file from the v6 file system to an external source.  Parameters of the form (string V6File, string externalFile). Example: "cpout myV6File.txt myRegFile.txt"

"mkdir" - makes a directory chain in the v6 file system.  Parameter of the form (string directoryLocation).  Example: "mkdir /directory1/directory2/directoryZ"

"q" - Quits access to the file system.  No parameters.  Example: "q"

