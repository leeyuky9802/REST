#pragma once
#include <stdint.h>
//#include <tchar.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <list>
#include <vector>
//#include <SDKDDKVer.h>
#include <algorithm>

#include <queue>

#include <sstream>

using namespace std;

struct CmpByValue {
	bool operator()(const std::pair<int, double>& lhs, const std::pair<int, double>& rhs) {
		return lhs.second < rhs.second;
	}
};

struct G_Node    //save the info for operation node
{
	int id; //node ID
	int type; //node Function-type
	list<G_Node*> child;   // successor nodes (distance = 1)
	list<G_Node*> parent;  // predecessor nodes (distance = 1)
	int asap, alap, tasap, talap;	// T-asap, T-alap (in # of clock cycle), Temp-Tasap, Temp-Talap;
	bool schl, tschl;	//bool, = 1 scheduled; =0 not

	int prev_asap, prev_alap;

	int fuid;
};

//Local-PG part:

//input S&B solution storage structure

/*
	What S&B solution is easiler to access:

	using 2-D map:

	1-D: FUs
	2-D: Schedulings
*/

struct S
{
	std::map<int, int> schedule; //<operation, sch'ed cc>
	std::map<int, int> bind; //<operation, FU>
	std::map<int, int> fu_info; //FU-id, FU-type(int)>
};

struct Lib
{
	std::map<string, int> fu_type; //<str-name, type-id>
	std::map<int, int> delay; //<type-id, delay>
	std::map<int, double> dp; //<type-id, dp>
	std::map<int, double> lp; //<type-id, lp>
};




//------------------INPUT FUNCTION-----------------//
void get_Lib_structure(string& filename, std::map<int, int>& delay);
void Read_DFG(int& DFG, string& filename, string& dfg_name); //Read-DFG filename
void readGraphInfo(string& filename, int& DFG, int* edge_num); //Read-DFG info: node type, node's predecessors/successors

void get_S_structure(string& filename, S& s, int& LC);