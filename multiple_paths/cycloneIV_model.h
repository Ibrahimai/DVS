#include <vector>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <assert.h>
#include <algorithm>
#include <ctime>
#include <chrono>


class Path_logic_component
{
public:
	int path;
	int node;
	Path_logic_component() {};
	Path_logic_component(int p, int n) 
	{
		path = p;
		node = n;
	};
};

class Path_node // which part of the FPGA does a path node use
{
	
public:
	int x;
	int y;
	int z;
	Path_node() {};
	Path_node(int xx, int yy, int zz) 
	{
		x = xx;
		y = yy;
		z = zz;
	};

};


class Logic_element
{
	int utilization;
	std::vector<Path_logic_component> nodes;
public:
	Logic_element();
	Logic_element(int over);
	int get_utilization();
	void set_utilization(int x);
	void add_node(int p, int n);
	void remove_overlap(int p, std::vector<int> & deleted_nodes);

};

Logic_element::Logic_element()
{
	nodes.resize(0);
	utilization = 0;
}

Logic_element::Logic_element(int over)
{
	nodes.resize(0);
	utilization = over;
}
int Logic_element::get_utilization()
{
	return utilization;
}
void Logic_element::set_utilization(int x)
{
	utilization = x;
}

void Logic_element::add_node(int p, int n)
{
	Path_logic_component temp (p, n);
	nodes.push_back(temp);
	utilization++;
}

void Logic_element::remove_overlap(int p, std::vector<int> & deletedPaths) // deletes all utilization except path p and returns all deleted paths into deleted_nodes
{
	int i,j;
	bool add;
	for (i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].path != p)
		{
			
			add = true;
			for (j = 0; j < deletedPaths.size(); j++)
			{
				if (deletedPaths[j] == nodes[i].path)
				{
					add = false;
					break;
				}

			}
			if (add)
				deletedPaths.push_back(nodes[i].path);

			utilization--;
			nodes.erase(nodes.begin() + i);
		}
	}

}

Logic_element fpgaLogic[114][72][32]; // size of cyclone IV on DE2 board, got it from chip planner
std::vector< std::vector<Path_node> > paths;