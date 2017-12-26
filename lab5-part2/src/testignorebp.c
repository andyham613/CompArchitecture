//for different cpus



void bpt_init_3() { // all branch predictor structures are initialized to 0. is this function necessary to ensure that?
    //memset(bpt_3,0,sizeof(bpt_3));

    bpt_3.GHR = 0;
    int i,j;
    for (i=0; i<1024; i++) {
        bpt_3.BTB.bt_entry[i].address_tag = 0;
        bpt_3.BTB.bt_entry[i].valid_bit = 0;
        bpt_3.BTB.bt_entry[i].conditional_bit = 0;
        bpt_3.BTB.bt_entry[i].target_branch = 0;
    }
    for (j=0; j<256; j++) {
        bpt_3.PHT.two_bit_counter[j] = 0;
    }
    
}

void bp_predict_3(int64_t PC, int should_update_PC)
{
    //printf("predicting\n");
    /* Predict next PC */
    int btb_index = (PC >> 2) & 0x3FF;
    int pht_index = (PC >> 2) & 0xFF;
    pht_index = bpt_3.GHR ^ pht_index;

    //at every fetch cycle, check both BTB and PHT
    int is_valid_bit = bpt_3.BTB.bt_entry[btb_index].valid_bit;
    int64_t address_tag = bpt_3.BTB.bt_entry[btb_index].address_tag;

  //  int instruction = cache_update()
    // printf("address_tag: 0x%" PRIx64 "\n", address_tag);
    if ((is_valid_bit == 0 || address_tag != PC) && should_update_PC == 1) {
            // printf("PC SET IN BP PREDICT\n");
        CURRENT_STATE_3.PC = PC + 4;
    } else if (bpt_3.BTB.bt_entry[btb_index].conditional_bit == 0 || //(GHR & 0x1) == 1 <-- wrong i think
                bpt_3.PHT.two_bit_counter[pht_index] >= 2) {
        // printf("branching to: 0x%"  PRIx64 "\n", bpt_3.BTB.bt_entry[btb_index].target_branch);
        CURRENT_STATE_3.PC = bpt_3.BTB.bt_entry[btb_index].target_branch;
    } else if (should_update_PC == 1) {
        CURRENT_STATE_3.PC = PC + 4;
    }
}

void bp_update_3(int64_t PC, instruction i, int64_t nPC_m, int is_branch_taken, int* is_correct_branch) // put this in execute stage in pipe.c
{
    /* Update BTB */ //Only BTB update when condtional bit == 0;
   

    /* Update gshare directional predictor */ //AKA pht??
    

    /* Update global history register */

    int btb_index = (PC >> 2) & 0x3FF;
    int pht_index = (PC >> 2) & 0xFF;
    pht_index = bpt_3.GHR ^ pht_index;

    if (bpt_3.BTB.bt_entry[btb_index].conditional_bit == 1) { // dont update PHT or GHR if unconditional
        //set the unconditional bit in corresponding entry. but how do i change corresponding entry??
        int predictor = bpt_3.PHT.two_bit_counter[pht_index];
        //pseudo code here for updating PHT (i) // book says 2bitcounter is inverted and stored, but i think this is right.
        if (is_branch_taken) {
            
            if (predictor != 3) {
                bpt_3.PHT.two_bit_counter[pht_index]++;  
            } 
            // could put something here for checking if prediction was correct. 
            if (predictor < 2) {
                memset(&PREGISTER_IF_ID_3, 0, sizeof(PREGISTER_IF_ID_3));
                memset(&PREGISTER_ID_EX_3,0, sizeof(PREGISTER_ID_EX_3));
                *is_correct_branch = 0;
                // printf("INCORRECT BRANCH; PREDICTED: not taken; ACTUAL: taken\n");
                // is_branch_predict_correct = 0;
            } else {
                // memset(&PREGISTER_IF_ID_3, 0, sizeof(PREGISTER_IF_ID_3));
                // memset(&PREGISTER_ID_EX_3,0, sizeof(PREGISTER_ID_EX_3));
                // printf("CORRECT BRANCH!; PREDICTED: taken; ACTUAL: taken\n");
                *is_correct_branch = 1;
                // is_branch_predict_correct = 1;.
            }
        } else {
            if (predictor != 0) {
                bpt_3.PHT.two_bit_counter[pht_index]--; 
            }
            if (predictor > 1) {
                // printf("INCORRECT BRANCH; PREDICTED: taken; ACTUAL: not taken\n");
                memset(&PREGISTER_IF_ID_3, 0, sizeof(PREGISTER_IF_ID_3));
                memset(&PREGISTER_ID_EX_3,0, sizeof(PREGISTER_ID_EX_3));
                *is_correct_branch = 0;
                // is_branch_predict_correct = 0;
            } else {
                // memset(&PREGISTER_IF_ID_3, 0, sizeof(PREGISTER_IF_ID_3));
                // memset(&PREGISTER_ID_EX_3,0, sizeof(PREGISTER_ID_EX_3));
                // printf("CORRECT BRANCH!; PREDICTED: not taken; ACTUAL: not taken\n");
                *is_correct_branch = 1;
                // is_branch_predict_correct = 1;
            }
        }
    }
    if (i != BR, i != BL, i != B) {
        //update GHR after updating PHT (ii)
        if (is_branch_taken) {
            bpt_3.GHR = ((bpt_3.GHR << 1) + 1) & 0xFF; //cuz it's only 8 bits?
        } else {
            bpt_3.GHR = (bpt_3.GHR << 1) & 0xFF; // only 8bits.
        }
    }
    

    // (iii) update BTB if miss. do we update anything if it's hit tho?
    if (is_branch_taken && bpt_3.BTB.bt_entry[btb_index].valid_bit == 0) { //do we need to see if branch taken? or just valid bit
        bpt_3.BTB.bt_entry[btb_index].address_tag = PC;
        bpt_3.BTB.bt_entry[btb_index].valid_bit = 1;
        //idk for these two below
        if (i <= BLE && i >= CBNZ ) { // if it's CB-type that means conditional
            bpt_3.BTB.bt_entry[btb_index].conditional_bit = 1;

            memset(&PREGISTER_IF_ID_3, 0, sizeof(PREGISTER_IF_ID_3));
            memset(&PREGISTER_ID_EX_3,0, sizeof(PREGISTER_ID_EX_3));
            // printf("INCORRECT BRANCH: EMPTY ENTRY");
            *is_correct_branch = 0;

        } else if (i == BR || i == B || i == BL ) { // if 37, 38, or 4 that means B or BL or BR which are unconditional
            bpt_3.BTB.bt_entry[btb_index].conditional_bit = 0;
        }
        bpt_3.BTB.bt_entry[btb_index].target_branch = nPC_m;

    }
    //If you need to add new PC/branch target to BTB, and there's an existing BTB entry, 
    // just overwrite it since this is the new one. 
}
