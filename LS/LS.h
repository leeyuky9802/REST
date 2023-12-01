#pragma once

#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <list>
#include <vector>
#include <algorithm>
#include <queue>
#include <sstream>

#define _CRT_SECURE_NO_WARNINGS

using namespace std;

struct G_Node    //save the info for operation node
{
	int id; //node ID
	int type; //node Function-type
	list<G_Node*> child;   // successor nodes (distance = 1)
	list<G_Node*> parent;  // predecessor nodes (distance = 1)
	int asap, alap;
	bool schl;
};

void get_Lib_structure(string& filename, std::map<int, int>& delay);

void Read_DFG(int& DFG, string& filename, string& dfg_name); //Read-DFG filename
void readGraphInfo(string& filename, int& DFG, int& edge_num, int& opn, std::map<int, G_Node>& ops);