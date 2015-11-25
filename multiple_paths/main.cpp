#include "cycloneIV_model.h"


void delete_paths(std::vector<int> & deletedPaths)
{
	deletedPaths.clear();
	int i,j;
	bool deletedAlready;
	for (i = 1; i < paths.size(); i++)
	{
		deletedAlready = false;
		for (j = 0; j < deletedPaths.size(); j++)
		{
			if (i == deletedPaths[j])
			{
				deletedAlready = true;
				break;
			}
		}
		if (deletedAlready)
			continue;
		for (j = 1; j < paths[i].size(); j++)
		{
			if (fpgaLogic[paths[i][j].x][paths[i][j].y][paths[i][j].z].get_utilization()>1) // delete all paths that uses this component other than path i
			{
			//	fpgaLogic[paths[i][j].x][paths[i][j].y][paths[i][j].z].remove_overlap(i, deletedPaths);
				fpgaLogic[paths[i][j].x][paths[i][j].y][paths[i][j].z].remove_overlap_with_fanin(i, paths[i][j].portIn, deletedPaths);

			}

		}
	}
}

int main(int argc, char* argv[])
{
	if (argc<2)
	{ 
		std::cout << "No Input file was given. Terminating.... /n";
		return 0;
	}
	std::ifstream metaData(argv[1]);
	if (!metaData)
	{
		std::cout << "Can not find file" << argv[1] << "  Terminating.... " << std::endl;
		return 0;
	}
	std::string line;
	std::vector<Path_node> tempPath;
	int path, node;
	path = 0;
	node = 0;
	int i, counter,index1,x,y,z, pIn, pOut;
	int maxOverlap = 0;
	while (std::getline(metaData,line))
	{
		if (line[0] == 'P')
		{
			//if (path > 0)
			//{
				paths.push_back(tempPath);
				tempPath.clear();
			//}
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

	

		if (!std::getline(metaData, line))
		{
			std::cout << "Incorrect file format. Terminating...." << std::endl;
			return 0;
		}
		if (line[0] == 'F')
		{
			pIn = FFd;
			pOut = FFq;
		}
		else
		{
			pIn = line[4] - '0';
			pOut = line[6] - '0';
		}
		// have x and y and z stored
		fpgaLogic[x][y][z].add_node(path, node, pIn, pOut);
		tempPath.push_back(Path_node(x, y, z,pIn,pOut));
		if (node!=0) // skip source registers
			if (fpgaLogic[x][y][z].get_utilization()>maxOverlap)
				maxOverlap = fpgaLogic[x][y][z].get_utilization();
		node++;
	}
	// push last path to list of paths
	paths.push_back(tempPath);
	tempPath.clear();

	std::vector<int> deletedPaths;

	delete_paths(deletedPaths);
	std::sort(deletedPaths.begin(), deletedPaths.end());
	std::cout << deletedPaths.size() << std::endl;
	return 0;
}