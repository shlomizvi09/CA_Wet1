/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */
#define __cplusplus
#include "bp_api.hpp"
#define VALID_BIT 1
#define TARGET_SIZE 32

class BranchPred {

	int ** btbTable;
	int ** historyTable;
	int ** FsmStateTable;
	int btbEntrySize;
	int historyTableSize;
	int FsmTableSize;

public:

	BranchPred(unsigned btbSize, unsigned historySize, unsigned tagSize,
			unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared) {
		if (!isGlobalHist && !isGlobalTable) {
			btbTable = new int[btbSize][tagSize + VALID_BIT + TARGET_SIZE];
			historyTable = new int[btbSize][historySize];
			FsmTableSize = new int[btbSize][2 ^ (historySize + 1)];
		}
		if (isGlobalHist && !isGlobalTable) {
			btbTable = new int[btbSize][tagSize + VALID_BIT + TARGET_SIZE];
			historyTable = new int[1][historySize];
			FsmTableSize = new int[btbSize][2 ^ (historySize + 1)];
		}
		if (!isGlobalHist && isGlobalTable) {
			btbTable = new int[btbSize][tagSize + VALID_BIT + TARGET_SIZE];
			historyTable = new int[btbSize][historySize];
			FsmTableSize = new int[1][2 ^ (historySize + 1)];
		}
		if (isGlobalHist && isGlobalTable) {
			btbTable = new int[btbSize][tagSize + VALID_BIT + TARGET_SIZE];
			historyTable = new int[1][historySize];
			FsmTableSize = new int[1][2 ^ (historySize + 1)];
		}
			}
};

class L_H_L_T_BP: public BranchPred {

};

BranchPred * global_branch_pred = nullptr;

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize,
		unsigned fsmState,
		bool isGlobalHist, bool isGlobalTable, int Shared) {

	global_branch_pred = new BranchPred(btbSize, historySize, tagSize, fsmState,
			isGlobalHist, isGlobalTable, Shared);
	global_branch_pred->init_btbTable(btbSize, historySize, tagSize, fsmState,
			isGlobalHist, isGlobalTable, Shared);

	if (!isGlobalHist && !isGlobalTable)
		global_branch_pred = new L_H_L_T_BP;

	return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst) {
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst) {
	return;
}

void BP_GetStats(SIM_stats *curStats) {
	return;
}

