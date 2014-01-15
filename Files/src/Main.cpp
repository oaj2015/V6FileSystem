#include "P2.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using std::cout;
using std::cin;
int main(int argc, char** argv) {
	std::string line;
	std::string input;
	int fd = 0;
	std::string NOT_INITIALIZED = "fsaccess: file system not initialized.";
	while(true) {
		cout << "$ ";
		std::getline(std::cin, line);

		std::stringstream ss;
		ss.str(line);
		ss >> input;
		dout << input << " " << fd << std::endl;

		if(input == "q") {
			close(fd);
			return 0;
		}
		else if(input == "initfs") {
			std::string name;
			int n1, n2;
			ss >> name >> n1 >> n2;
			fd = initfs(name, n1, n2);
		}
		else if(input == "cpin") {
			if(fd == 0) {
				cout << NOT_INITIALIZED << std::endl; 

			} else {
				std::string external_name, v6_name;
				ss >> external_name >> v6_name;
				cpin(fd, external_name, v6_name);
			}
		}
		else if(input == "cpout") {
			if(fd == 0) {
				cout << NOT_INITIALIZED << std::endl; 
			} else {
				std::string v6_name, external_name;
				ss >> v6_name >> external_name;
				cpout(fd, v6_name, external_name);
			}
		}
		else if(input == "mkdir") {
			if(fd == 0) {
				cout << NOT_INITIALIZED << std::endl; 
			} else {
				std::string name;
				ss >> name;
				mkdir(fd, name);
			}
		}
		else {
			cout << "fsacess: " << input << ": command not found." << std::endl;
		}
	}
}


