/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */
//#define __cplusplus
#include "bp_api.h"
#include "math.h"
#include <stdio.h>

#define VALID_BIT 1
#define TARGET_SIZE 30

void initFsmTable(unsigned btbSize, unsigned fsmState, bool isGlobalTable, unsigned historySize, unsigned **FsmStateTable)
{
	//printf("in initFsmTable function. initializing all values to: %d \n", fsmState);
	if (isGlobalTable)
	{
		for (int i = 0; i < pow(2, historySize); i++)
		{
			FsmStateTable[0][i] = fsmState;
			//printf("init fsmEntry[%d] to %u  ------ historySize= %d\n ",i, FsmStateTable[0][i],  historySize );
		}
	}
	else
	{
		for (int j = 0; j < btbSize; j++)
		{
			for (int i = 0; i < pow(2, historySize); i++)
			{
				FsmStateTable[j][i] = fsmState;
			}
		}
	}
}

unsigned log_2_ciel(unsigned x)
{

	int res = 0;
	while (pow(2, res) < x)
	{
		res++;
	}
	return res;
}

int calcTotalSize(unsigned btbSize, unsigned historySize, unsigned tagSize,
				  bool isGlobalHist, bool isGlobalTable)
{
	int retval = btbSize * (TARGET_SIZE + tagSize);
	if (!isGlobalHist)
	{
		retval += btbSize * historySize;
	}
	else
	{
		retval += historySize;
	}

	if (!isGlobalTable)
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
	SIM_stats stats = {0, 0, 0};

	unsigned getTagFromPc(uint32_t pc)
	{
		int btbIndexLength = log_2_ciel(btbSize);
		unsigned tag = pc >> (2 + btbIndexLength); //allign address
		return tag % int(pow(2, tagSize));
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
		this->fsmDefault = fsmState;
		this->stats.size = calcTotalSize(btbSize, historySize, tagSize,
								  isGlobalHist, isGlobalTable);

		btbTable = new btb_entry[btbSize];

		if (isGlobalHist){
			historyTable = new unsigned;
			*historyTable = 0;
		} else{
			historyTable = new unsigned[btbSize];
			for (size_t i = 0; i < btbSize; i++)
			{
				historyTable[i] = 0;
			}
		}

		if (isGlobalTable){
			FsmStateTable = new unsigned* ;
			*FsmStateTable = new unsigned[unsigned(pow(2, historySize))];
		} else{
			FsmStateTable = new unsigned *[btbSize];

			for (unsigned i = 0; i < btbSize; i++)
			{
				FsmStateTable[i] = new unsigned[unsigned(pow(2, historySize))];
			}	
		}

		initFsmTable(btbSize, fsmState, isGlobalTable, historySize, FsmStateTable);
	}
	void addBtbEntry(uint32_t tag, uint32_t targetPc, int btb_index)
	{
		btbTable[btb_index].tag = tag;
		btbTable[btb_index].valid = 1;
		btbTable[btb_index].target = targetPc;

		if (!isGlobalHist)
		{
			historyTable[btb_index] = 0;
		}
		if (!isGlobalTable)
		{
			for (int i = 0; i < pow(2, historySize); i++)
			{
				FsmStateTable[btb_index][i] = fsmDefault;
			}
		}
	}
	unsigned calcBtbIndex(uint32_t pc)
	{
		unsigned btb_index = pc >> 2;			   //remove 2 bits for allignment
		// fprintf(stderr, "pc=0x%x, log_2_ciel(btbSize) = %d\n", pc, log_2_ciel(btbSize));

		btb_index = btb_index % unsigned(pow(2, log_2_ciel(btbSize))); //take only bits needed for btb entry decision.
		// fprintf(stderr, "pow = %d\n", unsigned(pow(2, log_2_ciel(btbSize))));
		return btb_index;
	}
	unsigned calcFsmIndex(uint32_t pc)
	{
		unsigned shareValue = 0;
		unsigned fsmNum = 0;
		unsigned btb_index = calcBtbIndex(pc);

		if (this->Shared == 1)
		{											//using_lsb_share
			shareValue = pc >> 2;					//remove 2 lower bits.
			shareValue = shareValue % (unsigned(pow(2, historySize))); //take the lowest |history| bits
		}
		if (this->Shared == 2)
		{															   //using_mid_share
			shareValue = pc >> 16;									   //remove 16 lower bits.
			shareValue = shareValue % (unsigned(pow(2, historySize))); //take the lowest |history| bits
		}

		// if this->Shared == 0 , do nothing

		if (isGlobalHist) //lookup history for branch
		{
			fsmNum = *historyTable;
		} else
		{
			fsmNum = historyTable[btb_index];
		}

		return fsmNum ^ shareValue; // num XOR 0 == num
	}

	bool predict(uint32_t pc, uint32_t *dst)
	{
		unsigned btb_index = calcBtbIndex(pc);
		unsigned fsmNum = calcFsmIndex(pc);
		unsigned fsmCurrentState = 0;

		if (btbTable[btb_index].valid == 0 || btbTable[btb_index].tag != getTagFromPc(pc)) //check if entry is valid
		{ 
			*dst = pc + 4;
			return false; // we assume the first prediction in Not-Taken
		}

		if (isGlobalTable) //lookup fsm current state for branch + history
		{
			fsmCurrentState = (*FsmStateTable)[fsmNum];
		} else
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

	void update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
	{
		unsigned btb_index = calcBtbIndex(pc);
		unsigned fsm_index = calcFsmIndex(pc);

		if (((targetPc != pred_dst) && taken) || ((targetPc == pred_dst) && !taken)){
			this->stats.flush_num++;
		}

		this->stats.br_num++;

		if (btbTable[btb_index].valid == 0 || btbTable[btb_index].tag != getTagFromPc(pc)) //check if entry is valid
		{ 
			addBtbEntry(getTagFromPc(pc), targetPc, btb_index);
		}

		if (isGlobalTable) //update fsm table
		{

			if ((*FsmStateTable)[fsm_index] < 3 && taken)
				(*FsmStateTable)[fsm_index]++;

			if ((*FsmStateTable)[fsm_index] > 0 && !taken)
				(*FsmStateTable)[fsm_index]--;
		}

		else
		{
			if (FsmStateTable[btb_index][fsm_index] < 3 && taken)
				FsmStateTable[btb_index][fsm_index]++;

			if (FsmStateTable[btb_index][fsm_index] > 0 && !taken)
				FsmStateTable[btb_index][fsm_index]--;
		}

		if (isGlobalHist)
		{
			*historyTable = (*historyTable) << 1;						  //move one bit left
			*historyTable += int(taken);								  //update lsb
			*historyTable = *historyTable % unsigned(pow(2, historySize)); 	// cut msb out of history.
		}
		else
		{
			historyTable[btb_index] = historyTable[btb_index] << 1;						  //move one bit left
			historyTable[btb_index] += int(taken);								  //update lsb
			historyTable[btb_index] = historyTable[btb_index] % unsigned(pow(2, historySize)); // cut msb out of history.
		}
	}
};

BranchPred *global_branch_pred;

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize,
			unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){

	global_branch_pred = new BranchPred(btbSize, historySize, tagSize, fsmState,
										isGlobalHist, isGlobalTable, Shared);
	// TODO: think about when BP_init return other than 0 ?
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	return global_branch_pred->predict(pc, dst);
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	global_branch_pred->update(pc, targetPc, taken, pred_dst);
	return;
}

void BP_GetStats(SIM_stats *curStats)
{
	curStats->br_num = global_branch_pred->stats.br_num;
	curStats->flush_num = global_branch_pred->stats.flush_num;
	curStats->size = global_branch_pred->stats.size;
	return;
}
