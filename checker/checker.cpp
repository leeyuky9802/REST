//The original code is created by Huan Ren. Last modified by Owen and Keivn.
//"lookahead" approach was not discussed in the class but is used in this code, for forces calculation. please refer to the FDS paper section IV: E
#include "checker.h"
#include <climits>
#include <bitset>
#include <queue>

using namespace std;

const int tnum = 2;	//number of operation type

int DFG = 0; //DFG ID, this is automatically run from range 1 to 11
int LC = 0; //global parameter latency constraint. LC = latency_parameter * ASAP latency.
string DFGname; //filename (DFG name) for input/output
string outputfile; //output filename

int opn = 0; //# of operations in current DFG
//G_Node* ops; //operations list

std::map<int, G_Node> ops;

std::map<int, int> delay;

ofstream output_sb_results;

int main(int argc, char** argv)
{
	//string swt_nf = argv[1];
	//string swt_f = argv[2];

	int edge_num = 0;

	//get delay structure;
	string filename = "lib.txt";
	Lib lib;

	delay.clear();
	get_Lib_structure(filename, delay);

	for (DFG = 1; DFG <= 13; DFG++)
	{
		std::cout << "DFG = " << DFG << endl;

		ops.clear();

		//std::cout << "The current DFG is #" << DFG << endl;
		string filename, dfg_name;
		Read_DFG(DFG, filename, dfg_name); //read DFG filename
		readGraphInfo(filename, DFG, &edge_num); //read DFG info

		stringstream str(dfg_name);
		string tok;
		while (getline(str, tok, '.'))
		{
			if (tok != "txt")
				DFGname = tok;
			//cout << tok << endl;
		}
		string output_sb_res = DFGname + "_final_sb_res.txt";


		//read s&b results:

		S sb_res;

		//std::cout << output_sb_res << endl;

		get_S_structure(output_sb_res, sb_res, LC);

	//	for (auto i = 0; i < opn; i++) 
	//		std::cout << "op = " << i << " schl cc = " << sb_res.schedule[i] << " fu id = " << sb_res.bind[i] << endl;

		
		//check dependency errors:

		//for each op, for each pr: if pr.schl + d - 1 < u.schl this is correct; for each su: if u.schl + d - 1 < su.schl, correct.

		int dependency_error = 0;
		std::map<int, int> error_pair;
		error_pair.clear();

		for (auto u = 0; u < opn; u++) {

			int u_type = ops[u].type;
			int u_delay = delay[u_type];
			int u_start_time = sb_res.schedule[u];
			int u_end_time = u_start_time + u_delay - 1;

			if (ops[u].parent.size() > 0) {
				for (auto prsu = ops[u].parent.begin(); prsu != ops[u].parent.end(); prsu++) {

					int prsu_type = (*prsu)->type;
					int prsu_delay = delay[prsu_type];
					int prsu_start_time = sb_res.schedule[(*prsu)->id];
					int prsu_end_time = prsu_start_time + prsu_delay - 1;

					if (u_start_time > prsu_end_time) {

					}
					else {
						//cout << "error: u = " << u << " time = " << u_time << " pr = " << (*prsu)->id << " pr_time = " << prsu_time << " pr_delay = " << prsu_delay << endl;

						if (u < (*prsu)->id)
							error_pair[u] = (*prsu)->id;
						else
							error_pair[(*prsu)->id] = u;
					}
				}
			}
		
			if (ops[u].child.size() > 0) {
				for (auto prsu = ops[u].child.begin(); prsu != ops[u].child.end(); prsu++) {

					int prsu_type = (*prsu)->type;
					int prsu_delay = delay[prsu_type];
					int prsu_start_time = sb_res.schedule[(*prsu)->id];
					int prsu_end_time = prsu_start_time + prsu_delay - 1;

					if (u_end_time < prsu_start_time) {

					}
					else {

						//	cout << "error: u = " << u << " time = " << u_time << " su = " << (*prsu)->id << " su_time = " << prsu_time << " u_delay = " << u_delay << endl;

						if (u < (*prsu)->id)
							error_pair[u] = (*prsu)->id;
						else
							error_pair[(*prsu)->id] = u;
					}
				}
			}
			
		}

		cout << "LC = " << LC << endl;
		cout << "# of dependency error = " << error_pair.size() << endl;

		//check FU-overlapping:

		std::map<int, int> FUs;
		FUs.clear();

		for (auto u = 0; u < opn; u++)
			FUs[sb_res.bind[u]] = 1;

		std::map<int, std::map<int, int>> FU_bind;
		FU_bind.clear();
		
		for (auto fu = FUs.begin(); fu != FUs.end(); fu++) 
			for (auto cc = 1; cc <= LC; cc++)
				FU_bind[fu->first][cc] = 0;

		for (auto u = 0; u < opn; u++)
		{
			int my_fu = sb_res.bind[u];

			int my_delay = delay[ops[u].type];

			int my_cc = sb_res.schedule[u];

			for (auto c = my_cc; c < my_cc + my_delay; c++)
				FU_bind[my_fu][c]++;
		}


		int fu_overlapped_error = 0;
		for (auto fu = FU_bind.begin(); fu != FU_bind.end(); fu++)
			for (auto cc = fu->second.begin(); cc != fu->second.end(); cc++)
				if (cc->second > 1) {
					cout << "for FU " << fu->first << " cc " << cc->first << " has 2 op executed on it, error exists." << endl;
					fu_overlapped_error++;
				}
		
		cout << "# of FU usage error (mutliple ops executes on the same cc and same FU) = " << fu_overlapped_error << endl;

	}




	std::cout << "Press ENTER to terminate the program." << endl;
	std::cin.get();	//press Enter to terminate the program
    return 0;
}


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
void readGraphInfo(string& filename, int& DFG, int* edge_num)
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
				(*edge_num)++;
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
	}
	
	fclose(bench);
	delete[] line;
}

//----------------//
void get_S_structure(string& filename, S& s, int& LC)
{
	std::istringstream iss;
	std::string lineStr;
	std::ifstream ifs(filename);
	std::string line;

	//cout << "filename = " << filename << endl;

	s.schedule.clear();
	s.bind.clear();
	s.fu_info.clear();

	int counter = 0;

	while (std::getline(ifs, line))
	{

		//1st line is the LC info.
		if (counter == 0)
		{
			std::istringstream iss(line);
			string a;
			int lc;
			if (!(iss >> a >> lc))
			{
				break;
			}
			else {
				LC = lc;
			}
		
			counter++;
			continue;
		}

		//other lines are op S&B info.

		std::istringstream iss(line);
		string type;
		int op_id, fu_id, cc;
		if (!(iss >> op_id >> cc >> fu_id))
		{
			break;
		}
		else
		{
			//obtain op_id, fu_id, cc, fu-type;
			s.schedule[op_id] = cc;
			s.bind[op_id] = fu_id;
		}
	}
}


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