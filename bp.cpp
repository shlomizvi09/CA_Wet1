/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */
#define __cplusplus
#include "bp_api.hpp"
#define VALID_BIT 1
#define TARGET_SIZE 32
class btb_entry
{
public:
	int valid;
	int tag;
	int target;
};

class BranchPred
{
	bool isGlobalTable;
	bool isGlobalHist;
	int Shared;

	int totalSize;

	btb_entry *btbTable;
	int *historyTable;
	int **FsmStateTable;
	int btbEntrySize;
	int historyTableSize;
	int FsmTableSize;

public:
	BranchPred(unsigned btbSize, unsigned historySize, unsigned tagSize,
			   unsigned fsmState,
			   bool isGlobalHist, bool isGlobalTable, int Shared)
	{
		this->isGlobalTable=isGlobalTable;
		this->isGlobalHist=isGlobalHist;
		this->Shared=Shared;
		

		if (!isGlobalHist && !isGlobalTable)
		{
			btbTable = new btb_entry[btbSize];
			
			historyTable = new int[btbSize];
			
			FsmStateTable = new int *[btbSize];
			for (int i = 0; i < btbSize; i++)
			{
				FsmStateTable[i] = new int[2 ^ historySize];
			}
		}

		if (isGlobalHist && !isGlobalTable)
		{
			btbTable = new btb_entry[btbSize];
			
			historyTable = new int[1];

			FsmStateTable = new int *[btbSize];
			for (int i = 0; i < btbSize; i++)
			{
				FsmStateTable[i] = new int[2 ^ historySize];
			}
		}
		if (!isGlobalHist && isGlobalTable)
		{
			btbTable = new btb_entry[btbSize];

			historyTable = new int[btbSize];

			FsmStateTable = new int *[1];
			FsmStateTable[0] = new int[2 ^ historySize];
		}
		if (isGlobalHist && isGlobalTable)
		{
			btbTable = new btb_entry[btbSize];

			historyTable = new int[1];

			FsmStateTable = new int *[1];
			FsmStateTable[0] = new int[2 ^ historySize];
		}
	}
};

/*class L_H_L_T_BP : public BranchPred
{
	L_H_L_T_BP(unsigned btbSize, unsigned historySize, unsigned tagSize,
			   unsigned fsmState,
			   bool isGlobalHist, bool isGlobalTable, int Shared){

			   }
};*/

BranchPred *global_branch_pred;

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize,
			unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared)
{

	global_branch_pred = new BranchPred(btbSize, historySize, tagSize, fsmState,
										isGlobalHist, isGlobalTable, Shared);


	// if (!isGlobalHist && !isGlobalTable)
	// 	global_branch_pred = new L_H_L_T_BP(btbSize, historySize, tagSize, fsmState,
	// 									isGlobalHist, isGlobalTable, Shared);

	return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	return;
}

void BP_GetStats(SIM_stats *curStats)
{
	return;
}
