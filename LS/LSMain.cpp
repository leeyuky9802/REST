#include "LS.h"

using namespace std;

double latencyParameter = 1.5; //latency constant parameter, change this parameter can affect the latency constraint and hence, the final scheduling result is changed
int DFG = 0; //DFG ID, this is automatically run from range 1 to 11
int LC = 0; //global parameter latency constraint. LC = latency_parameter * ASAP latency.
int opn = 0; //# of operations in current DFG
int edge_num = 0;
//G_Node* ops; //operations list


std::map<int, int> delay;
std::map<int, int> lp;

void LS(std::map<int, int>& schlResult, std::map<int, int>& FUAllocationResult, std::map<int, std::map<int, std::vector<int>>>& bindingResult, int& actualLatency,
	std::map<int, G_Node>& ops, int& latencyConstraint, double& latencyParameter, std::map<int, int>& delay);

ofstream output_sb_result;

int main(int argc, char** argv)
{

	//read "lib"
	//get delay structure: only ADD/MUL are needed. you can change delay of ADD/MUL (# of cc's) in "lib.txt" file.
	string filename = "lib.txt";
	std::map<int, int> delay;
	get_Lib_structure(filename, delay);

	for (DFG = 1; DFG <= 13; DFG++) {

		std::map<int, G_Node> ops;
		LC = 0, opn = 0, edge_num = 0;
		ops.clear();

		string filename, dfg_name;
		Read_DFG(DFG, filename, dfg_name); //read DFG filename
		readGraphInfo(filename, DFG, edge_num, opn, ops); //read DFG info

		std::map<int, int> ops_schl_cc, ops_schl_FU, FU_type;
		ops_schl_cc.clear();
		ops_schl_FU.clear();
		FU_type.clear();

		std::map<int, int> types;

		for (auto i = 0; i < 2; i++)
			types[i] = 0;

		for (auto i = 0; i < opn; i++)
			types[ops[i].type]++;

		std::map<int, int> schlResult;
		schlResult.clear();

		std::map<int, int> FUAllocationResult;
		FUAllocationResult.clear();

		std::map<int, std::map<int, std::vector<int>>> bindingResult;
		bindingResult.clear();

		int actualLatency = 0;
		int latencyConstraint = 0;

		LS(schlResult, FUAllocationResult, bindingResult, actualLatency,
			ops, latencyConstraint, latencyParameter, delay);

		int total_FUs = 0;



		//the following part until the end is the output-file function which is used to generate output-files for checker.

		std::map<int, int> opBindingResult;
		opBindingResult.clear();

		int curr_idx = 0;
		int totalFUs = 0;

		for (auto type = 0; type < FUAllocationResult.size(); type++){
			int numOfFUs = FUAllocationResult[type];
			totalFUs += numOfFUs;
			for (auto fu = 0; fu < numOfFUs; fu++) 
				for (auto op = bindingResult[type][fu].begin(); op != bindingResult[type][fu].end(); op++) 
					opBindingResult[*op] = fu + curr_idx;
			curr_idx = numOfFUs;
		}

		std::cout << "For the curren DFG " << DFG << ", the actual Latency is " << actualLatency << " (latency constraint is " << latencyConstraint << ")," << " # of total FUs used = " << totalFUs << endl;

		//get output s&b result file.
		string DFGname;
		stringstream str(dfg_name);
		string tok;
		while (getline(str, tok, '.')){
			if (tok != "txt")
				DFGname = tok;
			//cout << tok << endl;
		}
		string output_sb_res = DFGname + "_final_sb_res.txt";

		output_sb_result.open(output_sb_res, ios::out);

		output_sb_result << "LC " << latencyConstraint << endl;
		for (auto i = 0; i < opn; i++) 
			output_sb_result << i << " " << schlResult[i] << " " << opBindingResult[i] << endl;
		
		output_sb_result.close();

		//end of output-file function part.
	}

	std::cout << "All 13 DFGs are done." << endl;

	return 0;
}