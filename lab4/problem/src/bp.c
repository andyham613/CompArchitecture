//Andy Ham and Bijan Oviedo (aham) (boviedo)
/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2016
 * Gushu Li and Reza Jokar
 */


#include "bp.h"
#include <stdlib.h>
#include <stdio.h>
#include "pipe.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

void bpt_init() { // all branch predictor structures are initialized to 0. is this function necessary to ensure that?
    //memset(bpt,0,sizeof(bpt));

    bpt.GHR = 0;
    int i,j;
    for (i=0; i<1024; i++) {
        bpt.BTB.bt_entry[i].address_tag = 0;
        bpt.BTB.bt_entry[i].valid_bit = 0;
        bpt.BTB.bt_entry[i].conditional_bit = 0;
        bpt.BTB.bt_entry[i].target_branch = 0;
    }
    for (j=0; j<256; j++) {
        bpt.PHT.two_bit_counter[j] = 0;
    }

}

void bp_predict(int64_t PC, int should_update_PC)
{
    //printf("predicting\n");
    /* Predict next PC */
    int btb_index = (PC >> 2) & 0x3FF;
    int pht_index = (PC >> 2) & 0xFF;
    pht_index = bpt.GHR ^ pht_index;

    //at every fetch cycle, check both BTB and PHT
    int is_valid_bit = bpt.BTB.bt_entry[btb_index].valid_bit;
    int64_t address_tag = bpt.BTB.bt_entry[btb_index].address_tag;

  //  int instruction = cache_update()
    // printf("address_tag: 0x%" PRIx64 "\n", address_tag);
    if ((is_valid_bit == 0 || address_tag != PC) && should_update_PC == 1) {
            // printf("PC SET IN BP PREDICT\n");
        CURRENT_STATE.PC = PC + 4;
    } else if (bpt.BTB.bt_entry[btb_index].conditional_bit == 0 || //(GHR & 0x1) == 1 <-- wrong i think
                bpt.PHT.two_bit_counter[pht_index] >= 2) {
        // printf("branching to: 0x%"  PRIx64 "\n", bpt.BTB.bt_entry[btb_index].target_branch);
        CURRENT_STATE.PC = bpt.BTB.bt_entry[btb_index].target_branch;
    } else if (should_update_PC == 1) {
        CURRENT_STATE.PC = PC + 4;
    }
}

void bp_update(int64_t PC, instruction i, int64_t nPC_m, int is_branch_taken, int* is_correct_branch) // put this in execute stage in pipe.c
{
    /* Update BTB */ //Only BTB update when condtional bit == 0;


    /* Update gshare directional predictor */ //AKA pht??


    /* Update global history register */

    int btb_index = (PC >> 2) & 0x3FF;
    int pht_index = (PC >> 2) & 0xFF;
    pht_index = bpt.GHR ^ pht_index;

    if (bpt.BTB.bt_entry[btb_index].conditional_bit == 1) { // dont update PHT or GHR if unconditional
        //set the unconditional bit in corresponding entry. but how do i change corresponding entry??
        int predictor = bpt.PHT.two_bit_counter[pht_index];
        //pseudo code here for updating PHT (i) // book says 2bitcounter is inverted and stored, but i think this is right.
        if (is_branch_taken) {

            if (predictor != 3) {
                bpt.PHT.two_bit_counter[pht_index]++;
            }
            // could put something here for checking if prediction was correct.
            if (predictor < 2) {
                memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
                memset(&PREGISTER_ID_EX,0, sizeof(PREGISTER_ID_EX));
                *is_correct_branch = 0;
                // printf("INCORRECT BRANCH; PREDICTED: not taken; ACTUAL: taken\n");
                // is_branch_predict_correct = 0;
            } else {
                // memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
                // memset(&PREGISTER_ID_EX,0, sizeof(PREGISTER_ID_EX));
                // printf("CORRECT BRANCH!; PREDICTED: taken; ACTUAL: taken\n");
                *is_correct_branch = 1;
                // is_branch_predict_correct = 1;.
            }
        } else {
            if (predictor != 0) {
                bpt.PHT.two_bit_counter[pht_index]--;
            }
            if (predictor > 1) {
                // printf("INCORRECT BRANCH; PREDICTED: taken; ACTUAL: not taken\n");
                memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
                memset(&PREGISTER_ID_EX,0, sizeof(PREGISTER_ID_EX));
                *is_correct_branch = 0;
                // is_branch_predict_correct = 0;
            } else {
                // memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
                // memset(&PREGISTER_ID_EX,0, sizeof(PREGISTER_ID_EX));
                // printf("CORRECT BRANCH!; PREDICTED: not taken; ACTUAL: not taken\n");
                *is_correct_branch = 1;
                // is_branch_predict_correct = 1;
            }
        }
    }
    if (i != BR, i != BL, i != B) {
        //update GHR after updating PHT (ii)
        if (is_branch_taken) {
            bpt.GHR = ((bpt.GHR << 1) + 1) & 0xFF; //cuz it's only 8 bits?
        } else {
            bpt.GHR = (bpt.GHR << 1) & 0xFF; // only 8bits.
        }
    }


    // (iii) update BTB if miss. do we update anything if it's hit tho?
    if (is_branch_taken && bpt.BTB.bt_entry[btb_index].valid_bit == 0) { //do we need to see if branch taken? or just valid bit
        bpt.BTB.bt_entry[btb_index].address_tag = PC;
        bpt.BTB.bt_entry[btb_index].valid_bit = 1;
        //idk for these two below
        if (i <= BLE && i >= CBNZ ) { // if it's CB-type that means conditional
            bpt.BTB.bt_entry[btb_index].conditional_bit = 1;

            memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
            memset(&PREGISTER_ID_EX,0, sizeof(PREGISTER_ID_EX));
            // printf("INCORRECT BRANCH: EMPTY ENTRY");
            *is_correct_branch = 0;

        } else if (i == BR || i == B || i == BL ) { // if 37, 38, or 4 that means B or BL or BR which are unconditional
            bpt.BTB.bt_entry[btb_index].conditional_bit = 0;
        }
        bpt.BTB.bt_entry[btb_index].target_branch = nPC_m;

    }
    //If you need to add new PC/branch target to BTB, and there's an existing BTB entry,
    // just overwrite it since this is the new one.



}
//BEQ, BLE, etc where you compare something and it has a condition, you set the condition bit to 1, since it is CONDITOINAL
//for stuff like that always jumps, BR, or BL, conditonal bit is 0, since it is unconditional.
//we have to set this bit when when we see the type of instruction. if beq, conditional_bit = 1;

//Branch predict should go in decode stage, once we know it is a branch instruction.

// we check the history register first to find the index into pht. inside pht, we gon check if the prev/most recent value for that index was 00,01,10,11.
// if branch is taken, we ADD that pht value +1, if not, -1. If the pht indicated NOT taken when branch was taken, OR indicated taken when branch]
// was NOT taken, then the prediction was incorrect.

// DONT GET CONFUSED, GSHARE PREDICTOR IS JUST THE NAME OF THIS WHOL PROCESS, GLBOAL HISTORY REGISTER IS UPDATED by doing the bit shift left plus 1;

//beyond these, need to predict indirect and direct branches.
//Also need to flush pipeline after resolving a branch WHEN:
// (I) instr is branch, but predicted direction does not match the actual direction
// (II) instr is indirect branch, and it is taken, but the predictred destination (target) does not match actual destination
// (III) instr is a branch, but it was not recognized as a branch. (BTB miss)

//udate the btb and pht at execute if required
// do not stall for lab3

//BTB is a cache that contains the predicted PC value for taken branches
