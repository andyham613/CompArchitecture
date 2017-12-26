/*------- FOR NAME SWAPPING DIFFERENT CPUS ----------*/
// TODO: so the plan is to do this entire pipe cycle shit 3 more times for the 3 additional CPUS.
// change all the variable names to something_x, since we made global variables for all 4



/*---------------------------------FETCH--------------------------------*/

void handle_cache_delays_3() {
    
    if (globals_3.icache_num_to_squash >= 2) {
        
        memset(&PREGISTER_IF_ID_3,0,sizeof(PREGISTER_IF_ID_3));
        printf("icache bubble (%d)\n", globals_3.icache_num_to_squash);
        globals_3.icache_num_to_squash--;
    } else if (globals_3.icache_num_to_squash == 1) {
        globals_3.should_update_i_cache = TRUE;
        globals_3.machine_code = i_cache_update(i_cache_3, CURRENT_STATE_3.PC);
         printf("icache bubble (%d)\n", globals_3.icache_num_to_squash);
        printf("icache fill at cycle %d\n", stat_cycles_3 + 1);
        globals_3.icache_num_to_squash--;
        // TODO: is this wrong?
        globals_3.should_update_i_cache = TRUE;
        memset(&PREGISTER_IF_ID_3,0,sizeof(PREGISTER_IF_ID_3));
        // printf("icache fill at cycle %d\n", stat_cycles_3);
    }

    if (globals_3.dcache_num_to_stall >= 2) {
        // printf("dcache stall (%d)\n", globals_3.dcache_num_to_stall);
        globals_3.dcache_num_to_stall--;
    } else if (globals_3.dcache_num_to_stall == 1) {
        // printf("dcache stall (%d)\n", globals_3.dcache_num_to_stall);
        // printf("dcache fill at cycle %d\n", stat_cycles_3 + 1);
        globals_3.dcache_num_to_stall--;
      //  globals_3.should_update_d_cache = TRUE;
    }
}
void pipe_stage_fetch_3()
{
 //   printf("Fetch Instruction");
   // print_instruction(decode(mem_read_32(CURRENT_STATE_3.PC)), mem_read_32(CURRENT_STATE_3.PC));
   // printf("\n****************************************************************************************************\n\n");
    if (globals_3.should_squash_fetch || globals_3.detected_halt) {
        globals_3.should_squash_fetch = FALSE;
        if (globals_3.icache_num_to_squash != 0) {
            printf("icache bubble (%d)\n", globals_3.icache_num_to_squash);
        }
        globals_3.icache_num_to_squash = 0;
        memset(&PREGISTER_IF_ID_3,0,sizeof(PREGISTER_IF_ID_3));
        return;
    }
    if (globals_3.detected_load_hazard) {
        globals_3.detected_load_hazard = FALSE;
    //    stat_cycles_3--;
        return;
    }

    if (globals_3.is_unconditional == TRUE) {
        bp_predict_3(CURRENT_STATE_3.PC, 0);
        globals_3.icache_num_to_squash = 0;
        globals_3.is_unconditional = FALSE;
        return;
    }
    if (globals_3.dcache_num_to_stall != 0 || globals_3.icache_num_to_squash != 0) {
        handle_cache_delays_3();
        return;
    } else if (globals_3.icache_num_to_squash == 0 && globals_3.should_update_i_cache) {
     //   printf("icache fill at cycle %d\n", stat_cycles_3);
        printf("icache hit (0x%" PRIx64 ") at cycle %d\n", CURRENT_STATE_3.PC, stat_cycles_3 /*+ 1*/);

        uint32_t machine_code = globals_3.machine_code;
        globals_3.should_update_i_cache = FALSE;
        PREGISTER_IF_ID_3.IR_d = machine_code;
        PREGISTER_IF_ID_3.PC_d = CURRENT_STATE_3.PC;
        bp_predict_3(CURRENT_STATE_3.PC, 1);
        return;
    }
    if (!globals_3.did_forward_flags) {
        PREGISTER_IF_ID_3.flag_n = CURRENT_STATE_3.FLAG_N;
        PREGISTER_IF_ID_3.flag_z = CURRENT_STATE_3.FLAG_Z;
        PREGISTER_IF_ID_3.flag_v = CURRENT_STATE_3.FLAG_V;
        PREGISTER_IF_ID_3.flag_n = CURRENT_STATE_3.FLAG_C;
       // globals_3.did_forward_flags = 0;
    } else {
        globals_3.did_forward_flags = FALSE;
    }
    bool is_hit = i_cache_did_hit(i_cache_3, CURRENT_STATE_3.PC);
    if (!is_hit) {
        memset(&PREGISTER_IF_ID_3,0,sizeof(PREGISTER_IF_ID_3));
        printf("icache miss (0x%" PRIx64 ") at cycle %d\n", CURRENT_STATE_3.PC, stat_cycles_3 + 1);
       // handle_cache_delays_3();
        globals_3.icache_num_to_squash = 49;
        return;
    } else {
        printf("icache hit (0x%" PRIx64 ") at cycle %d\n", CURRENT_STATE_3.PC, stat_cycles_3 /*+ 1*/);
        uint32_t machine_code = i_cache_update(i_cache_3, CURRENT_STATE_3.PC);
        PREGISTER_IF_ID_3.IR_d = machine_code;
        PREGISTER_IF_ID_3.PC_d = CURRENT_STATE_3.PC;
    }

  //  int64_t PC_before_bp_predict_3 = CURRENT_STATE_3.PC;
    bp_predict_3(CURRENT_STATE_3.PC, 1);
   // int64_t PC_after_bp_predict_3 = CURRENT_STATE_3.PC;
}

/*--------------------------------DECODE--------------------------------*/

void pipe_stage_decode_r_instruction_3(uint32_t machine_code) {
    uint32_t shamt = (machine_code >> 10) & 0x3F;
    uint32_t Rn = (machine_code >> 5) & 0x1F; // Rn
    uint32_t Rm = (machine_code >> 16) & 0x1F; // Rm
    uint32_t shift_bits = (((machine_code) >> 22) & 0x3) << 6;
    PREGISTER_ID_EX_3.register_Rn1 = Rn;
    PREGISTER_ID_EX_3.register_Rm2 = Rm;
    PREGISTER_ID_EX_3.A_e = CURRENT_STATE_3.REGS[Rn];
    PREGISTER_ID_EX_3.B_e = CURRENT_STATE_3.REGS[Rm];
    PREGISTER_ID_EX_3.Imm_e = shamt | shift_bits;
    PREGISTER_ID_EX_3.write_register_number = machine_code & 0x1F;
}

void pipe_stage_decode_cb_instruction_3(uint32_t machine_code) {
    uint32_t read_register_1 = (machine_code >> 5) & 0x1F; //Rt
    uint32_t COND_BR_address = (machine_code >> 5) & 0x7FFFF;

    PREGISTER_ID_EX_3.A_e = CURRENT_STATE_3.REGS[read_register_1];
    PREGISTER_ID_EX_3.Imm_e = COND_BR_address & 0x00000000FFFFFFFF;
    PREGISTER_ID_EX_3.write_register_number = 32;
    PREGISTER_ID_EX_3.register_Rn1 = read_register_1;
    PREGISTER_ID_EX_3.register_Rm2 = 32;
}
void pipe_stage_decode_d_instruction_3(uint32_t machine_code, instruction i) {
    uint32_t Rn = (machine_code >> 5) & 0x1F; //Rn
    // printf("Register Rn: %08x\n", Rn);
    uint32_t immediate = (machine_code >> 10) & 0x7FF;
    // the immediate is the dt address and the op in one
    PREGISTER_ID_EX_3.A_e = CURRENT_STATE_3.REGS[Rn];
    PREGISTER_ID_EX_3.Imm_e = immediate;
    
    if (i == LDUR || i == LDURB || i == LDURH) {
        PREGISTER_ID_EX_3.write_register_number = machine_code & 0x1F;
        PREGISTER_ID_EX_3.register_Rn1 = Rn;
        PREGISTER_ID_EX_3.register_Rm2 = 32;
    } else {
        PREGISTER_ID_EX_3.register_Rn1 = Rn;
        PREGISTER_ID_EX_3.register_Rm2 = machine_code & 0x1F;
        // printf("Register Rt: %08x\n", machine_code & 0x1F);
        PREGISTER_ID_EX_3.B_e = CURRENT_STATE_3.REGS[machine_code & 0x1F];
        PREGISTER_ID_EX_3.write_register_number = 32;
    }
}

void pipe_stage_decode_i_instruction_3(uint32_t machine_code) {
    uint32_t Rn = (machine_code >> 5) & 0x1F; // Rn
    uint32_t ALU_immediate = (machine_code >> 10) & 0xFFF;

    PREGISTER_ID_EX_3.write_register_number = machine_code & 0x1F;
    PREGISTER_ID_EX_3.register_Rn1 = Rn;
    PREGISTER_ID_EX_3.register_Rm2 = 32;
    PREGISTER_ID_EX_3.A_e = CURRENT_STATE_3.REGS[Rn];
    PREGISTER_ID_EX_3.Imm_e = ALU_immediate;
}

void pipe_stage_decode_shift_instruction_3(uint32_t machine_code, instruction i) {
    uint32_t Rn = (machine_code >> 5) & 0x1F;
    uint32_t Rm = (machine_code >> 16) & 0x1F;
    uint32_t immediate = (machine_code >> 16) & 0x3F;
    PREGISTER_ID_EX_3.A_e = CURRENT_STATE_3.REGS[Rn];
    PREGISTER_ID_EX_3.B_e = CURRENT_STATE_3.REGS[Rm];
    PREGISTER_ID_EX_3.Imm_e = immediate;
    PREGISTER_ID_EX_3.register_Rn1 = Rn;
    PREGISTER_ID_EX_3.register_Rm2 = Rm;
    PREGISTER_ID_EX_3.write_register_number = machine_code & 0x1F;
}

void pipe_stage_decode_b_instruction_3(uint32_t machine_code, instruction i) {
    uint32_t immediate = machine_code & 0x3FFFFFF;
    PREGISTER_ID_EX_3.Imm_e = immediate;
    // no need for write register
    PREGISTER_ID_EX_3.write_register_number = 32;
    uint32_t BR_address = PREGISTER_ID_EX_3.Imm_e;
    int64_t BR_address64 = (BR_address >> 25) ? (0xFFFFFFFFF0000000 | ((int64_t)BR_address << 2)) : 0 | (BR_address << 2);
    
    int nPC_m = PREGISTER_IF_ID_3.PC_d + BR_address64;
    bp_update_3(PREGISTER_IF_ID_3.PC_d, i, nPC_m,1,NULL);
    globals_3.is_unconditional = TRUE;
    CURRENT_STATE_3.PC = nPC_m;

    memset(&PREGISTER_IF_ID_3,0, sizeof(PREGISTER_IF_ID_3));
}
void pipe_stage_decode_iw_format_3(uint32_t machine_code) {
    uint32_t MOV_immediate = (machine_code >> 5) & 0x3FFFF;
    PREGISTER_ID_EX_3.Imm_e = MOV_immediate;
    PREGISTER_ID_EX_3.write_register_number = machine_code & 0x1F;
}
void pipe_stage_decode_3()
{
    if (globals_3.dcache_num_to_stall != 0) {
        return;
    }
    if (globals_3.should_squash_decode == TRUE) {
       // memset(&PREGISTER_ID_EX_3,0,sizeof(PREGISTER_ID_EX_3));
        PREGISTER_ID_EX_3.i = NO_INSTRUCTION;
        return;
    }
    if (globals_3.detected_load_hazard == TRUE) {
        return;
    }
    uint32_t machine_code = PREGISTER_IF_ID_3.IR_d;
    instruction i = decode(machine_code);
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("Decode Instruction: \n");
        print_instruction(i, machine_code);
    }
 
    if (i == NO_INSTRUCTION) { 
        memset(&PREGISTER_ID_EX_3,0,sizeof(PREGISTER_ID_EX_3));
        return;
    } else if (i <= UDIV) { // R-FORMAT
        pipe_stage_decode_r_instruction_3(machine_code);
    } else if (i <= BLE) { // CB-FORMAT
        pipe_stage_decode_cb_instruction_3(machine_code);
    } else if (i <= STURW) { // D-FORMAT
        pipe_stage_decode_d_instruction_3(machine_code,i);
    } else if (i <= SUBIS) { // I-FORMAT
        pipe_stage_decode_i_instruction_3(machine_code);
    } else if (i <= LSR) { // SHIFT-TYPE
        pipe_stage_decode_shift_instruction_3(machine_code, i);
    } else if (i <= BL) { // B-FORMAT
        pipe_stage_decode_b_instruction_3(machine_code, i);
    } else if (i <= MOVZ) { // IW-FORMAT
        pipe_stage_decode_iw_format_3(machine_code);
    } else if (i == ERET) {
        //stall in decode stage until other instr in ex, mem, wb are complete.
        printf("ERET detected\n");
    } else { // HLT
        CURRENT_STATE_3.PC += 4;
        globals_3.detected_halt = TRUE;
    }

    PREGISTER_ID_EX_3.flag_n = PREGISTER_IF_ID_3.flag_n;
    PREGISTER_ID_EX_3.flag_z = PREGISTER_IF_ID_3.flag_z;
    PREGISTER_ID_EX_3.flag_v = PREGISTER_IF_ID_3.flag_v;
    PREGISTER_ID_EX_3.flag_c = PREGISTER_IF_ID_3.flag_c;
    PREGISTER_ID_EX_3.PC_e = PREGISTER_IF_ID_3.PC_d;
    PREGISTER_ID_EX_3.i = i;
    PREGISTER_ID_EX_3.machine_code = machine_code;
}

/*--------------------------------EXECUTE--------------------------------*/

void pipe_stage_execute_r_instruction_3(instruction i) {
    uint32_t shamt = (PREGISTER_ID_EX_3.Imm_e & 0x3F);
    uint32_t shift = ((PREGISTER_ID_EX_3.Imm_e >> 6) & 0x3);

    int64_t op1 = PREGISTER_ID_EX_3.A_e;  //1st register source operand Rn's value
    int64_t op2 = PREGISTER_ID_EX_3.B_e;  //2nd register source operand Rm's value

    switch (i) {
        case ADD:
            PREGISTER_EX_MEM_3.Aout_m = op1 + op2;
            break;
        case ADDS:
            PREGISTER_EX_MEM_3.flag_n = IS_NEGATIVE(op1 + op2);
            PREGISTER_EX_MEM_3.flag_z = IS_ZERO(op1 + op2);
            PREGISTER_EX_MEM_3.flag_c = (op1 + op2 < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM_3.flag_v = ((op1 > 0 && op2 > 0 && op1 + op2 < op1) || (op1 < 0 && op2 < 0 && op1 + op2 > 0)) ? 1 : 0;

            PREGISTER_EX_MEM_3.Aout_m = op1 + op2;
            break;
        case AND:
            if (shift == 0) { // LSL
                 PREGISTER_EX_MEM_3.Aout_m = op1 & (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 PREGISTER_EX_MEM_3.Aout_m = op1 & ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if (shift == 2) {  // ASR
                 PREGISTER_EX_MEM_3.Aout_m = op1 & (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 PREGISTER_EX_MEM_3.Aout_m = op1 & (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }
            break;
        case ANDS:
            PREGISTER_EX_MEM_3.flag_c = 0;
            PREGISTER_EX_MEM_3.flag_v = 0;
            PREGISTER_EX_MEM_3.flag_z = IS_ZERO(op1 & op2);
            PREGISTER_EX_MEM_3.flag_n = IS_NEGATIVE(op1 & op2);

            if (shift == 0) { // LSL
                 PREGISTER_EX_MEM_3.Aout_m = op1 & (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 PREGISTER_EX_MEM_3.Aout_m = op1 & ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if ( shift == 2) {  // ASR
                 PREGISTER_EX_MEM_3.Aout_m = op1 & (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 PREGISTER_EX_MEM_3.Aout_m = op1 & (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }  
            break;
        case BR:
            PREGISTER_EX_MEM_3.nPC_m = op1;
            break;
        case EOR: ;
            if (shift == 0) { // LSL
                 PREGISTER_EX_MEM_3.Aout_m = op1 ^ (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 PREGISTER_EX_MEM_3.Aout_m = op1 ^ ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if (shift == 2) {  // ASR
                 PREGISTER_EX_MEM_3.Aout_m = op1 ^ (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 PREGISTER_EX_MEM_3.Aout_m = op1 ^ (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }
            break;
        case MUL:
            PREGISTER_EX_MEM_3.Aout_m = op1 * op2;
            break;
        case ORR:
            if (shift == 0) { // LSL
                 PREGISTER_EX_MEM_3.Aout_m = op1 | (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 PREGISTER_EX_MEM_3.Aout_m = op1 | ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if ( shift == 2) {  // ASR
                 PREGISTER_EX_MEM_3.Aout_m = op1 | (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 PREGISTER_EX_MEM_3.Aout_m = op1 | (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64 - shamt)));
            }
            break;
        case SDIV:
            PREGISTER_EX_MEM_3.Aout_m = (op2 != 0) ? ((int64_t) ((double) op1 / (double) op2)) : 0;
            break;
        case SUB:
            PREGISTER_EX_MEM_3.Aout_m = op1 - op2;
            break;
        case SUBS:
        case CMP:

            PREGISTER_EX_MEM_3.flag_n = IS_NEGATIVE(op1 - op2);
            PREGISTER_EX_MEM_3.flag_z = IS_ZERO(op1 - op2);
            PREGISTER_EX_MEM_3.flag_c = (op1 - op2 < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM_3.flag_v = ((op1 > 0 && op2 < 0 && op1 - op2 < op1) || (op1 < 0 && op2 < 0 && op1 - op2 > 0)) ? 1 : 0;

            PREGISTER_EX_MEM_3.Aout_m = op1 - op2;
            break;
        case UDIV:
            PREGISTER_EX_MEM_3.Aout_m = (op2 != 0) ? ((int64_t) ((unsigned long long) op1 / (unsigned long long) op2)) : 0;
            break;
    }

    PREGISTER_EX_MEM_3.will_write = 1;
    PREGISTER_EX_MEM_3.write_register_number = PREGISTER_ID_EX_3.write_register_number;

}      

void pipe_stage_execute_cb_instruction_3(instruction i) {

    int64_t current_state_pc = PREGISTER_ID_EX_3.PC_e;
    int64_t value_in_reg_rt = PREGISTER_ID_EX_3.A_e;
    uint32_t COND_BR_address = PREGISTER_ID_EX_3.Imm_e;

    int64_t CondBranchAddr = ((COND_BR_address >> 18) != 0) ? (COND_BR_address << 2) | 0xFFFFFFFFFFE00000 : (CondBranchAddr = 0 | (COND_BR_address << 2));
    int64_t branch_address = current_state_pc + CondBranchAddr;
    int64_t next_address = current_state_pc + 4;

    uint32_t N = PREGISTER_ID_EX_3.flag_n;
    uint32_t Z = PREGISTER_ID_EX_3.flag_z;
    uint32_t V = PREGISTER_ID_EX_3.flag_v;
    uint32_t C = PREGISTER_ID_EX_3.flag_c;

    switch(i) {
        case CBNZ:
            PREGISTER_EX_MEM_3.nPC_m = (value_in_reg_rt != 0) ? branch_address : next_address;
            break;
        case CBZ:
            PREGISTER_EX_MEM_3.nPC_m = (value_in_reg_rt == 0) ? branch_address : next_address;
            break;
        case BEQ:
            PREGISTER_EX_MEM_3.nPC_m = (Z == 1) ? branch_address : next_address;
            break;
        case BNE:
            PREGISTER_EX_MEM_3.nPC_m = (Z == 0) ? branch_address : next_address;
            break;
        case BGT:
            PREGISTER_EX_MEM_3.nPC_m = (Z == 0 && N == V) ? branch_address : next_address;
            break;
        case BLT:
            PREGISTER_EX_MEM_3.nPC_m = (N != V) ? branch_address : next_address;
            break;
        case BGE:
            PREGISTER_EX_MEM_3.nPC_m = (N == V) ? branch_address : next_address;
            break;
        case BLE:
            PREGISTER_EX_MEM_3.nPC_m = (!(Z == 0 && N == V)) ? branch_address : next_address;
            break;
    }

    PREGISTER_EX_MEM_3.will_write = 0;
}

void pipe_stage_execute_d_instruction_3(instruction i) {
    uint32_t DT_address = ((PREGISTER_ID_EX_3.Imm_e >> 2) & 0x1FF);
    uint32_t Rn_value = PREGISTER_ID_EX_3.A_e;
    int64_t address_to_rw = Rn_value + DT_address;

    PREGISTER_EX_MEM_3.Aout_m = address_to_rw;
    PREGISTER_EX_MEM_3.nPC_m = PREGISTER_ID_EX_3.PC_e;
    PREGISTER_EX_MEM_3.B_m = PREGISTER_ID_EX_3.B_e;
    PREGISTER_EX_MEM_3.will_write = 0;
}

void pipe_stage_execute_i_instruction_3(instruction i) {
    int64_t immediate = PREGISTER_ID_EX_3.Imm_e;
    int64_t op1 = PREGISTER_ID_EX_3.A_e; //Rn 1st register source operand

    switch(i) {
        case ADDI:
            PREGISTER_EX_MEM_3.Aout_m = op1 + immediate;
            break;
        case ADDIS:
            PREGISTER_EX_MEM_3.flag_c = (op1 + immediate < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM_3.flag_n = IS_NEGATIVE(op1 + immediate);
            PREGISTER_EX_MEM_3.flag_z = IS_ZERO(op1 + immediate);
            PREGISTER_EX_MEM_3.flag_v = ((op1 > 0 && immediate > 0 && op1 + immediate < op1) || (op1 < 0 && immediate < 0 && op1 + immediate > 0)) ? 1 : 0;

            PREGISTER_EX_MEM_3.Aout_m = op1 + immediate;
            break;
        case SUBI:
            PREGISTER_EX_MEM_3.Aout_m = op1 - immediate;
            break;
        case SUBIS:
            PREGISTER_EX_MEM_3.flag_c = (op1 - immediate < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM_3.flag_n = IS_NEGATIVE(op1 - immediate);
            PREGISTER_EX_MEM_3.flag_z = IS_ZERO(op1 - immediate);
            PREGISTER_EX_MEM_3.flag_v = ((op1 > 0 && immediate < 0 && op1 - immediate < 0) || (op1 < 0 && immediate > 0 && op1 - immediate > 0)) ? 1 : 0;

            PREGISTER_EX_MEM_3.Aout_m = op1 - immediate;
            break;
    }
    PREGISTER_EX_MEM_3.will_write = 1;
}

void pipe_stage_execute_shift_instruction_3(instruction i) {
    uint32_t immr = PREGISTER_ID_EX_3.Imm_e; // for immediate shifts // (machine_code >> 16) & 0x3F;
    int64_t op1 = PREGISTER_ID_EX_3.A_e; //Rn
    int64_t op2 = PREGISTER_ID_EX_3.B_e; // Rm for shift registers
    uint32_t shamt = (i == LSLI) ? ((immr ^ 0x3F) + 1) : immr;
    if (i == LSL) {
        PREGISTER_EX_MEM_3.Aout_m = op1 << op2;
    } else if (i == LSR) {
        PREGISTER_EX_MEM_3.Aout_m = op1 >> op2;
    } else if (i == LSLI) {
        PREGISTER_EX_MEM_3.Aout_m = op1 << shamt;
    } else if (i == LSRI) {
        PREGISTER_EX_MEM_3.Aout_m = op1 >> shamt;
    }
    PREGISTER_EX_MEM_3.will_write = 1;
}

void pipe_stage_execute_b_instruction_3(instruction i) {
    uint32_t BR_address = PREGISTER_ID_EX_3.Imm_e;
    int64_t BR_address64 = (BR_address >> 25) ? (0xFFFFFFFFF0000000 | ((int64_t)BR_address << 2)) : 0 | (BR_address << 2);
    PREGISTER_EX_MEM_3.nPC_m = PREGISTER_ID_EX_3.PC_e + BR_address64;
    PREGISTER_EX_MEM_3.will_write = 0;
}

void pipe_stage_execute_iw_instruction_3(instruction i) {
    uint32_t MOV_immediate = PREGISTER_ID_EX_3.Imm_e & 0xFFFF;
    uint32_t op2 = (PREGISTER_ID_EX_3.Imm_e >> 16) & 0x3;
    // the instruction can only be MOVZ
    PREGISTER_EX_MEM_3.Aout_m = MOV_immediate << (op2 * 16);
    PREGISTER_EX_MEM_3.will_write = 1;
}


void pipe_stage_execute_3() {
    instruction i = PREGISTER_ID_EX_3.i;
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("Execute Instruction:\n");
        print_instruction(i,PREGISTER_ID_EX_3.machine_code);
    }
    if (globals_3.dcache_num_to_stall != 0) {
        return;
    }
    if (globals_3.detected_load_hazard) {
        memset(&PREGISTER_EX_MEM_3,0,sizeof(PREGISTER_EX_MEM_3));
        globals_3.detected_load_hazard = 1;
        return;
    }
    if (i == NO_INSTRUCTION) {
        memset(&PREGISTER_EX_MEM_3,0,sizeof(PREGISTER_EX_MEM_3));
        return;
    } else if (i <= UDIV) { // R-Format
        pipe_stage_execute_r_instruction_3(i);
    } else if (i <= BLE) { // CB Format
        pipe_stage_execute_cb_instruction_3(i);
    } else if (i <= STURW) { // D Format //IMM has DT addr + op2
        pipe_stage_execute_d_instruction_3(i);
    } else if (i <= SUBIS) { //I Format
        pipe_stage_execute_i_instruction_3(i);
    } else if (i <= LSR) { // Shift Format
        pipe_stage_execute_shift_instruction_3(i);
    } else if (i <= BL) { // B Format
        pipe_stage_execute_b_instruction_3(i);
    } else if (i <= MOVZ) { // IW Format
        pipe_stage_execute_iw_instruction_3(i);
    }

    if (i == ADDS || i == ANDS || i == SUBS || i == CMP || i == ADDIS || i == SUBIS) {
        globals_3.did_forward_flags = TRUE;
        PREGISTER_ID_EX_3.flag_n = PREGISTER_EX_MEM_3.flag_n;
        PREGISTER_IF_ID_3.flag_n = PREGISTER_EX_MEM_3.flag_n;
        PREGISTER_ID_EX_3.flag_z = PREGISTER_EX_MEM_3.flag_z;
        PREGISTER_IF_ID_3.flag_z = PREGISTER_EX_MEM_3.flag_z;
        PREGISTER_ID_EX_3.flag_c = PREGISTER_EX_MEM_3.flag_c;
        PREGISTER_IF_ID_3.flag_c = PREGISTER_EX_MEM_3.flag_c;
        PREGISTER_ID_EX_3.flag_v = PREGISTER_EX_MEM_3.flag_v;
        PREGISTER_IF_ID_3.flag_v = PREGISTER_EX_MEM_3.flag_v;
    }

    if (i <= BLE && i >= CBNZ) {
        int is_correct_branch;
        if (PREGISTER_EX_MEM_3.nPC_m != PREGISTER_ID_EX_3.PC_e + 4) {

            // TODO: do we need this?
           
            bp_update_3(PREGISTER_ID_EX_3.PC_e, i, PREGISTER_EX_MEM_3.nPC_m,1, &is_correct_branch);
            if (!is_correct_branch) {
          //      globals_3.icache_num_to_squash = 0;
                CURRENT_STATE_3.PC = PREGISTER_EX_MEM_3.nPC_m;
                globals_3.should_squash_fetch = TRUE;
                memset(&PREGISTER_IF_ID_3, 0, sizeof(PREGISTER_IF_ID_3));
                memset(&PREGISTER_ID_EX_3,0, sizeof(PREGISTER_ID_EX_3));
            }
        } else {
            bp_update_3(PREGISTER_ID_EX_3.PC_e, i, PREGISTER_EX_MEM_3.nPC_m,0, &is_correct_branch);
            // printf("PC SET\n");
            // printf("Address to jump to: 0x%" PRIx64 "\t", PREGISTER_EX_MEM_3.nPC_m);
            // printf("next address: 0x%" PRIx64 "\n", PREGISTER_ID_EX_3.PC_e + 4);
            CURRENT_STATE_3.PC = PREGISTER_EX_MEM_3.nPC_m;

            if (!is_correct_branch) {
                // printf("not correct branch\n");
                globals_3.should_squash_fetch = TRUE;
                memset(&PREGISTER_ID_EX_3, 0, sizeof(PREGISTER_IF_ID_3));
                memset(&PREGISTER_IF_ID_3,0, sizeof(PREGISTER_IF_ID_3));
            }
            // memset(&PREGISTER_ID_EX_3, 0, sizeof(PREGISTER_IF_ID_3));
            //     memset(&PREGISTER_IF_ID_3,0, sizeof(PREGISTER_IF_ID_3));
        }
    } else if (i == BR || i == B || i == BL) {
       // bp_update_3(PREGISTER_ID_EX_3.PC_e, i, PREGISTER_EX_MEM_3.nPC_m,1, NULL);
        // printf("BR/B/BL Instruction\n");
        //stat_inst_retire_3++;
        // PREGISTER_MEM_WB_3.is_unconditional = 1;
        // if (!is_correct_branch) {
        //     CURRENT_STATE_3.PC
        // }
        //CURRENT_STATE_3.PC = PREGISTER_EX_MEM_3.nPC_m;
    }

    PREGISTER_EX_MEM_3.write_register_number = PREGISTER_ID_EX_3.write_register_number;
    PREGISTER_EX_MEM_3.i = i;
    PREGISTER_EX_MEM_3.machine_code = PREGISTER_ID_EX_3.machine_code;
}

/*------------------------------MEMORY ACCESS------------------------------*/
void pipe_stage_mem()
{
    instruction i = PREGISTER_EX_MEM_3.i;
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("Mem Instruction:\n");
        print_instruction(i, PREGISTER_EX_MEM_3.machine_code);
    }
    if (i == NO_INSTRUCTION) {
        memset(&PREGISTER_MEM_WB_3,0,sizeof(PREGISTER_MEM_WB_3));
        return;
    }

    if (globals_3.dcache_num_to_stall >= 2) {
        printf("dcache stall (%d)\n", globals_3.dcache_num_to_stall);
        memset(&PREGISTER_MEM_WB_3,0,sizeof(PREGISTER_MEM_WB_3));
        return;
    }
    if (globals_3.dcache_num_to_stall == 1) {
        int64_t address_to_rw = PREGISTER_EX_MEM_3.Aout_m;
        int64_t write_data = PREGISTER_EX_MEM_3.B_m;
        printf("dcache stall (%d)\n", globals_3.dcache_num_to_stall);
        printf("dcache fill at cycle %d\n", stat_cycles_3 + 1);
        switch(i) {
            case LDUR: ;
                int64_t full_value;
                d_cache_update(d_cache_3, i, address_to_rw, 0, &full_value);
                globals_3.dcache_stored_value = full_value;
                // printf("LDUR address read: %" PRId64 "\n", full_value);
                break;
            case LDURB:
                globals_3.dcache_stored_value = d_cache_update(d_cache_3, i, address_to_rw, 0, NULL) & 0xFF;
                break;
            case LDURH:
                globals_3.dcache_stored_value = d_cache_update(d_cache_3, i, address_to_rw, 0, NULL) & 0xFFFF;;
                break;
            case STUR:
                d_cache_update(d_cache_3, i, address_to_rw, write_data, NULL);
                break;
            case STURB:
                d_cache_update(d_cache_3, i, address_to_rw, write_data & 0xFF, NULL);
                break;
            case STURH:
                d_cache_update(d_cache_3, i, address_to_rw, write_data & 0xFFFF, NULL);
                break;
            case STURW:
                d_cache_update(d_cache_3, i, address_to_rw, write_data & 0xFFFFFFFF, NULL);
                break;
        }
        globals_3.should_update_d_cache = TRUE;
        memset(&PREGISTER_MEM_WB_3,0,sizeof(PREGISTER_MEM_WB_3));
        return;
    }
    if (globals_3.dcache_num_to_stall == 0 && globals_3.should_update_d_cache) {
        int64_t address_to_rw = PREGISTER_EX_MEM_3.Aout_m;
        if (i == LDUR || i == LDURB || i == LDURH) {
            PREGISTER_MEM_WB_3.Aout_w = address_to_rw;
            PREGISTER_MEM_WB_3.MDR_w = globals_3.dcache_stored_value;
        }
        globals_3.should_update_d_cache = FALSE;
        PREGISTER_MEM_WB_3.write_register_number = PREGISTER_EX_MEM_3.write_register_number;
        PREGISTER_MEM_WB_3.i = i;
        PREGISTER_MEM_WB_3.will_write = PREGISTER_EX_MEM_3.will_write;
        PREGISTER_MEM_WB_3.machine_code = PREGISTER_EX_MEM_3.machine_code;
        return;
    }
    
    if (PREGISTER_EX_MEM_3.will_write && PREGISTER_EX_MEM_3.write_register_number < 31) {
        if (PREGISTER_EX_MEM_3.write_register_number == PREGISTER_ID_EX_3.register_Rn1) {
            PREGISTER_ID_EX_3.A_e = PREGISTER_EX_MEM_3.Aout_m;
        }
        if (PREGISTER_EX_MEM_3.write_register_number == PREGISTER_ID_EX_3.register_Rm2) {
            PREGISTER_ID_EX_3.B_e = PREGISTER_EX_MEM_3.Aout_m;
        }
    }

    if (i == LDUR || i == LDURH || i == LDURB) {
        if (PREGISTER_EX_MEM_3.write_register_number == PREGISTER_ID_EX_3.register_Rn1 ||
            PREGISTER_EX_MEM_3.write_register_number == PREGISTER_ID_EX_3.register_Rm2) {
            // stall the pipeline
            globals_3.detected_load_hazard = TRUE;
        }
    }
    
    if (i <= STURW && i >= LDUR) {// if load or store
        int64_t address_to_rw = PREGISTER_EX_MEM_3.Aout_m;
        int64_t write_data = PREGISTER_EX_MEM_3.B_m;
     //   uint32_t half_byte_1 = 0, half_byte_2 = 0;
      //  int64_t full_value = 0;
        bool is_hit = d_cache_did_hit(d_cache_3, address_to_rw, i);
        if (!is_hit) {
            printf("dcache miss (0x%" PRIx64 ") at cycle %d\n", address_to_rw, stat_cycles_3 + 1);
            globals_3.dcache_num_to_stall = 50;
            PREGISTER_MEM_WB_3.i = NO_INSTRUCTION;
            return;
        } else {
            printf("dcache hit (0x%" PRIx64 ") at cycle %d\n", address_to_rw, stat_cycles_3 /*+ 1*/);
            switch(i) {
                case LDUR: ;
                    int64_t full_value;
                    d_cache_update(d_cache_3, i, address_to_rw, 0, &full_value);
                    PREGISTER_MEM_WB_3.MDR_w = full_value;
                    PREGISTER_MEM_WB_3.Aout_w = address_to_rw;
                    break;
                case LDURB:
                    PREGISTER_MEM_WB_3.MDR_w = d_cache_update(d_cache_3, i, address_to_rw, 0, NULL) & 0xFF;
                    PREGISTER_MEM_WB_3.Aout_w = address_to_rw;
                    break;
                case LDURH:
                    PREGISTER_MEM_WB_3.MDR_w = d_cache_update(d_cache_3, i, address_to_rw, 0, NULL) & 0xFFFF;;
                    PREGISTER_MEM_WB_3.Aout_w = address_to_rw;
                    break;
                case STUR:
                    d_cache_update(d_cache_3, i, address_to_rw, write_data, NULL);
                    break;
                case STURB:
                    d_cache_update(d_cache_3, i, address_to_rw, write_data & 0xFF, NULL);
                    break;
                case STURH:
                    d_cache_update(d_cache_3, i, address_to_rw, write_data & 0xFFFFFFFF, NULL);
                    break;
            }
        }
    } else if (i == BL) {
        PREGISTER_MEM_WB_3.Aout_w = PREGISTER_EX_MEM_3.nPC_m + 4;
    } else {
        PREGISTER_MEM_WB_3.Aout_w = PREGISTER_EX_MEM_3.Aout_m;
        PREGISTER_MEM_WB_3.write_register_number = PREGISTER_EX_MEM_3.write_register_number;
        if (i == ADDS || i == ANDS || i == SUBS || i == CMP || i == ADDIS || i == SUBIS) {
            PREGISTER_MEM_WB_3.flag_n = PREGISTER_EX_MEM_3.flag_n;
            PREGISTER_MEM_WB_3.flag_z = PREGISTER_EX_MEM_3.flag_z;
            PREGISTER_MEM_WB_3.flag_v = PREGISTER_EX_MEM_3.flag_v;
            PREGISTER_MEM_WB_3.flag_c = PREGISTER_EX_MEM_3.flag_c;
        }
    }
    if (i == HLT) {
        cache_destroy(i_cache_3);
        cache_destroy(d_cache_3);
        RUN_BIT_3 = 0;
    }

    PREGISTER_MEM_WB_3.write_register_number = PREGISTER_EX_MEM_3.write_register_number;
    PREGISTER_MEM_WB_3.i = i;
    PREGISTER_MEM_WB_3.will_write = PREGISTER_EX_MEM_3.will_write;
    PREGISTER_MEM_WB_3.machine_code = PREGISTER_EX_MEM_3.machine_code;
}

/*-------------------------------WRITE BACK-------------------------------*/
void pipe_stage_wb1() {
    instruction i = PREGISTER_MEM_WB_3.i;
    if (i == NO_INSTRUCTION) { return; }
    uint32_t write_register = PREGISTER_MEM_WB_3.write_register_number;
    if (PREGISTER_MEM_WB_3.will_write && write_register != 31 && ((i <= UDIV) || (i <= SUBIS && i >= ADDI) || (i <= LSR && i >= LSL) || (i == MOVZ))) {
        if (write_register == PREGISTER_ID_EX_3.register_Rn1) {
            PREGISTER_ID_EX_3.A_e = PREGISTER_MEM_WB_3.Aout_w;
        }
        if (write_register == PREGISTER_ID_EX_3.register_Rm2) {
            PREGISTER_ID_EX_3.B_e = PREGISTER_MEM_WB_3.Aout_w;
        }
    }
    if ((i == LDUR || i == LDURB || i == LDURH) && write_register < 31) {
        if (write_register == PREGISTER_ID_EX_3.register_Rn1) {
            PREGISTER_ID_EX_3.A_e = PREGISTER_MEM_WB_3.MDR_w;
        }
        if (write_register == PREGISTER_ID_EX_3.register_Rm2) {
            PREGISTER_ID_EX_3.B_e = PREGISTER_MEM_WB_3.MDR_w;
        }
    }
    
    if (((i <= UDIV) || (i <= SUBIS && i >= ADDI) || (i <= LSR && i >= LSL) || (i == MOVZ)) && write_register < 31) {
        // R-Instruction, I-Instruction, Shift-Type, and IW-Type
        CURRENT_STATE_3.REGS[write_register] = PREGISTER_MEM_WB_3.Aout_w;
    } else if ((i == LDUR || i == LDURB || i == LDURH) && write_register < 31){
        CURRENT_STATE_3.REGS[write_register] = PREGISTER_MEM_WB_3.MDR_w;
    } else if (i == BL) {
        CURRENT_STATE_3.REGS[30] = PREGISTER_MEM_WB_3.Aout_w;
    }

    if (i == ADDS || i == ANDS || i == SUBS || i == CMP || i == ADDIS || i == SUBIS) {

        CURRENT_STATE_3.FLAG_N = PREGISTER_MEM_WB_3.flag_n;
        CURRENT_STATE_3.FLAG_Z = PREGISTER_MEM_WB_3.flag_z;
        CURRENT_STATE_3.FLAG_V = PREGISTER_MEM_WB_3.flag_v;
        CURRENT_STATE_3.FLAG_C = PREGISTER_MEM_WB_3.flag_c;
    }
    if ((i == B || i == BL || i == BR) && stat_cycles_3) {
        stat_cycles_3--;
    }
    stat_inst_retire_3++;
   // printf("***************************Cycle number: %d, instruction retired: %d\n", stat_cycles_3, i);
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("WRITEBACK INSTRUCTION\n");
        print_instruction(i, PREGISTER_MEM_WB_3.machine_code);
    }
}