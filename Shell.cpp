#include "FileSystem.h"
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <cstring>
using namespace std;

int main()
{
	FileSystem File;

	string location = "";
	string filename = "";
	string line;
	string command;
	string current;
	string secondInput;
	string thirdInput;
	string out = "";
	string name;
	string disk_cont;

	int index;
	int pos;
	int count;
	char charWrite;
	char charName[4] = {0};
	char s;

	cout << "Enter file name:";
	cin >> filename;

	ofstream output(location +"result.txt", ofstream::out);
	ifstream iFile(location + filename + ".txt", ifstream::in);

	while (iFile >> command)
	{
		try 
		{
			if (command == "cr")
			{
				iFile >> charName;
				string crResult;
				crResult = File.create(charName);
				output << crResult << endl;
			}
			else if (command == "de")
			{
				iFile >> name;
				strcpy(charName, name.c_str());
				if(File.destroy(charName) == true)
				output << name << " destroyed " << endl;
			}
			else if (command == "op")
			{
				iFile >> charName;
				index = File.open(charName);
				output << charName << " opened " << index << endl;
			}
			else if (command == "cl")
			{
				iFile >> index;
				if (index < 1)
					throw s;
				index = File.close(index);
				output << index << " closed" << endl;
			}
			else if (command == "rd")
			{
				iFile >> index >> count;
				char buffer[192] = {0};
				count = count - File.read(index, buffer, count);
				for(int i = 0; buffer[i] != '\0'; ++i)
					output << buffer[i];
				output << endl;
			}
			else if (command == "wr")
			{
				iFile >> index >> charWrite >> count;
				if (index < 1)
					throw s;
				
				count = count - File.write(index, &charWrite, count);
				output << count << " bytes written" << endl;
			}
			else if (command == "sk")
			{
				iFile >> index >> pos;
				pos = File.lseek(index, pos);
				output << "position is " << pos << endl;
			}
			else if (command == "dr")
			{ 
				out = File.directory();
				output << out << endl;
			}
			else if (command == "in")
			{
				if (iFile.peek() == ' ')
				{
					iFile >> disk_cont;
					out = File.init(&disk_cont);
				}
				else
				{
					out = File.init();
					disk_cont = "";
				}
				output << out << endl;
			}
			else if (command == "sv")
			{
				string svResult;
				iFile >> disk_cont;
				svResult = File.save(&disk_cont);
				output << svResult << endl;
			}
 		}
		catch (exception s)
		{
			output << "error" << endl;
		}
	}

	iFile.close();
	output.close();
}
