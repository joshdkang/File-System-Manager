#ifndef OFT_H
#define OFT_H
#include <iostream>

using namespace std;

class OFT
{
public:
	OFT();
	void init();
	void init(int i, int j);
	int read(char* c);
	int write(char* c);
	int seek(int index);
	char* getBuf();
	int getCurPos();
	int getDescIndex();
	int getLength();
	int getBlock();
	int status();
	void emptyAll();
	~OFT();
private:
	char* buf;
	int curPos;
	int descIndex;
	int length;
};

#endif
