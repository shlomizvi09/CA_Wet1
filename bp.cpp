/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */
//#define __cplusplus
#include "bp_api.hpp"
#include "math.h"
#include <stdio.h>


#define VALID_BIT 1
#define TARGET_SIZE 30

void initFsmTable(unsigned btbSize, unsigned fsmState, bool isGlobalTable, bool historySize, unsigned **FsmStateTable)
{
	if (isGlobalTable)
	{
		for (int i = 0; i < pow(2, historySize); i++)
			FsmStateTable[0][i] = fsmState;
	}
	else
	{
		for (int j = 0; j < btbSize; j++)
		{
			for (int i = 0; i < pow(2, historySize); i++)
				FsmStateTable[j][i] = 1;
		}
	}
}

unsigned log_2_ciel(unsigned x)
{

	int res = 0;
	while (pow(2, res) <= x)
	{
		res++;
	}
	return res;
}

int calcTotalSize(unsigned btbSize, unsigned historySize, unsigned tagSize,
				  bool isGlobalHist, bool isGlobalTable)
{
	int retval = btbSize * (VALID_BIT + TARGET_SIZE + tagSize);
	if (isGlobalHist)
	{
		retval += btbSize * historySize;
	}
	else
	{
		retval += historySize;
	}

	if (isGlobalTable)
	{
		retval += btbSize * pow(2, (historySize + 1));
	}
	else
	{
		retval += pow(2, (historySize + 1));
	}
	return retval;
}

class btb_entry
{
public:
	int valid;
	int tag;
	int target;

	btb_entry()
	{
		valid = 0;
		tag = 0;
		target = 0;
	}
};

class BranchPred
{

	unsigned historySize;
	btb_entry *btbTable;
	unsigned *historyTable;
	unsigned **FsmStateTable;
	unsigned historyTableSize;
	unsigned FsmTableSize;
	unsigned fsmDefault;

public:
	unsigned btbSize;
	bool isGlobalTable;
	bool isGlobalHist;
	unsigned Shared;
	unsigned tagSize;
	unsigned totalSize;

	unsigned getTagFromPc(uint32_t pc)
	{
		int btbIndexLength = log_2_ciel(btbSize);
		unsigned tag = pc >> 2; //allign address
		tag = tag % int(pow(2, tagSize));
	}

	BranchPred(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			   bool isGlobalHist, bool isGlobalTable, int Shared)
	{
		this->historySize = historySize;
		this->tagSize = tagSize;
		this->btbSize = btbSize;
		this->isGlobalTable = isGlobalTable;
		this->isGlobalHist = isGlobalHist;
		this->Shared = Shared;
		this->fsmDefault=fsmDefault;
		totalSize = calcTotalSize(btbSize, historySize, tagSize,
								  isGlobalHist, isGlobalTable);

		btbTable = new btb_entry[btbSize];

		if (!isGlobalHist && !isGlobalTable)
		{
			historyTable = new unsigned[btbSize];

			FsmStateTable = new unsigned *[btbSize];

			for (unsigned i = 0; i < btbSize; i++)
			{
				historyTable[i] = 0;
				FsmStateTable[i] = new unsigned[unsigned(pow(2, historySize))];
			}
		}

		if (isGlobalHist && !isGlobalTable)
		{

			historyTable = new unsigned[1];

			FsmStateTable = new unsigned *[btbSize];
			for (unsigned i = 0; i < btbSize; i++)
			{
				FsmStateTable[i] = new unsigned[unsigned(pow(2, historySize))];
			}
		}
		if (!isGlobalHist && isGlobalTable)
		{

			historyTable = new unsigned[btbSize];

			FsmStateTable = new unsigned *[1];
			FsmStateTable[0] = new unsigned[unsigned(pow(2, historySize))];
		}

		if (isGlobalHist && isGlobalTable)
		{

			historyTable = new unsigned[1];

			FsmStateTable = new unsigned *[1];
			FsmStateTable[0] = new unsigned[unsigned(pow(2, historySize))];
		}

		initFsmTable(btbSize, fsmState, isGlobalTable, historySize, FsmStateTable);
	}
	void addBtbEntry(uint32_t tag, uint32_t *dst, int btb_index)
	{
		btbTable[btb_index].tag = tag;
		btbTable[btb_index].valid=1;
		btbTable[btb_index].target= *dst;

		if (!isGlobalHist)
		{
			historyTable[btb_index]=0;
		}
		if(!isGlobalTable){
			for(int i=0;i<pow(2,historySize);i++){
				FsmStateTable[btb_index][i]=fsmDefault;
			}
		}
	}
	bool predict(uint32_t pc, uint32_t *dst)
	{
		unsigned btb_index = pc;
		unsigned fsmNum = 0;
		unsigned shareValue = 0;
		unsigned fsmCurrentState = 0;
		btb_index = btb_index >> 2;					   //remove 2 bits for allignment
		btb_index = btb_index % (log_2_ciel(btbSize)); //take only bits needed for btb entry decision.

		if (Shared == 1)
		{											//using_lsb_share
			shareValue = pc >> 2;					//remove 2 lower bits.
			shareValue = shareValue << historySize; //take the lowest |history| bits
		}
		if (Shared == 2)
		{															   //using_mid_share
			shareValue = pc >> 16;									   //remove 16 lower bits.
			shareValue = shareValue % (unsigned(pow(2, historySize))); //take the lowest |history| bits
		}

		if (btbTable[btb_index].valid == 0 || btbTable[btb_index].tag != getTagFromPc(pc))
		{ //check if entry is valid
			*dst = pc + 4;
			return false; //TODO: do we replace btb entry with new brach if they have a different tag ??
			addBtbEntry(getTagFromPc(pc), dst, btb_index);
			//TODO: update btb table
		}

		if (isGlobalHist) //lookup history for branch
		{
			fsmNum = historyTable[0];
		}

		else
		{
			fsmNum = historyTable[btb_index];
		}

		if (Shared > 0)
		{ //xor shared bits with history
			fsmNum = fsmNum ^ shareValue;
		}

		if (isGlobalTable) //lookup fsm current state for branch + history
		{
			fsmCurrentState = FsmStateTable[0][fsmNum];
		}

		else
		{
			fsmCurrentState = FsmStateTable[btb_index][fsmNum];
		}

		if (fsmCurrentState > 1)
		{

			*dst = btbTable[btb_index].target;
			return true;
		}
		else
		{

			*dst = pc + 4;
			return false;
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

	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	unsigned btb_entry = pc;
	
	return global_branch_pred->predict(pc, dst);
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	return;
}

void BP_GetStats(SIM_stats *curStats)
{
	return;
}
