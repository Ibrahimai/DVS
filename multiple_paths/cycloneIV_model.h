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

class Logic_element
{
	int overLap;
	std::vector<Path_logic_component> nodes;
public:
	Logic_element();
	Logic_element(int over);
	int get_overLap();
	void set_overLap(int x);
	void add_node(int p, int n);

};

Logic_element::Logic_element()
{
	nodes.resize(0);
	overLap = 0;
}

Logic_element::Logic_element(int over)
{
	nodes.resize(0);
	overLap = over;
}
int Logic_element::get_overLap()
{
	return overLap;
}
void Logic_element::set_overLap(int x)
{
	overLap = x;
}

void Logic_element::add_node(int p, int n)
{
	Path_logic_component temp (p, n);
	nodes.push_back(temp);
	overLap++;
}

Logic_element fpgaLogic[114][72][32];