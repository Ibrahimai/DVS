#include "cycloneIV_model.h"

void set_testing_phase(int fixed, int change)
{
	int phase1 = paths[fixed][0].testPhase;
	int phase2 = paths[change][0].testPhase;
	if ( phase1 != phase2 )
		return;
	paths[change][0].testPhase++;
	if (paths[change][0].testPhase > numberOfTestPhases - 1)
		numberOfTestPhases = paths[change][0].testPhase + 1;
	// change phase 2 to the inimum allowed phase
}

void assign_test_phases() // incorrect algorithm, see assign_test_phases_2 // assuming  one output per LUT (this outpu can have multiple fan-outs)
{
	unsigned int i, j,k;
	int tempComponentX, tempComponentY, tempComponentZ;
	Path_logic_component tempNode;
	for (i = 0; i < paths.size(); i++) // loop across all paths, another approach would be to loop across used logic elements
	{
		for (j = 0; j < paths[i].size(); j++) // loop across nodes in that path
		{
			tempComponentX = paths[i][j].x;
			tempComponentY = paths[i][j].y;
			tempComponentZ = paths[i][j].z;
			for (k = 0; k < fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes.size(); k++) // loop across all nodes sharing the LE used by node j in path i
			{
				tempNode = fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[k];
				if (i == tempNode.path) // samw path
					continue;
				if (paths[i][j].portIn != paths[tempNode.path][tempNode.node].portIn) // diffenent paths and use different inputs
				{
					if (i > tempNode.path) // paths more critical than the current path should have been handeled already
					{
					//	assert(paths[i][0].testPhase != paths[tempNode.path][0].testPhase);
						if (paths[i][0].testPhase == paths[tempNode.path][0].testPhase)
							std::cout << "error ya she2oo \n" << std::endl;
					//	continue;
					}
					// change the testing phase of path tempNode.path
					if (tempNode.path == 300)
						std::cout << "fef" << std::endl;
					set_testing_phase(i, tempNode.path);
				}

			}
			 
		}
	}
}


void add_connection(std::vector < std::vector <int> > & pathRelationGraph, int x, int y) // x and y paths that should not be tested together
{
	bool connExist = false;
	int i, j;
	if (x == 0 || y == 0)
		std::cout << "efef";
	for (i = 0; i < pathRelationGraph[x].size(); i++) // loop across all neighbours of x
	{
		if (pathRelationGraph[x][i] == y) // if y is already connected to x then dont add it
		{
			connExist = true;
			break;
		}
	}

	if (!connExist) // no connection exists, so add connection
	{
		if (x  == y)
			std::cout << y << std::endl;
		pathRelationGraph[x].push_back(y);
		pathRelationGraph[y].push_back(x);
	}
	
}



void generate_pathRelationGraph(std::vector < std::vector <int> > & pathRelationGraph)
{
	pathRelationGraph.resize(paths.size());
	unsigned int i, j, k, x, y,kk;
	bool shouldDelete = false;
	int tempComponentX, tempComponentY, tempComponentZ;
	int deletedPaths = 0;
	Path_logic_component tempNode;
	std::vector < Path_logic_component > rootNodes;
	std::vector <bool> inputs(InputPortSize, false);
//	Logic_element tempCell;
	for (i = 0; i < paths.size(); i++) // loop across all paths, another approach would be to loop across used logic elements
	{
		for (j = 0; j < paths[i].size(); j++) // loop across nodes in that path
		{
			if (paths[i][0].deleted) // this path is deleted then continue to the next path
				break;
			tempComponentX = paths[i][j].x;
			tempComponentY = paths[i][j].y;
			tempComponentZ = paths[i][j].z;

			for (k = 0; k < fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes.size(); k++) // loop across all nodes sharing the LE used by node j in path i
			{
				tempNode = fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[k];
				if (i == tempNode.path) // same path
					continue;
				////// trial stuff
				if (paths[tempNode.path][0].deleted) // if this path is deleted then dont do anything
					continue;
				if (paths[i][j].portIn != paths[tempNode.path][tempNode.node].portIn) // diffenent paths and use different inputs
				{
					if (!inputs[paths[tempNode.path][tempNode.node].portIn]) // first time to see a path using this input
					{
						inputs[paths[tempNode.path][tempNode.node].portIn] = true;
						rootNodes.push_back(tempNode);
					}

				}

			}
			assert(rootNodes.size() < InputPortSize-1);
			////// if we can control the outpuf of an LE then just ensure that all paths using any node before rootNodes are no tested at the same time as path i
#ifdef ControlLE
			for (k = 0; k < rootNodes.size(); k++)
			{ 
				tempNode = rootNodes[k];
				tempComponentX = paths[tempNode.path][tempNode.node-1].x; // get the location of the LE feeding the node in rootNodes
				tempComponentY = paths[tempNode.path][tempNode.node-1].y;
				tempComponentZ = paths[tempNode.path][tempNode.node-1].z;
				//// trial studd
				for (kk = 0; kk < fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes.size(); kk++) // check the presence of the special reconvergent fanout, if it exists then delete the (less critical) path causing this
				{
					if (i == fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[kk].path)
					{
						shouldDelete = true;
						paths[tempNode.path][0].deleted = true;
						std::cout << "deleted : " << tempNode.path << std::endl;
						deletedPaths++;
					}

				}

				if (!shouldDelete)
				{
					for (kk = 0; kk < fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes.size(); kk++) // tod0: currently we fix cout and combout together, if we need to fix combout we also fix cout. This is not necessary we could change that.
					{
						//		if (paths[tempNode.path][tempNode.node - 1].portOut == paths[fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[kk].path][fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[kk].node].portOut)
						if (!paths[fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[kk].path][0].deleted)
							add_connection(pathRelationGraph, i, fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[kk].path);
						//		else
						//			std::cout << "7assal" << std::endl;
					}
				}
				shouldDelete = false;;
			
			}
			
#endif

#ifndef ControlLE
			///// if no ability to control output of LE then must recurse back to all sources that can affect each node in rootNodes and disable them
			for (k = 0; k <rootNodes.size(); k++)
			{
			//	tempCell = fpgaLogic[paths[rootNodes[k].path][rootNodes[k].node - 1].x][paths[rootNodes[k].path][rootNodes[k].node - 1].y][paths[rootNodes[k].path][rootNodes[k].node - 1].z];
				
				unkown(pathRelationGraph, i, paths[rootNodes[k].path][rootNodes[k].node - 1]);
			}
#endif
			rootNodes.clear();
			std::fill(inputs.begin(), inputs.end(), false);
		}
	}
	//// add edges between cascaded paths
	for (i = 1; i < paths.size(); i++) // loop over all paths
	{
		for (j = 0; j < fpgaLogic[paths[i][0].x][paths[i][0].y][paths[i][0].z].nodes.size(); j++) // loop across nodes using the source of the ith pass
		{
			if (fpgaLogic[paths[i][0].x][paths[i][0].y][paths[i][0].z].nodes[j].node != 0) // if a path uses this node and not at its first node then these two paths are cascaded
			{
				std::cout << "7assal CASCADE " << std::endl;
				add_connection(pathRelationGraph, i, fpgaLogic[paths[i][0].x][paths[i][0].y][paths[i][0].z].nodes[j].path);

			}
		}
	}

	std::cout << "NUmber of Deleted Paths " << deletedPaths << std::endl;
}

void unkown(std::vector < std::vector <int> > & pathRelationGraph, int i, Path_node  tempCell)
{
	unsigned int  j, k, x, y;
	bool connExist = false;
	Path_logic_component tempNode;
	std::vector < Path_logic_component > rootNodes;
	std::vector <bool> inputs(InputPortSize, false);

	if (tempCell.z % 2 == 1) // this is a register, all paths starting at this register can not be tested with path i
	{
		for (y = 0; y < fpgaLogic[tempCell.x][tempCell.y][tempCell.z].nodes.size(); y++)
		{
			tempNode = fpgaLogic[tempCell.x][tempCell.y][tempCell.z].nodes[y];
			// connect path i and path tempNode.path in the pathRelationGraph
			// add connection to path i
			connExist = false;
			for (x = 0; x < pathRelationGraph[i].size(); x++)
			{
				if (pathRelationGraph[i][x] == tempNode.path) // edge already exists
				{
					connExist = true;
					break;
				}
			}
			if (!connExist)
			{
				pathRelationGraph[i].push_back(tempNode.path);
				if (i == tempNode.path)
					std::cout << i << std::endl;
			}
			connExist = false;
			// add connection to path rempNode.path
			for (x = 0; x < pathRelationGraph[tempNode.path].size(); x++)
			{
				if (pathRelationGraph[tempNode.path][x] == i) // edge already exists
				{
					connExist = true;
					break;
				}
			}
			if (!connExist)
				pathRelationGraph[tempNode.path].push_back(i);

			connExist = false;

		}
	}
	else
	{
		for (k = 0; k < fpgaLogic[tempCell.x][tempCell.y][tempCell.z].nodes.size(); k++) 
		{
			tempNode = fpgaLogic[tempCell.x][tempCell.y][tempCell.z].nodes[k];
			if (i == tempNode.path) // same path
				continue;
			if (!inputs[paths[tempNode.path][tempNode.node].portIn]) // first time to see a path using this input
			{
				inputs[paths[tempNode.path][tempNode.node].portIn] = true;
				rootNodes.push_back(tempNode);
			}
		}
		assert(rootNodes.size() < InputPortSize - 1);
		for (k = 0; k <rootNodes.size(); k++)
		{
			unkown(pathRelationGraph, i, paths[rootNodes[k].path][rootNodes[k].node - 1]);
		}
		rootNodes.clear();
		std::fill(inputs.begin(), inputs.end(), false);
	}



}

void delete_especial_reconvergent_fanout()
{
	
	unsigned int i, j, k, x, y, kk;
	int tempComponentX, tempComponentY, tempComponentZ;
	int deletedPaths = 0;
	Path_logic_component tempNode;
	std::vector < Path_logic_component > rootNodes;
	//std::vector <bool> inputs(InputPortSize, false);
	//	Logic_element tempCell;
	for (i = 0; i < paths.size(); i++) // loop across all paths, another approach would be to loop across used logic elements
	{
		for (j = 0; j < paths[i].size(); j++) // loop across nodes in that path
		{
			if (paths[i][0].deleted) // this path is deleted then continue to the next path
				break;
			tempComponentX = paths[i][j].x;
			tempComponentY = paths[i][j].y;
			tempComponentZ = paths[i][j].z;

			for (k = 0; k < fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes.size(); k++) // loop across all nodes sharing the LE used by node j in path i
			{
				tempNode = fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[k];
				if (i == tempNode.path) // same path
					continue;
				////// trial stuff
				if (paths[tempNode.path][0].deleted) // if this path is deleted then dont do anything
					continue;
				if (paths[i][j].portIn != paths[tempNode.path][tempNode.node].portIn) // diffenent paths and use different inputs
				{

						rootNodes.push_back(tempNode);


				}

			}
				////// if we can control the outpuf of an LE then just ensure that all paths using any node before rootNodes are no tested at the same time as path i

			for (k = 0; k < rootNodes.size(); k++)
			{
				tempNode = rootNodes[k];
				tempComponentX = paths[tempNode.path][tempNode.node - 1].x; // get the location of the LE feeding the node in rootNodes
				tempComponentY = paths[tempNode.path][tempNode.node - 1].y;
				tempComponentZ = paths[tempNode.path][tempNode.node - 1].z;
				//// trial studd
				for (kk = 0; kk < fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes.size(); kk++) // check the presence of the special reconvergent fanout, if it exists then delete the (less critical) path causing this
				{
					if (i == fpgaLogic[tempComponentX][tempComponentY][tempComponentZ].nodes[kk].path) // this means the fanout is present so we must delete all paths between tempNode.node and tempNode.node - 1 
					{
						//paths[tempNode.path][0].deleted = true;
						delete_path(tempNode.path);
						std::cout << "deleted : " << tempNode.path << std::endl;
						deletedPaths++;
					}

				}


			}
			rootNodes.clear();
		//	std::fill(inputs.begin(), inputs.end(), false);
		}
	}
	//// add edges between cascaded paths


	std::cout << "NUmber of Deleted Paths from delete function :  " << deletedPaths << std::endl;

}


void assign_test_phases_ib()
{
	std::vector < std::vector <int> > pathRelationGraph;
	// create the graph representing the connections between paths
	delete_especial_reconvergent_fanout();
	generate_pathRelationGraph(pathRelationGraph);

	// start assigning phases
	unsigned int i, j;
	int testingPhases = 1;
	std::vector <int> possPhases(testingPhases, 1);
	int minPhase;
	bool newColor;
	for (i = 1; i < pathRelationGraph.size(); i++) // traverse across the graph based on the node criitcality and assign test phases
	{
	/*	minPhase = -2;
		for (j = 0; j < pathRelationGraph[i].size(); j++)
		{
			if (paths[pathRelationGraph[i][j]][0].testPhase < minPhase || minPhase == -2)
			{
				minPhase = paths[pathRelationGraph[i][j]][0].testPhase;
			}
		}
		paths[i][0].testPhase = minPhase+*/
		if (paths[i][0].deleted)
			continue;

		for (j = 0; j < pathRelationGraph[i].size(); j++)
		{
			if (paths[pathRelationGraph[i][j]][0].testPhase>-1) // if color is defined
				possPhases[paths[pathRelationGraph[i][j]][0].testPhase] = 0; // set this as a not allowed color in the possible color list
		}
		newColor = true;
		for (j = 0; j < possPhases.size(); j++)
		{
			if (possPhases[j]==1 && newColor)
			{
				newColor = false;
				paths[i][0].testPhase = j;
			}
			possPhases[j] = 1; // restore colors for next iteration
		}
		if (newColor) // must introduce a new color
		{
			paths[i][0].testPhase = possPhases.size();
			possPhases.push_back(1);

		}
	}

	numberOfTestPhases = possPhases.size();
}
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
		for (j = 0; j < paths[i].size(); j++)
		{
			if (fpgaLogic[paths[i][j].x][paths[i][j].y][paths[i][j].z].get_utilization()>1) // delete all paths that uses this component other than path i
			{
			//	fpgaLogic[paths[i][j].x][paths[i][j].y][paths[i][j].z].remove_overlap(i, deletedPaths);
				fpgaLogic[paths[i][j].x][paths[i][j].y][paths[i][j].z].remove_overlap_with_fanin(i, paths[i][j].portIn, deletedPaths);

			}

		}
	}

	for (i = 0; i < deletedPaths.size(); i++)
	{
		paths[deletedPaths[i]][0].deleted = true;
	}
}


void count_cascaded_paths()
{

	int total = 0;
	int i,j,k;
	for (i = 1; i < paths.size(); i++) // loop over all paths
	{
		for (j = 0; j < fpgaLogic[paths[i][0].x][paths[i][0].y][paths[i][0].z].nodes.size(); j++)
		{
			if (fpgaLogic[paths[i][0].x][paths[i][0].y][paths[i][0].z].nodes[j].node != 0)
			{
				std::cout << "7assal " << std::endl;
				total++;
			}
		}
	}
	std::cout << total << std::endl;
}


void check_LE_outputs()
{

	int total = 0;
	int totalIn = 0;
	int all = 0;
	int i, j, k;
	for (i = 0; i < 114; i++)
	{
		for (j = 0; j < 72; j++)
		{
			for (k = 0; k < 32; k++)
			{
				if (fpgaLogic[i][j][k].usedOutputPorts == 2)
				{
					total++;
					all++;
				}
				else if (fpgaLogic[i][j][k].usedOutputPorts == 1)
				{
					all++;
				}
				if (fpgaLogic[i][j][k].usedInputPorts>2)
					totalIn++;
			}
		}
	}
	std::cout << total << std::endl;

}

void check_critical_path_from_Input_toCout() // checks how many LUTS have a critical connection between an input different to Cin and Cout
{
	int total = 0;
	int i, j, k;
	for (i = 1; i < paths.size(); i++) // loop over all paths
	{
		for (j = 0; j < paths[i].size(); j++)
		{
			if (paths[i][j].portIn!=Cin&&paths[i][j].portOut==Cout)
			{
				if (fpgaLogic[paths[i][j].x][paths[i][j].y][paths[i][j].z].usedInputPorts>1)
				{

		//			std::cout << i << " " << j << std::endl;
					total++;
				}
			}
		}
	}

	total = 0;
	int total2 = 0;
	int kk;
	for (i = 0; i < 114; i++)
	{
		for (j = 0; j < 72; j++)
		{
			for (k = 0; k < 32; k++)
			{
				if (fpgaLogic[i][j][k].outputPorts[Cout - 5])// cout is used 
				{
					if (fpgaLogic[i][j][k].usedInputPorts>1) // more than one fanin
					{
						if (!fpgaLogic[i][j][k].inputPorts[Cin]) // no cin
						{
				//			std::cout << "sava3" << std::endl;
							total++;
							for (kk = 0; kk < fpgaLogic[i][j][k].nodes.size(); kk++)
							{
								if (fpgaLogic[paths[fpgaLogic[i][j][k].nodes[kk].path][fpgaLogic[i][j][k].nodes[kk].node + 1].x][paths[fpgaLogic[i][j][k].nodes[kk].path][fpgaLogic[i][j][k].nodes[kk].node + 1].y][paths[fpgaLogic[i][j][k].nodes[kk].path][fpgaLogic[i][j][k].nodes[kk].node + 1].z].usedInputPorts>1)
								{
									total2++;
								}
							}
						}
					}
				}
			}
		}
	}
	std::cout << total << std::endl;
}


void create_location_contraint_file()
{
	int i, j, k;
	int x;
	int total = 0;
	std::ofstream LoFile;
	LoFile.open("LocationFile.txt");
	int path = -1;
	int node = -1;
	for (i = 0; i < FPGAsizeX; i++)
	{
		for (j = 0; j < FPGAsizeY; j++)
		{
			for (k = 0; k < FPGAsizeZ; k++)
			{
				if (fpgaLogic[i][j][k].utilization>0) // this LE is used, in this case assign that location to the most critical path using it.
				{
					total++;
					if (fpgaLogic[i][j][k].usedOutputPorts < 1 && k%2!=1)
						std::cout << "error" << std::endl;

					for (x = 0; x < fpgaLogic[i][j][k].nodes.size(); x++) // find the undeleted most critical path using this node
					{
						if (!paths[fpgaLogic[i][j][k].nodes[x].path][0].deleted)
						{
							path = fpgaLogic[i][j][k].nodes[x].path;
							node = fpgaLogic[i][j][k].nodes[x].node;
					//		LoFile << fpgaLogic[i][j][k].nodes[x].path << "NODE" << fpgaLogic[i][j][k].nodes[x].node << "_t" << std::endl;
							break;
						}
					}

					if (path == -1) // if no deleted path was found then continue 
						continue;

					if (k % 2 == 0)
					{
						LoFile << "set_location_assignment LCCOMB_X" << i << "_Y" << j << "_N" << k << " -to PATH" << path << "NODE" << node << "_t" << std::endl;
					}
					else
					{
						LoFile << "set_location_assignment FF_X" << i << "_Y" << j << "_N" << k << " -to PATH" << path << "NODE" << node << "_t" << std::endl;
					}
					
				}
				path = -1;
				node = -1;
			}
		}
	}

	LoFile.close();
}

bool get_feeder(int x, int y, int z, int & feederPath, int & feederNode) // return path and node that feeds the register at x, y and z
{
	if (z % 2 != 1) // ensure that the given cell is a register
		return false;

	if (fpgaLogic[x][y][z].nodes.size() < 1) // make sure that this element is actually used
		return false;

	if (fpgaLogic[x][y][z].nodes[0].node < 1) // make sure that this element is not a source, i.e something feeds it
		return false;

	int feederX, feederY, feederZ;

	/// gets the element that feeds the given element through the first path using this node. Even if this path is deleted the feeder x, y and z would be the same, it is imposisble for two different cells to feed the same cell
	feederX = paths[fpgaLogic[x][y][z].nodes[0].path][fpgaLogic[x][y][z].nodes[0].node - 1].x;
	feederY = paths[fpgaLogic[x][y][z].nodes[0].path][fpgaLogic[x][y][z].nodes[0].node - 1].y;
	feederZ = paths[fpgaLogic[x][y][z].nodes[0].path][fpgaLogic[x][y][z].nodes[0].node - 1].z;
	feederPath = -1;
	feederNode = -1;
	int i;

	for (i = 0; i < fpgaLogic[feederX][feederY][feederZ].nodes.size(); i++) // loop through paths using this feeder the first nondeletred one is the name that should be used
	{
		if (!paths[fpgaLogic[feederX][feederY][feederZ].nodes[i].path][0].deleted)
		{
			feederPath = fpgaLogic[feederX][feederY][feederZ].nodes[i].path;
			feederNode = fpgaLogic[feederX][feederY][feederZ].nodes[i].node;
			return true;
		}
	}
	return false;
}

bool get_feeder(int x, int y, int z, int portIn, int & feederPath, int & feederNode) // returns the path and node that feeds element x,y,z through portIn
{
	int i;
	if (z % 2 != 0) // ensure that the given cell is a LUT
		return false;

	if (fpgaLogic[x][y][z].nodes.size() < 1) // make sure that this element is actually used
		return false;



	if (!fpgaLogic[x][y][z].inputPorts[portIn]) // ensures thatthis ports is actually used
		return false;
	int possibleFeedingPath = -1;
	int possibleFeedingNode = -1;
	for (i = 0; i < fpgaLogic[x][y][z].nodes.size(); i++) // loop through paths using the questioned element, the first non-deleted path feeding the desired input port is used to get the feeder.
	{
		if (paths[fpgaLogic[x][y][z].nodes[i].path][fpgaLogic[x][y][z].nodes[i].node].portIn == portIn && !paths[fpgaLogic[x][y][z].nodes[i].path][0].deleted)
		{
			possibleFeedingPath = fpgaLogic[x][y][z].nodes[i].path;
			possibleFeedingNode = fpgaLogic[x][y][z].nodes[i].node;
			break;
		}
	}
	if (possibleFeedingNode == -1 || possibleFeedingPath == -1)
		return false;

	int feederX, feederY, feederZ;
	feederX = paths[possibleFeedingPath][possibleFeedingNode-1].x;
	feederY = paths[possibleFeedingPath][possibleFeedingNode - 1].y;
	feederZ = paths[possibleFeedingPath][possibleFeedingNode - 1].z;

	for (i = 0; i < fpgaLogic[feederX][feederY][feederZ].nodes.size(); i++) // loop through paths using this feeder the first nondeletred one is the name that should be used
	{
		if (!paths[fpgaLogic[feederX][feederY][feederZ].nodes[i].path][0].deleted)
		{
			feederPath = fpgaLogic[feederX][feederY][feederZ].nodes[i].path;
			feederNode = fpgaLogic[feederX][feederY][feederZ].nodes[i].node;
			return true;
		}
	}
	return false;
}
void create_WYSIWYGs_file()
{
	int i, j, k;
	int x;
	int total = 0;
	bool deleted = true;
	std::ofstream verilogFile;
	verilogFile.open("VerilogFile.txt");
	int path = -1;
	int node = -1;
	std::vector <Path_logic_component> sinks; // stores the output signals of the tested paths;
	std::vector <Path_logic_component> controlSignals; // stores the control signals of the tested paths;
	int pathFeeder, nodeFeeder;
	int pathFeederPort1, pathFeederPort2, nodeFeederPort1, nodeFeederPort2;
	int port1, port2;
	for (i = 0; i < FPGAsizeX; i++)
	{
		for (j = 0; j < FPGAsizeY; j++)
		{
			for (k = 0; k < FPGAsizeZ; k++)
			{
				port1 = -1;
				port2 = -1;
				pathFeederPort1 = -1;
				pathFeederPort2 = -1;
				nodeFeederPort1 = -1;
				nodeFeederPort2 = -2;

				for (x = 0; x < fpgaLogic[i][j][k].nodes.size(); x++) // make sure that this node is not deleted and gets the name of the wysiwyg, or we can check utilization
				{
					if (!paths[fpgaLogic[i][j][k].nodes[x].path][0].deleted)
					{
						path = fpgaLogic[i][j][k].nodes[x].path;
						node = fpgaLogic[i][j][k].nodes[x].node;
						break;
					}
				}
				
				if (path == -1 || node == -1) // all paths using this node are deleted so do not instantiate a wysiwygs for this node
					continue;

				if (k % 2 == 0) // LUT
				{
					
					if (fpgaLogic[i][j][k].outputPorts[Combout - 5] && !fpgaLogic[i][j][k].outputPorts[Cout - 5]) // output is only from Combout
					{
						
						if (fpgaLogic[i][j][k].usedInputPorts < 3) // check that maximum 2 inputs are being used
						{
							// get the two ports used
							for (x = 0; x < InputPortSize; x++)
							{
								if (fpgaLogic[i][j][k].inputPorts[x] && port1 < 0) // 1st port not yet set
								{
									port1 = x;
								}
								else if (fpgaLogic[i][j][k].inputPorts[x] ) // 1st port ws set, so set the second port
								{
									port2 = x;
								}

							}
							// at least one port must be used
							assert(port1 > -1);
							if (fpgaLogic[i][j][k].usedInputPorts == 2)
								assert(port2>-1);
							if (fpgaLogic[i][j][k].inputPorts[Cin]) // Cin is used to connect to combout
							{
								assert(get_feeder(i, j, k, port1, pathFeederPort1, nodeFeederPort1));
								if (port2>-1) // second input port is used
									assert(get_feeder(i, j, k, port2, pathFeederPort2, nodeFeederPort2));
								controlSignals.push_back(Path_logic_component(path, node)); //stores all the nodes requiring control signals
								verilogFile << "cycloneive_lcell_comb PATH" << path << "NODE" << node << "_t (" << std::endl;
								verilogFile << "	.dataa(PATH" << path << "NODE" << node << "Con)," << std::endl;
								if (port1 == Cin) // port 1 is Cin
								{
									if (port2 > -1) // no port 2, set port b to gnd
									{
										verilogFile << "	.datab(gnd)," << std::endl;
										verilogFile << "	.cin(PATH" << pathFeederPort1 << "NODE" << nodeFeederPort1 << ")," << std::endl;
										verilogFile << "	.datad(PATH" << path << "NODE" << node << "F)," << std::endl;
									}
									else // port 2 is also used 
									{
										verilogFile << "	.datab(PATH" << pathFeederPort2 << "NODE" << nodeFeederPort2 << ")," << std::endl;
										verilogFile << "	.cin(PATH" << pathFeederPort1 << "NODE" << nodeFeederPort1 << ")," << std::endl;
										verilogFile << "	.datad(PATH" << path << "NODE" << node << "F)," << std::endl;
									}
								}
								else // port 2 is  cin and port 1 must be used
								{
									verilogFile << "	.datab(PATH" << pathFeederPort1 << "NODE" << nodeFeederPort1 << ")," << std::endl;
									verilogFile << "	.cin(PATH" << pathFeederPort2 << "NODE" << nodeFeederPort2 << ")," << std::endl;
									verilogFile << "	.datad(PATH" << path << "NODE" << node << "F)," << std::endl;
								}
								verilogFile << "defparam PATH" << path << "NODE" << node << "_t .sumlutc_input = \"cin\";" << std::endl;
								verilogFile << "defparam PATH" << path << "NODE" << node << "_t .lut_mask = \"D77D\";" << std::endl;

							}
							else // normal inputs to combout // common case
							{

								assert(get_feeder(i, j, k, port1, pathFeederPort1, nodeFeederPort1));
								if (port2>-1) // second input port is used
									assert(get_feeder(i, j, k, port2, pathFeederPort2, nodeFeederPort2));

								controlSignals.push_back(Path_logic_component(path, node)); //stores all the nodes requiring control signals
								//// instantiate the wysiwyg
								verilogFile << "cycloneive_lcell_comb PATH" << path << "NODE" << node << "_t (" << std::endl;
								verilogFile << "	.dataa(PATH" << path << "NODE" << node << "Con),"  << std::endl;
								verilogFile << "	.datab(PATH" << pathFeederPort1 << "NODE" << nodeFeederPort1 << ")," << std::endl;
								if (port2>-1)
									verilogFile << "	.datac(PATH" << pathFeederPort2 << "NODE" << nodeFeederPort2 << ")," << std::endl;
								else // only one port is used
									verilogFile << "	.datac(gnd)," << std::endl;

								verilogFile << "	.datad(PATH" << path << "NODE" << node << "F)," << std::endl;
								verilogFile << "	.combout(PATH" << path << "NODE" << node << std::endl;
								verilogFile << "defparam PATH" << path << "NODE" << node << "_t .sumlutc_input = \"datac\";" << std::endl;
								verilogFile << "defparam PATH" << path << "NODE" << node << "_t .lut_mask = \"D77D\";" << std::endl;
							}
						}
						else
						{
							std::cout << "ERROR: A LUT which uses " << fpgaLogic[i][j][k].usedInputPorts << " is not supported. SomethingWrong.";
						}
					}
					else // output can be cout or (cout,combout)
					{
						if (fpgaLogic[i][j][k].usedInputPorts < 3) // max two ports are used
						{
							// get the two ports used
							for (x = 0; x < InputPortSize; x++)
							{
								if (fpgaLogic[i][j][k].inputPorts[x] && port1 < 0) // 1st port not yet set
								{
									port1 = x;
								}
								else if (fpgaLogic[i][j][k].inputPorts[x]) // 1st port ws set, so set the second port
								{
									port2 = x;
								}

							}

						}
						else
						{
							std::cout << "ERROR: A LUT (Cout) which uses " << fpgaLogic[i][j][k].usedInputPorts << " is not supported. SomethingWrong.";
						}

					}
					
				}
				else // FLIPFLOP
				{
					verilogFile << "dffeas PATH" << path << "NODE" << node << "_t (" << std::endl;
					verilogFile << "	.clk(CLK)," << std::endl;
					if (node == 0) // this is a source register, assuming no cascaded paths, no register is a source and a sink
					{
						verilogFile << "	.d(xin)," << std::endl; // assuming that all sources will share the same input
						verilogFile << "	.q(PATH" << path << "NODE" << node << "));" << std::endl;
					}
					else // this is a sink register
					{
						assert(get_feeder(i, j, k, pathFeeder, nodeFeeder));
						verilogFile << "	.d(PATH" << pathFeeder << "NODE" << nodeFeeder << ")," <<   std::endl; // assuming that all sources will share the same input
						verilogFile << "	.q(PATH" << path << "NODE" << node << "));" << std::endl;
						sinks.push_back(Path_logic_component(path, node));

					}
					verilogFile << "defparam PATH" << path << "NODE" << node << "_t .power_up = \"low\";" << std::endl;
					verilogFile << "defparam PATH" << path << "NODE" << node << "_t .is_wysiwyg = \"true\";" << std::endl;
				}
				path = -1;
				node = -1;
			}
		}
	}

	verilogFile.close();

}

void delete_path(int path)
{
	if (paths[path][0].deleted) // already deleted this path
		return;
	// mark the path as deleted
	paths[path][0].deleted = true;
	int i,x,y,z,j;
	bool portInStillExists = false;
	bool portOutStillExists = false;
	/// loop across all nodes of this path and adjust them accordingly
	for (i = 0; i < (int)paths[path].size(); i++)
	{
		x = paths[path][i].x;
		y = paths[path][i].y;
		z = paths[path][i].z;
		// reduce this node utilization
		fpgaLogic[x][y][z].utilization--;
		// todo: remove this from the nodes of fpgalogic[x][y][z].nodes
		portInStillExists = false;
		portOutStillExists = false;
		for (j = 0; j < fpgaLogic[x][y][z].nodes.size(); j++)
		{
			if (fpgaLogic[x][y][z].nodes[j].path == path) // same path, so continue to next path
				continue;
			if (paths[fpgaLogic[x][y][z].nodes[j].path][0].deleted) // if this path is deleted then continue to next path
				continue;

			if (paths[fpgaLogic[x][y][z].nodes[j].path][fpgaLogic[x][y][z].nodes[j].node].portIn == paths[path][i].portIn) // another path is using the same element using the same portIn
				portInStillExists = true;

			if (paths[fpgaLogic[x][y][z].nodes[j].path][fpgaLogic[x][y][z].nodes[j].node].portOut == paths[path][i].portOut) // another path is using the same element using the same portOut
				portOutStillExists = true;

		}

		if (!portInStillExists) // input port must be deleted
		{
			fpgaLogic[x][y][z].inputPorts[paths[path][i].portIn] = false;
			fpgaLogic[x][y][z].usedInputPorts--;
		}

		if (!portOutStillExists) //output port must be deleted
		{
			fpgaLogic[x][y][z].outputPorts[paths[path][i].portOut-5] = false;
			fpgaLogic[x][y][z].usedOutputPorts--;
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
	bool red;
	bool invertingSignal;
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
		// inverting behanviour
		if (!std::getline(metaData, line))
		{
			std::cout << "Incorrect file format. Terminating...." << std::endl;
			return 0;
		}
		if (line[0] == 'I')
		{
			invertingSignal = true;
		}
		else
		{
			invertingSignal = false;
		}
		// have x and y and z stored
		fpgaLogic[x][y][z].add_node(path, node, pIn, pOut);
		tempPath.push_back(Path_node(x, y, z,pIn,pOut, invertingSignal));
		for (i = 0; i<fpgaLogic[x][y][z].nodes.size(); i++) // pass through all path nodes that uses this logic element
		{
			if (fpgaLogic[x][y][z].nodes[i].path != path) // check if it is used by another path through the same portIn and portOut
			{
				if (pIn == paths[fpgaLogic[x][y][z].nodes[i].path][fpgaLogic[x][y][z].nodes[i].node].portIn&&pOut == paths[fpgaLogic[x][y][z].nodes[i].path][fpgaLogic[x][y][z].nodes[i].node].portOut) // check if the other path node uses the same input pin and output pin, which means its redundant, we only check for the first match as this is the most critical path using this node and is considered the master path usign this node
				{
					tempPath[tempPath.size() - 1].redun = true;
					tempPath[tempPath.size() - 1].eqPath = fpgaLogic[x][y][z].nodes[i].path;
					tempPath[tempPath.size() - 1].eqNode = fpgaLogic[x][y][z].nodes[i].node;
					break;
				}

			}
		}

		if (node!=0) // skip source registers
			if (fpgaLogic[x][y][z].get_utilization()>maxOverlap)
				maxOverlap = fpgaLogic[x][y][z].get_utilization();
		node++;
	}
	// push last path to list of paths
	paths.push_back(tempPath);
	tempPath.clear();

	///// finished parsing
	count_cascaded_paths();
	check_LE_outputs();
	check_critical_path_from_Input_toCout();
	std::vector <bool> phase1( std::vector<bool>(paths.size(), true));
	testingPhases.push_back(phase1);
	numberOfTestPhases = 1;
	assign_test_phases_ib();
	std::vector <std::vector<int> > test_structure;
	test_structure.resize(numberOfTestPhases);
	create_location_contraint_file();
	create_WYSIWYGs_file();
	for (i = 1; i < paths.size(); i++)
	{
		if (!paths[i][0].deleted)
			test_structure[paths[i][0].testPhase].push_back(i);
	}
	unsigned int j;
	std::cout << " test phases look like : " << std::endl;
	for (i = 0; i < test_structure.size(); i++)
	{
		std::cout << "Phase " << i << " : ";
		for (j = 0; j < test_structure[i].size(); j++)
		{
			std::cout << test_structure[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::vector<int> deletedPaths;

	delete_paths(deletedPaths);
	std::sort(deletedPaths.begin(), deletedPaths.end());
	std::cout << deletedPaths.size() << std::endl;
	return 0;
}