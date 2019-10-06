#include "FileSystem.h"
using namespace std;

string filepath = "";

FileSystem::FileSystem()
{
	for(int i = 0; i < 64; ++i)
		buffer[i] = '\0';
	for(int i = 0; i < 64; ++i)
		ldisk.write_block(i, buffer);
	for(int i = 31; i >= 0 ; --i)
		mask[i] = 1 << (31-i);
	init();
}

string FileSystem::init()
{
	((unsigned int*)buffer)[0] = mask[0];
	((unsigned int*)buffer)[0] = ((unsigned int*)buffer)[0] | mask[1] | mask[2] | mask[3] | mask[4] | mask[5] | mask[6];
	((unsigned int*)buffer)[1] = mask[1] & mask[2];

	ldisk.write_block(0, buffer);
	initDesc();

	for(int i = 0; i < 4; ++i)
		oft[i].init();
	
	oft[0].init(0, 0);
	ldisk.read_block(7, oft[0].getBuf());
	
	return "disk initialized";
}

void FileSystem::initDesc()
{
	descriptor[0] = 0;
	for(int i = 1; i < 4; ++i)
		descriptor[i] = -1;
	
	ldisk.read_block(1, buffer);
	for(int i = 0; i < 4; ++i)
		((int*)buffer)[i] = descriptor[i];
	ldisk.write_block(1, buffer);
	
	for(int i = 0; i < 4; ++i)
		descriptor[i] = -1;
	
	for(int i = 1; i < 24; ++i)
	{
		ldisk.read_block(1 + (i / 4), buffer);
		for(int j = 0; j < 4; ++j)
			((int*)buffer)[j + (i % 4) * 4] = descriptor[j];
		ldisk.write_block(1 + (i / 4), buffer);
	}
}

string FileSystem::init(string* filename)
{
	char blank[1];
	ifstream iFile(filepath+(*filename), ifstream::in);
	try
	{
		if(iFile)
		{
			for(int i = 0; i < 64; ++i)
			{
				for(int j = 0; j < 64; ++j)
				{
					iFile.read(buffer+j, 1);
					if (j % 4 == 3)
						iFile.read(blank, 1);
				}
				iFile.read(blank, 1);
				ldisk.write_block(i, buffer);
			}
			for(int i = 0; i < 4; ++i)
				oft[i].init();
			
			ldisk.read_block(1, buffer);
			for(int i = 0; i < 4; ++i)
				descriptor[i] = ((int*)buffer)[i];
			oft[0].init(0, descriptor[0]);
			ldisk.read_block(descriptor[1], oft[0].getBuf());
			
			iFile.close();

			return "disk restored";
		}
	}
	catch (...) {}
	
	return init();
}

string FileSystem::create (char* filename)
{
	int openDesc = -1;
	char temp[4] = {0};
	string name(filename);
	char c;

	try
	{
		if(searchDir(filename) != oft[0].getLength())
			throw c;

		//find free descriptor and check if descriptors are full
		for(int i = 0; i < 6; ++i)
		{
			ldisk.read_block(i + 1, buffer);
			for(int j = 0; j < 4; ++j)
			{
				if(((int*)buffer)[j * 4] == -1)
				{
					openDesc = ((i) * 4) + j;
					goto Directory;
				}
			}
		}

		if(openDesc == -1)
			throw c;
		//find a free directory entry
	
		Directory:
		lseek(0,0);
		searchDir(temp);
		if(oft[0].getCurPos() == 192)
			throw c;

		//fill both entries
		for(int i = 0; i < 4; ++i)
			write(0, &(filename[i]), 1);
		for(int i = 0; i < 4; ++i)
			write(0, &(((char*)&openDesc)[i]), 1);
	
		descriptor[0] = 0;
		for(int i = 1; i < 4; ++i)
			descriptor[i] = -1;

		ldisk.read_block(1 + (openDesc / 4), buffer);
		for(int i = 0; i < 4; ++i)
			((int*)buffer)[i + (openDesc % 4) * 4] = descriptor[i];
		ldisk.write_block(1 + (openDesc / 4), buffer);

		return name + " created";
	}
	catch (...)
	{
		return "error";
	}
}

bool FileSystem::destroy(char* filename)
{
	int b;
	lseek(0, 0);
	char n[8] = {0};

	try
	{	
		if(searchDir(filename) == oft[0].getLength())
			return false;
	
		read(0, buffer, 8);
		int d = (int)(((int*)buffer)[1]);
		
		ldisk.read_block(1 + (d/4), buffer);				
		for(int i = 0; i < 4; ++i)
			descriptor[i] = ((int*)buffer)[i + (d % 4) * 4];		
		
		for(int i = 1; i < 4; ++i)
		{
			if(oft[i].getDescIndex() == d);
			{
				close(i);
				break;
			}
		}
	
		ldisk.read_block(0, buffer);
		for (int i = 1; i < 4; ++i) 
		{
			b = descriptor[i];
			if (b != -1)
				((unsigned int*)buffer)[b / 32] = ((unsigned int*)buffer)[b / 32] & ~mask[b % 32];
			descriptor[i] = -1;
		}
		ldisk.write_block(0, buffer);								
		descriptor[0] = -1;
		
		ldisk.read_block(1 + (d / 4), buffer);
		for(int i = 0; i < 4; ++i)
			((int*)buffer)[i + (d % 4) * 4] = descriptor[i];
		ldisk.write_block(1 + (d / 4), buffer);
		searchDir(filename);
		write(0, n, 8);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

int FileSystem::open(char* filename)
{
	lseek(0, 0);
	string c;

	try
	{	
		if(searchDir(filename) == oft[0].getLength())
			throw "File does not exist";
	
		read(0, buffer, 8);
		int d = ((int*)buffer)[1];
		ldisk.read_block(1 + (d / 4), buffer);
		for(int i = 0; i < 4; ++i)
			descriptor[i] = ((int*)buffer)[i + (d % 4) * 4];
	
		for(int i = 1; i < 4; ++i)
		{
			if(oft[i].getDescIndex() == ((int*)buffer)[0])
				throw "File opened already";
		}
	
		for(int i = 1; i < 4; ++i)
		{
			if(oft[i].getDescIndex() == -1)
			{
				oft[i].init(d, descriptor[0]);
				return i;
			}
		}
	}
	catch (...)
	{
		return -1;
	}
}

int FileSystem::close (int index)
{
	int d = oft[index].getDescIndex();
	int b;
	int status;

	try
	{
		if(d != -1)
		{
			ldisk.read_block(1 + (d / 4), buffer);
			for(int i = 0; i < 4; ++i)
				descriptor[i] = ((int*)buffer)[i + (d % 4) * 4];

			((int*)descriptor)[0] = oft[index].getLength();

			ldisk.read_block(1 + (d / 4), buffer);
			for(int i = 0; i < 4; ++i)
				((int*)buffer)[i + (d % 4) * 4] = descriptor[i];
			ldisk.write_block(1 + (d / 4), buffer);
		
			b = oft[index].getBlock();
			status = oft[index].status();

			if(status < -1 || status > 1)
			{
				b -= 1;
				ldisk.write_block(descriptor[b], oft[index].getBuf());
			}
			else if (status == 1)
			{
				oft[index].init();
				return index;
			}
			else
			{
				ldisk.write_block(descriptor[b], oft[index].getBuf());
			}
			oft[index].init();
			return index;
		}
	}
	catch (...)
	{
		return -1;
	}
}

string FileSystem::save(string* filename)
{
	string location = filepath;
	int i = 3;
	while(i >= 0)
	{
		if(oft[i].getDescIndex() != -1)
			close(i);
		--i;
	}

	ofstream ofile(location+(*filename), ofstream::out);
	if(ofile)
	{
		for(int i = 0; i < 64; ++i)
		{
			ldisk.read_block(i, buffer);
			for(int j = 0; j < 64; ++j)
			{
				ofile << buffer[j];
				if(j % 4 == 3)
					ofile << " ";
			}
			ofile << endl;
		}
		ofile.close();
		init();
		return "disk saved";
	}
	return "save failed";
}

int FileSystem::read(int index, char* memArea, int count)
{
	int status = oft[index].status();
	int d = oft[index].getDescIndex();
	int c = count;

	ldisk.read_block( 1 + (d / 4), buffer);
	for(int i = 0; i < 4; ++i)
		descriptor[i] = ((int*)buffer)[i + (d % 4) * 4];

	for(int i = 0; ((oft[index].getLength() > oft[index].getCurPos()) && (status <= 0) && (c > 0)) ; ++i, --c)
	{
		if(status < 0)
			ldisk.read_block(descriptor[-1 * status], oft[index].getBuf());
		status = oft[index].read(&memArea[i]);
	}
	return c;
}

int FileSystem::write(int index, char* memArea, int count)
{
	int status = oft[index].status();
	int d = oft[index].getDescIndex();
	int c = count;
	int t = 1;
	int b = -1;

	ldisk.read_block(1 + (d/4), buffer);
	for(int i = 0; i < 4; ++i)
		descriptor[i] = ((int*)buffer)[i + (d % 4) * 4];

	for(; c > 0 && status != 4; --c)
	{
		if (status < 0)
		{
			if(status < -1)
				ldisk.write_block(descriptor[(-1) * status - 1], oft[index].getBuf());
			ldisk.read_block(descriptor[(-1) * status], oft[index].getBuf());
		}
		else if (status > 0)
		{
			if (status > 1 && status != 5)
				ldisk.write_block(descriptor[status - 1], oft[index].getBuf());
			if (status != 4)
			{
				ldisk.read_block(0, buffer);
				for(int i = 0; i < 2 && b == -1; ++i)				
				{
					for(int j = 0; j < 32 && b == -1; ++j)
					{
						if((((unsigned int*)buffer)[i] & mask[j]) == 0)
						{
							b = i * 32 + j;
							break;
						}
					}
				}
				
				if (b == -1)
					return c;

				ldisk.read_block(0, buffer);
				((unsigned int*)buffer)[b / 32] = ((unsigned int*)buffer)[b / 32] | mask[b % 32];
				ldisk.write_block(0, buffer);
				descriptor[status] = b;
				descriptor[0] = oft[index].getCurPos();
				
				ldisk.read_block(1 + (d / 4), buffer);
				for(int i = 0; i < 4; ++i)
					((int*)buffer)[i + (d % 4) * 4] = descriptor[i];
				ldisk.write_block(1 + (d / 4), buffer);
				
				oft[index].emptyAll();
			}
			else
			{
				return c;
			}
		}
		status = oft[index].write(memArea);
	}	
	return c;
}

string FileSystem::directory()
{
	lseek(0, 0);
	string result = "";
	int d = oft[0].getDescIndex();
	char name[4] = {0};
	
	ldisk.read_block(1 + ( d / 4 ), buffer);
	for(int i = 0; i < 4; ++i)
		descriptor[i] = ((int*)buffer)[i + (d % 4) * 4];
	
	int l = oft[0].getLength() / 8;

	if(oft[0].getLength() == 0)
		return result;
		
	for(int i = 0; i < l; ++i)
	{
		read(0, name, 4);
		if(name[0] != '\0')
		{
			result.append(string(name));
			result.append(" ");
		}
		read(0, name, 4);
	}
	return result;
}

int FileSystem::lseek(int index, int pos)
{
	int d = oft[index].getDescIndex();

	ldisk.read_block(1 + (d / 4), buffer);
	for(int i = 0; i < 4; ++i)
		descriptor[i] = ((int*)buffer)[i + (d % 4) * 4];

	int b = oft[index].getBlock();
	int status = oft[index].status();	

	if(status == 0)
		ldisk.write_block(descriptor[b], oft[index].getBuf());
	else if(status != -1 && status != 1)
		ldisk.write_block(descriptor[b - 1], oft[index].getBuf());
	
	oft[index].seek(pos);
	status = oft[index].status();
	b = oft[index].getBlock();

	if(status <= 0)
		ldisk.read_block(descriptor[b], oft[index].getBuf());

	return pos;
}

int FileSystem::searchDir(char* filename) 
{
	lseek(0, 0);
	char query[8];
	for(;oft[0].getCurPos() < oft[0].getLength();)
	{
		read(0, query, 8);
		if((int)*((int*)query) == (int)*((int*)filename))
		{
			lseek(0, oft[0].getCurPos() - 8);
			return oft[0].getCurPos();
		}
	}
	return oft[0].getCurPos();
}

FileSystem::~FileSystem() {} 

