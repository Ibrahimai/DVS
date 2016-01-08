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

#define ControlLE

#define InputPortSize (5)
#define OutputPortSize (2)

#define FPGAsizeX (114)
#define FPGAsizeY (72)
#define FPGAsizeZ	(32)


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
	int testPhase; // which test wave is this path in 
	bool redun; // is true if there is at least one path that use the same node with same input and output
	bool deleted;
	bool inverting; // determine the behaviour of the edge across this node (ff,rr->noninverting, or fr,rf->inverting)
	int eqPath; // if redun is true, eqPath stores the most critical path that uses the same node with the same input and output
	int eqNode; // if redun is true, ewNode stores the node of the eqPath that uses this node equivalently.
	Path_node() {};
	Path_node(int xx, int yy, int zz, int in, int out, bool invert) 
	{
		x = xx;
		y = yy;
		z = zz;
		portIn = in;
		portOut = out;
		redun = false;
		eqPath = 0;
		eqNode = 0;
		deleted = false;
		testPhase = -1; // -1
		inverting = invert;
	};



};
std::vector< std::vector<Path_node> > paths; // model the paths

class Logic_element
{
public:
	int utilization;
	int usedInputPorts;
	int usedOutputPorts;
	bool inputPorts[InputPortSize];
	bool outputPorts[OutputPortSize];
	std::vector<Path_logic_component> nodes; // list of nodes representing which path and node use this le
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
	outputPorts[Cout-5] = false;
	outputPorts[Combout-5] = false;
	usedInputPorts = 0;
	usedOutputPorts = 0;
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
		outputPorts[out - 5] = true;
		for (i = 0; i < 5; i++)
			if (inputPorts[i])
				count++;

		if (outputPorts[0]&&outputPorts[1])
			usedOutputPorts = 2;
		else
			usedOutputPorts = 1;
	}
	else
	{
		count = 1;
		usedOutputPorts = 1;
	}
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

			if (nodes[i].path < p && add)
				std::cout << "smthng wrong" << std::endl;


		//	utilization--;
		//	indexToBeDeleted.push_back(i);
		//	nodes.erase(nodes.begin() + i);
		}
	}

//	for (i = 0; i < indexToBeDeleted.size();i++)
//		nodes.erase(nodes.begin() + (indexToBeDeleted[i]-i));

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
			if (paths[p][0].testPhase != paths[nodes[i].path][0].testPhase)
				continue;

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

			if (nodes[i].path < p && add)
				std::cout << "smthng wrong" << std::endl;


		//	utilization--;
		//	indexToBeDeleted.push_back(i);
			//	nodes.erase(nodes.begin() + i);
		}
	}

//	for (i = 0; i < indexToBeDeleted.size(); i++)
//		nodes.erase(nodes.begin() + (indexToBeDeleted[i] - i));

}

Logic_element fpgaLogic[114][72][32]; // size of cyclone IV on DE2 board, got it from chip planner, model the logic elements of the chip
std::vector < std::vector <bool>> testingPhases;
int numberOfTestPhases;




void assign_test_phases_ib();
void unkown(std::vector < std::vector <int> > & pathRelationGraph, int i, Path_node  tempCell);
void generate_pathRelationGraph(std::vector < std::vector <int> > & pathRelationGraph);
void delete_path(int path); // delete a given path by setting the first node of the path as deleted and reducing utilization of all its nodes, and adjusting portIn and poirtOut of fpgalogic according to the deleted action. The only thing left the same is fpgalogic.nodes array.