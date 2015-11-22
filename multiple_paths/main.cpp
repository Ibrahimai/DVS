#include "cycloneIV_model.h"

int main(int argc, char* argv[])
{
	if (argc<2)
	{ 
		std::cout << "No Input file was given. Terminating.... /n";
		return 0;
	}
	std::ifstream locationConst(argv[1]);
	if (!locationConst)
	{
		std::cout << "Can not find file" << argv[1] << "  Terminating.... " << std::endl;
		return 0;
	}
	std::string line;
	int path, node;
	path = 0;
	node = 0;
	char buf[512];
	int i, counter,index1,x,y,z;
	while (std::getline(locationConst,line))
	{
		if (line[0] == 'P')
		{
			path++;
			node = 0;
			continue;
		}
		counter = 0;
		for (i = 0; i < line.size(); i++)
		{
			if (line[i] == '_')
				counter++;
			if (counter == 3)
			{
				index1 = i+2;
				break;
			}
		}

		for (i = index1; i < line.size(); i++)
		{
			if (!isdigit(line[i]))
				break;
		}
		x = stoi(line.substr(index1, i - index1));
		index1 = i + 2;
		for (i=index1; i < line.size(); i++)
		{
			if (!isdigit(line[i]))
				break;
		}
		y = stoi(line.substr(index1, i - index1));

		index1 = i + 2;
		for (i = index1; i < line.size(); i++)
		{
			if (!isdigit(line[i]))
				break;
		}
		z = stoi(line.substr(index1, i - index1));

		// have x and y and z stored
		fpgaLogic[x][y][z].add_node(path, node);
		node++;
	}

	return 0;
}