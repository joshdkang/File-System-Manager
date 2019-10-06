#ifndef FILESYSTEM_H
#define FILESYSTEM_H

/*
ldisk: 64 blocks
block = 64 bytes = 16 integers
block 0 holds bitmap: 64 bits (one per block) = 2 integers
descriptor: 4 integers: 
	file length (1 int)
	3 block #s
# of descriptors: 24 = 6 blocks
descriptor 0 describes the directory
each directory entry: 2 integers 
	file name: max 4 chars, no extensions (1 int) 
	descriptor index: 1 int
*/

#include <iostream>
#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <fstream>
#include "IO.h"
#include "OFT.h"

using namespace std;

class FileSystem
{
public:
	FileSystem();
	string init();
	string init(string* filename);
	void initDesc();
	string create(char* filename);
	bool destroy(char* filename);
	int open(char* filename);
	int close(int index);
	int read(int index, char* memArea, int count);
	int write(int index, char* memArea, int count);
	string save(string* filename);
	string directory();
	int lseek(int index, int pos);
	int searchDir(char* filename);
	~FileSystem();

private:
	char buffer[64];
	unsigned int mask[32];
	IO ldisk;
	OFT oft[4];
	int descriptor[4];
};

#endif
