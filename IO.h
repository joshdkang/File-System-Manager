#ifndef IO_H
#define IO_H
#include <iostream>

using namespace std;

class IO
{
public:
	IO();
	void read_block(int i, char* p);
	void write_block(int i, char* p);
	~IO();

private:
	char ldisk[64][64];
};

#endif
