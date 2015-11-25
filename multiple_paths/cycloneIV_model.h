#include <vector>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <assert.h>
#include <algorithm>
#include <ctime>
#include <chrono>

#define portA (0)
#define portB (1)
#define portC  (2)
#define portD (3)
#define Cin (4)
#define Cout (5)
#define Combout (6)
#define FFd (7)
#define FFq (8)

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
	int portIn;
	int portOut;
	bool redun;
	int eqPath;
	int eqNode;
	Path_node() {};
	Path_node(int xx, int yy, int zz, int in, int out) 
	{
		x = xx;
		y = yy;
		z = zz;
		portIn = in;
		portOut = out;
	};



};
std::vector< std::vector<Path_node> > paths;

class Logic_element
{
	int utilization;
	int usedInputPorts;
	bool inputPorts[5];
	std::vector<Path_logic_component> nodes;
public:
	Logic_element();
	Logic_element(int over);
	int get_utilization();
	void set_utilization(int x);
	void add_node(int p, int n, int in, int out);
	void remove_overlap(int p, std::vector<int> & deleted_nodes);
	void remove_overlap_with_fanin(int p, int pIn, std::vector<int> & deleted_nodes);
};

Logic_element::Logic_element()
{
	nodes.resize(0);
	utilization = 0;
	inputPorts[portA] = false;
	inputPorts[portB] = false;
	inputPorts[portC] = false;
	inputPorts[portD] = false;
	inputPorts[Cin] = false;
	usedInputPorts = 0;
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

void Logic_element::add_node(int p, int n, int in, int out)
{
	
	int count = 0;
	int i;

	if (in < 5) // if input higher than 4 then this is a FF
	{
		inputPorts[in] = true;
		for (i = 0; i < 5; i++)
			if (inputPorts[i])
				count++;
	}
	else
		count = 1;
	usedInputPorts = count;
	Path_logic_component temp (p, n);
	nodes.push_back(temp);
	utilization++;
}

void Logic_element::remove_overlap(int p, std::vector<int> & deletedPaths) // deletes all utilization except path p and returns all deleted paths into deleted_nodes
{
	int i,j;
	bool add;
	std::vector<int> indexToBeDeleted;
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
			indexToBeDeleted.push_back(i);
		//	nodes.erase(nodes.begin() + i);
		}
	}

	for (i = 0; i < indexToBeDeleted.size();i++)
		nodes.erase(nodes.begin() + (indexToBeDeleted[i]-i));

}



void Logic_element::remove_overlap_with_fanin(int p, int pIn, std::vector<int> & deletedPaths) // deletes all utilization except path p and returns all deleted paths into deleted_nodes
{
	int i, j;
	bool add;
	std::vector<int> indexToBeDeleted;
	for (i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].path != p && paths[nodes[i].path][nodes[i].node].portIn !=pIn)
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
			indexToBeDeleted.push_back(i);
			//	nodes.erase(nodes.begin() + i);
		}
	}

	for (i = 0; i < indexToBeDeleted.size(); i++)
		nodes.erase(nodes.begin() + (indexToBeDeleted[i] - i));

}

Logic_element fpgaLogic[114][72][32]; // size of cyclone IV on DE2 board, got it from chip planner
