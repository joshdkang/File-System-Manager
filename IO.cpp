#include "IO.h"
using namespace std;

IO::IO()
{
}

void IO::read_block(int i, char*p)
{
	try
	{
		for(int j = 0; j < 64; ++j)
		{
			p[j] = ldisk[i][j];
		}
	}
	catch(...) {}
}

void IO::write_block(int i, char* p)
{
	try
	{
		for(int j = 0; j < 64; ++j)
		{
			ldisk[i][j] = p[j];
		}
	}
	catch(...) {}
}

IO::~IO() {}
