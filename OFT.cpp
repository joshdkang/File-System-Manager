#include "OFT.h"
#include <iostream>
using namespace std;

OFT::OFT()
	:buf(new char[64]), curPos(-1), descIndex(-1), length(-1)
{
}

void OFT::init()
{
	curPos = -1; 
	descIndex = -1;
	length = -1;
	emptyAll();
}

void OFT::init(int d, int l)
{
	emptyAll();
	curPos = 0;
	descIndex = d;
	length = l;
}

int OFT::read(char* c)
{
	*c = buf[curPos % 64];
	++curPos;
	return status();
}

int OFT::write(char* c)
{
	buf[curPos % 64] = *c;
	++curPos;
	if(curPos == length + 1)
		++length;
	return status();
}

int OFT::seek(int index)
{
	if(index >= 0 && index <= length)
	{
		curPos = index;
		return 0;
	}
	else 
		return -1;
}

char* OFT::getBuf()
{
	return buf;
}

int OFT::getCurPos()
{
	return curPos;
}

int OFT::getDescIndex()
{
	return descIndex;
}

int OFT::getLength()
{
	return length;
}

int OFT::getBlock()
{
	return ((curPos / 64) + 1);
}

int OFT::status()
{
	if(curPos == 192)
		return 4;
	else if(curPos == length)
	{
		switch (curPos)
		{
			case 0: return 1;
			case 64: return 2;
			case 128: return 3;
			default: return 0;
		}
	}
	else if(curPos < length)
	{
		switch (curPos)
		{
			case 0: return -1;
			case 64: return -2;
			case 128: return -3;
			default: return 0;
		}
	}
	else
		return 0;
}

void OFT::emptyAll()
{
	for (int i = 0; i < 64; ++i)
		buf[i] = '\0';
}

OFT::~OFT()
{
	delete[] buf;
}
