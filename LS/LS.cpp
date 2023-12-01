#include "LS.h"

using namespace std;

const int numberOfFunctionType = 2;	//the number of Function types in the input FU library. 

struct Sclbld				//S&B solution
{
	int achievedLatency;				//achieved latency
	int res[numberOfFunctionType];		//count the number of FUs of each function type used in the S&B solution
	vector<int> scl;					//save the scheduled cc of each operation in the S&B solution
	vector<vector<vector<int>>> bld;	//[function type of the FU][number of the FU][operations bound to the FU]
};

struct preAllocation		//allocation
{
	int FunctionType;		//the function type
	int preNum, postNum;	//the preallocation and postallocation
	double utilizationRate;	//the average utilization rate of all FUs of the function type
};


struct SortSlack {
	bool operator()(const std::pair<int, G_Node>& a, const std::pair<int, G_Node>& b) {
		return a.second.alap < b.second.alap;
	}
};

void ASAP(std::map<int, G_Node>& ops, std::map<int, int>& delay);
int checkParent(G_Node* op, std::map<int, int>& delay);
void ALAP(std::map<int, G_Node>& ops, std::map<int, int>& delay, int& LC);
int checkChild(G_Node* op, std::map<int, int>& delay, int& LC);
void getLC(int& LC, double& latency_parameter, std::map<int, G_Node>& ops, std::map<int, int>& delay);

void LS(std::map<int, int>& schlResult, std::map<int, int>& FUAllocationResult, std::map<int, std::map<int, std::vector<int>>>& bindingResult, int& actualLatency,
	std::map<int, G_Node>& ops, int& latencyConstraint, double& latencyParameter, std::map<int, int>& delay)
{
	ASAP(ops, delay);
	getLC(latencyConstraint, latencyParameter, ops, delay);
	ALAP(ops, delay, latencyConstraint);

	Sclbld sclbld;

	int opn = ops.size(); //# of operations in this DFG.

	vector<preAllocation> Allocation;

	//initialize the allocation structure: only Function types appearing in the DFG are considered
	for (int anOperation = 0; anOperation < opn; anOperation++)
	{
		auto pt = Allocation.begin();
		for (; pt != Allocation.end(); pt++)
			if (pt->FunctionType == ops[anOperation].type)
				break;

		//a new Function type needs to be considered in the allocation structure
		if (pt == Allocation.end())
		{
			preAllocation instance;
			instance.FunctionType = ops[anOperation].type;
			instance.preNum = instance.postNum = 1;
			Allocation.push_back(instance);
		}
	}

	int currentClockCycle;
	vector<int> availableOperations;					//available non-0 slack operations in current clock cycle
	vector<vector<int>> time(numberOfFunctionType);	//time[Function type][an FU] saves the finishing cc of each allocated FU

	currentClockCycle = 1;
	sclbld.scl.assign(opn, 0);
	sclbld.bld = vector<vector<vector<int>>>(numberOfFunctionType, vector<vector<int>>(1, vector<int>(0)));

	//initialize time[][] and res[] according to pre-allocation
	for (int aFunctionType = 0; aFunctionType < numberOfFunctionType; aFunctionType++)
	{
		time[aFunctionType].clear();
		sclbld.res[aFunctionType] = 0;

		auto tpt = Allocation.begin();
		for (; tpt != Allocation.end(); tpt++)
			if (tpt->FunctionType == aFunctionType)
				break;
		if (tpt == Allocation.end())
			continue;
		else
			for (int anFU = 0; anFU < tpt->preNum; anFU++)
			{
				time[aFunctionType].push_back(0);
				sclbld.res[aFunctionType]++;
			}
	}

	//initialize allocation
	for (auto spt = Allocation.begin(); spt != Allocation.end(); spt++)
	{
		for (int i = 0; i < spt->preNum - 1; i++)	//there is one FU allocated for each Function type at the beginning
			sclbld.bld[spt->FunctionType].push_back(vector<int>(0));
		spt->postNum = spt->preNum;
		spt->utilizationRate = 0;
	}

	int numberOfScheduledOperations = 0;		//number of scheduled operations
	while (numberOfScheduledOperations != opn)	//list scheduling begins
	{
		for (int currentFunctionType = 0; currentFunctionType < numberOfFunctionType; currentFunctionType++)	//for each Function type
		{
			for (int currentOperation = 0; currentOperation < opn; currentOperation++)	//for each operation
			{
				//choose an unscheduled operation of the current Function type
				if ((ops[currentOperation].type == currentFunctionType) && (sclbld.scl[currentOperation] == 0))
				{
					//check whether the chosen operation is available (an operation is considered to be available if it has no unscheduled parents
					auto pt = ops[currentOperation].parent.begin();
					if (ops[currentOperation].parent.size() > 0)
						for (; pt != ops[currentOperation].parent.end(); pt++)
							//the parent operation has not been scheduled or its finish cc (the node has been scheduled) is greater than current cc£¬ which means the operation is still unavailable
							if ((sclbld.scl[(*pt)->id] == 0) || (sclbld.scl[(*pt)->id] + delay[ops[(*pt)->id].type] > currentClockCycle))
								break;
					bool operationAvailability = false;
					if (ops[currentOperation].parent.size() > 0)	//the current operation has parent(s)
					{
						if (pt == ops[currentOperation].parent.end())
							operationAvailability = true;
					}
					//the current operation has no parent, means the current operation is available for sure
					else
						operationAvailability = true;

					//if the current operation is available
					if (operationAvailability)
					{
						//if the current operation is 0-slack
						if ((ops[currentOperation].alap - currentClockCycle) == 0)
						{
							//schedule the current operation to the current cc
							bool foundFU = false;	//indicator of whether an available FU is found
							sclbld.scl[currentOperation] = currentClockCycle;
							numberOfScheduledOperations++;

							//find an available FU to bind the current operation
							for (int k = 0; k < time[currentFunctionType].size(); k++)
								if (time[currentFunctionType][k] < currentClockCycle)	//an availble FU is found
								{
									sclbld.bld[currentFunctionType][k].push_back(currentOperation);								//bind the current operation to the FU
									time[currentFunctionType][k] = currentClockCycle + delay[ops[currentOperation].type] - 1;	//update the finishing cc of the FU
									foundFU = true;
									break;
								}

							//an available FU cannot be found, therefore a new FU of the Function type needs to be allocated
							if (!foundFU)
							{
								sclbld.res[currentFunctionType] += 1;															//update the number of FUs of the Function type
								sclbld.bld[currentFunctionType].push_back(vector<int>(0));
								sclbld.bld[currentFunctionType][time[currentFunctionType].size()].push_back(currentOperation);
								time[currentFunctionType].push_back(currentClockCycle + delay[ops[currentOperation].type] - 1);//save the cc after which this FU will be freed
								for (auto spt = Allocation.begin(); spt != Allocation.end(); spt++)									//update the post-allocation
									if (spt->FunctionType == currentFunctionType)
									{
										spt->postNum++;
										break;
									}
							}
						}
						//the current operation is non-0-slack
						else
							availableOperations.push_back(currentOperation);
					}
				}//end operation and Function type matching
			}//end each current operation

			//for the non-0-slack operations, schedule them to available FUs in increasing slack order
			if (!availableOperations.empty())
			{
				vector<std::pair<int, G_Node>> tempOpSet;
				tempOpSet.clear();

				for (auto it = availableOperations.begin(); it != availableOperations.end(); it++)
					tempOpSet.push_back(std::make_pair((*it), ops[*it]));

				//sort operations in increasing ALAP time (slack)
				std::sort(tempOpSet.begin(), tempOpSet.end(), SortSlack());

				//schedule avaialble operations in increasing slack order and bind them to avaialble FUs
				for (auto pt = availableOperations.begin(); pt != availableOperations.end(); pt++)
					for (int k = 0; k < time[currentFunctionType].size(); k++)
						if (time[currentFunctionType][k] < currentClockCycle)
						{
							sclbld.scl[*pt] = currentClockCycle;
							numberOfScheduledOperations++;
							sclbld.bld[currentFunctionType][k].push_back(*pt);
							time[currentFunctionType][k] = currentClockCycle + delay[ops[*pt].type] - 1;
							break;
						}
			}
			availableOperations.clear();
		}//end each Function type
		currentClockCycle++;	//move to the next cc
	}//end list scheduling

	//update allocation if any FU is not used in the LS iteration
	for (auto pt = Allocation.begin(); pt != Allocation.end(); pt++)
		for (int i = 0; i < sclbld.bld[pt->FunctionType].size(); i++)
			if (sclbld.bld[pt->FunctionType][i].size() == 0)
			{
				sclbld.bld[pt->FunctionType].pop_back();
				sclbld.res[pt->FunctionType]--;
				time[pt->FunctionType].pop_back();
				pt->postNum--;
				pt->preNum--;
				i--;
			}

	//get achieved latency of the LS iteration
	sclbld.achievedLatency = 0;
	for (int anOperation = 0; anOperation < opn; anOperation++)
		if (sclbld.achievedLatency < sclbld.scl[anOperation] + delay[ops[anOperation].type] - 1)
			sclbld.achievedLatency = sclbld.scl[anOperation] + delay[ops[anOperation].type] - 1;

	//get output results:
	//schl result
	for (auto i = 0; i < opn; i++)
		schlResult[i] = sclbld.scl[i];

	//FUAllocation result
	for (auto t = 0; t < numberOfFunctionType; t++)
		FUAllocationResult[t] = sclbld.res[t];

	//FU binding result
	for (auto t = 0; t < numberOfFunctionType; t++)
		for (auto i = 0; i < FUAllocationResult[t]; i++)
			bindingResult[t][i] = sclbld.bld[t][i];

	actualLatency = sclbld.achievedLatency;
}

void getLC(int& LC, double& latency_parameter, std::map<int, G_Node>& ops, std::map<int, int>& delay)
{
	int opn = ops.size();
	LC = 0;
	//obtain ASAP latency first
	for (auto i = 0; i < opn; i++)
		if (ops[i].child.empty())
			if (ops[i].asap + delay[ops[i].type] - 1 > LC)
				LC = ops[i].asap + delay[ops[i].type] - 1;
	LC *= latency_parameter;
}

void ASAP(std::map<int, G_Node>& ops, std::map<int, int>& delay)
{
	int opn = ops.size();
	queue <G_Node*> q; //queue to read/update nodes' ASAP
	for (auto i = 0; i < opn; i++)
	{
		ops[i].id = i; //initialize node id
		ops[i].asap = -1; //initialize all node's asap to -1
		ops[i].schl = false; //all nodes are not scheduled.
		if (ops[i].parent.empty()) //push all input nodes into q (no parent)
		{
			ops[i].asap = 1; //input nodes have asap = 1
			q.push(&ops[i]); //push all input nodes into q.
		}
	}
	G_Node* current = new G_Node; //temp node
	int temp = 0;
	while (!q.empty())
	{
		current = q.front(); //read the head of q
		if (current->asap > 0) //if head.asap > 0 (all visited), push all unvisited children into q.
		{
			for (auto it = current->child.begin(); it != current->child.end(); it++) //check all children and see if they can obtain ASAP and push into q
			{
				temp = checkParent(*it, delay);
				if (temp > 0) //all parent are visited (has > 0 T-asap, and then, return my Asap = max Parent Asap + d
				{
					(*it)->asap = temp; //get asap
					q.push((*it)); //push into q
				}
			}
			q.pop(); //pop the current (head node)
		}
	}

	//for (int i = 0; i < opn; i++)
		//cout << "my id: " << i << " , asap time = " << ops[i].asap << endl;


}

int checkParent(G_Node* op, std::map<int, int>& delay)
{
	bool test = false;
	int myAsap = -1;

	for (auto it = op->parent.begin(); it != op->parent.end(); it++)
	{
		if ((*it)->asap > 0)
		{
			test = true;
			if ((*it)->asap + delay[(*it)->type] > myAsap)
				myAsap = (*it)->asap + delay[(*it)->type]; //my ASAP = parent.ASAP + delay
			continue;
		}
		else
		{
			test = false;
			myAsap = -1;
			break;
		}
	}
	return myAsap;
}

void ALAP(std::map<int, G_Node>& ops, std::map<int, int>& delay, int& LC)
{
	int opn = ops.size();
	queue <G_Node*> q; //same as obtain ASAP:
	//push all output node into q first
	for (auto i = 0; i < opn; i++)
	{
		ops[i].alap = LC + 1; //intialize > LC
		if (ops[i].child.empty())
		{
			ops[i].alap = LC - delay[ops[i].type] + 1; //LC-Delay+1
			q.push(&ops[i]); //push into q.
		}
	}
	G_Node* current = new G_Node;
	int temp = 0;
	while (!q.empty())
	{
		current = q.front();
		if (current->alap <= LC) //less than LC, the parent ALAP may be computed
		{
			for (auto it = current->parent.begin(); it != current->parent.end(); it++)
			{
				temp = checkChild((*it), delay, LC);
				if (temp <= LC) //my ALAP has been updated
				{
					(*it)->alap = temp;
					q.push(*it);
				}
			}
			q.pop();
		}
	}
}

int checkChild(G_Node* op, std::map<int, int>& delay, int& LC)
{
	bool test = false;
	int myAlap = LC + 1;
	for (auto it = op->child.begin(); it != op->child.end(); it++)
	{
		if ((*it)->alap <= LC)
		{
			test = true;
			if ((*it)->alap - delay[op->type] <= myAlap)
				myAlap = (*it)->alap - delay[op->type];
			continue;
		}
		else
		{
			test = false;
			myAlap = LC + 1;
			break;
		}
	}
	return myAlap;
}