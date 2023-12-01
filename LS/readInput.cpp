#include "LS.h"
#define _CRT_SECURE_NO_WARNINGS

using namespace std;

/*
* 
* GET_LIB: read library: only have 2 types: Add/Mul
* 0: Add
* 1: Mul
* 
*/

void get_Lib_structure(string& filename, std::map<int, int>& delay)
{
	std::istringstream iss;
	std::ifstream ifs(filename);
	std::string line;

	std::map<string, int> libDelay;
	libDelay.clear();

	while (std::getline(ifs, line))
	{
		std::istringstream iss(line);
		string fu_name;
		int delay;

		if (!(iss >> fu_name >> delay))
			break;
		else
			libDelay[fu_name] = delay;
	}

	delay.clear();
	for (auto it = libDelay.begin(); it != libDelay.end(); it++) {
		if (it->first == "ADD")
			delay[0] = it->second;
		else if (it->first == "MUL")
			delay[1] = it->second;
	}
}

//DO NOT CHANGE FUNCTIONS BELOW//
/*
*
* Read_DFG: Read input DFG in the "DFG" folder, in the same path with other source files.
* 
* ReadGraphInfo: store DFG nodes/parent/child (arcs) into "G_Node" map
*/

void Read_DFG(int& DFG, string& filename, string& dfg_name)
{
	if (DFG == 0)
		filename = "DFG//example.txt";	//this DFG is not provided in the input DFG files, used for your customized DFG only
	else if (DFG == 1)
		filename = "DFG//hal.txt";
	else if (DFG == 2)
		filename = "DFG//horner_bezier_surf_dfg__12.txt";
	else if (DFG == 3)
		filename = "DFG//arf.txt";
	else if (DFG == 4)
		filename = "DFG//motion_vectors_dfg__7.txt";
	else if (DFG == 5)
		filename = "DFG//ewf.txt";
	else if (DFG == 6)
		filename = "DFG//feedback_points_dfg__7.txt";
	else if (DFG == 7)
		filename = "DFG//write_bmp_header_dfg__7.txt";
	else if (DFG == 8)
		filename = "DFG//interpolate_aux_dfg__12.txt";
	else if (DFG == 9)
		filename = "DFG//matmul_dfg__3.txt";
	else if (DFG == 10)
		filename = "DFG//smooth_color_z_triangle_dfg__31.txt";
	else if (DFG == 11)
		filename = "DFG//invert_matrix_general_dfg__3.txt";
	else if (DFG == 12)
		filename = "DFG//300_0.txt";
	else if (DFG == 13)
		filename = "DFG//1300_0.txt";

	dfg_name = filename.substr(5);
}
void readGraphInfo(string& filename, int& DFG, int& edge_num, int& opn, std::map<int, G_Node>& ops)
{
	FILE* bench;		//the input DFG file

	char* char_arr = &filename[0];

	if (!(bench = fopen(char_arr, "r")))	//open the input DFG file to count the number of operation nodes in the input DFG
	{
		std::cerr << "Error: Reading input DFG file " << char_arr << " failed." << endl;
		cin.get();	//waiting for user to press enter to terminate the program, so that the text can be read
		exit(EXIT_FAILURE);
	}
	char* line = new char[100];
	opn = 0;			//initialize the number of operation node
	if (DFG != 12 && DFG != 13)	//read input DFG file in format .txt
	{
		while (fgets(line, 100, bench))		//count the number of operation nodes in the input DFG
			if (strstr(line, "label") != NULL)	//if the line contains the keyword "label", the equation returns true, otherwise it returns false. search for c/c++ function "strstr" for detail
				opn++;
	}
	else //read input DFG file from format .gdl
	{
		while (fgets(line, 100, bench))
			if (strstr(line, "node") != NULL)
				opn++;
	}
	fclose(bench);	//close the input DFG file

	for (auto i = 0; i < opn; i++)
	{
		G_Node curr;
		ops[i] = curr;
	}


	//close the input DFG file
	//based on the number of operation node in the DFG, dynamically set the size
	std::map<string, int> oplist;
	string name, cname;
	char* tok, * label;	//label: the pointer point to "label" or "->" in each line
	char seps[] = " \t\b\n:";		//used with strtok() to extract DFG info from the input DFG file
	int node_id = 0;	//count the number of edges in the input DFG file
	if (!(bench = fopen(char_arr, "r")))	//open the input DFG file agian to read the DFG, so that the cursor returns back to the beginning of the file
	{
		std::cerr << "Error: Failed to load the input DFG file." << endl;
		cin.get();	//waiting for user to press enter to terminate the program, so that the text can be read
		exit(EXIT_FAILURE);
	}
	if (DFG != 12 && DFG != 13)
		while (fgets(line, 100, bench))	//read a line from the DFG file, store it into "line[100]
		{
			if ((label = strstr(line, "label")) != NULL)	//if a keyword "label" is incurred, that means a operation node is found in the input DFG file
			{
				tok = strtok(line, seps);	//break up the line by using the tokens in "seps". search the c/c++ function "strtok" for detail
				name.assign(tok);	//obtain the node name
				oplist.insert(make_pair(name, node_id));	//match the name of the node to its number flag
				tok = strtok(label + 7, seps);	//obtain the name of the operation type
				if (strcmp(tok, "ADD") == 0)	  ops[node_id].type = 0;			//match the operation type to the nod. search for c/c++ function "strcmp" for detail
				else if (strcmp(tok, "AND") == 0) ops[node_id].type = 0;
				else if (strcmp(tok, "MUL") == 0) ops[node_id].type = 1;
				else if (strcmp(tok, "ASR") == 0) ops[node_id].type = 0;
				else if (strcmp(tok, "LSR") == 0) ops[node_id].type = 0;
				else if (strcmp(tok, "LOD") == 0) ops[node_id].type = 0;
				else if (strcmp(tok, "STR") == 0) ops[node_id].type = 0;
				else if (strcmp(tok, "SUB") == 0) ops[node_id].type = 0;
				else if (strcmp(tok, "DIV") == 0) ops[node_id].type = 1;
				ops[node_id].id = node_id;
				node_id++;
			}
			else if ((label = strstr(line, "->")) != NULL)	//if a keyword "->" is incurred, that means an edge is found in the input DFG file
			{
				tok = strtok(line, seps);	//break up the line by using the tokens in "seps". search the c/c++ function "strtok" for detail
				name.assign(tok);	//obtain node name u from edge (u, v)
				cname.assign(strtok(label + 3, seps));	////obtain node name v from edge (u, v)
				(ops[oplist[name]].child).push_back(&(ops[oplist[cname]]));	//use double linked list to hold the children
				(ops[oplist[cname]].parent).push_back(&(ops[oplist[name]]));//use double linked list to hold the parents
				edge_num++;
			}
		}
	
	else
	{
		int source, target;
		while (fgets(line, 100, bench))
		{
			if (strstr(line, "node"))
			{
				fgets(line, 100, bench);
				tok = strtok(line, seps);
				tok = strtok(NULL, seps);
				if (atoi(tok) < 2)
					ops[node_id].type = 0;	//since there are only 4 operation types in .gdl file, we change type 2 to division and keep the other types unchanged.
				else if (atoi(tok) > 1)
					ops[node_id].type = 1;
				ops[node_id].id = node_id;
				node_id++;
			}
			else if (strstr(line, "edge"))
			{
				tok = strtok(line, seps);
				for (int i = 0; i < 4; i++)
					tok = strtok(NULL, seps);
				source = atoi(tok);
				fgets(line, 100, bench);
				tok = strtok(line, seps);
				for (int i = 0; i < 4; i++)
					tok = strtok(NULL, seps);
				target = atoi(tok);

				ops[source].child.push_back(&ops[target]);
				ops[target].parent.push_back(&ops[source]);
				edge_num++;
			}
		}
	}//end of reading DFG
	
	fclose(bench);
	delete[] line;
}