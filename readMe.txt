/******************
This ReadMe file contains the description and instructions of:
(1) files are sent to you for the ECE 565 F'23 Final Project;
(2) source codes you need to modify for the asked ML-RCS problem. 

Provided by Xiuyan Zhang, 
Nov 26, 2023.
******************/
(1) Files include:
[1] Library of ADD/MUL with delay (in # of cc's).
filename: lib.txt
delay(ADD): 2 cc's.
delay(MUL): 10 cc's.

In the source code, the data structure to store these metrics is: 
"std::map<int, int> delay", e.g., delay[0] = 2 (for ADD), delay[1] = 10 (for MUL).
------------
[2] 13 DFGs (media benchmark) + the DFG graphs for DFGs except for "300_0.txt" and "1300_0.txt".

filename: DFG.rar
------------
[3] Source code of an MR-LCS (minimize resources under latency constraint) list-scheduling algorithm.

filename: LS.rar
4 code files:
1. LS.h (header of this LS code).
2. LS.cpp: This is the core-LS function.
3. LSMain.cpp: This is the file where main() function is located. This also has the function for outputting result files for the checker to check the correctness of results.
4. readInput.cpp: This is the code to read library, DFGs.
------------
[4] Source code of a checker for the S&B result for the MR-LCS LS.
filename: checker.rar
2 code files:
1. checker.h (header)
2. checker.cpp (main function)

This checker file is used to check the correctness of a result ***FOR MR-LCS problem*** for:
1) Latency constraint satisfaction: output ops (no children/successors) cannot execute beyond the latency constraint. 
2) Data dependency constraint: each operation can only execute when all its predecessors/parents are finished).
3) FU execution error (w/ binding results): one FU cannot have more than one operation executing in the same clock cycle.
=================================

(2) What you need to change for modifying this MR-LCS-based code to an ML-RCS-based code:

[1] For the LS code:
1. You need to have additional inputs as the resource (FU) constraint:
e.g., ResConstr[ADD] = 4, ResConstr[MUL] = 8, etc. 

2. You need to change the LS.cpp (the core function of LS):
For ML-RCS, the fundamental issue is to ensure the maximum number of ops with the same functional type executing in the same clock cycle <= the resource constraint of this type.

[2] For the checker:
1. You need to add the function to check the FU/resource constraint satisfaction:
Again, as discussed above, for each functional type, you need to obtain the # of operations that are executed in each cc's (Note that, operations are executed in multiple cc's).
Check for every cc i and every functional-type k:
if # of operations are executed > resource constraint, then have FU/resource constraint violation for this cc i, type k.



