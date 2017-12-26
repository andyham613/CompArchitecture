//Lab by Andy Ham and Bijan Oviedo (aham, boviedo)
/*
 * CMSC 22200
 *
 * ARM pipeline timing simulator
 *
 * Reza Jokar and Gushu Li, 2016
 */
#include <inttypes.h>
#include "pipe.h"
#include "bp.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


#define IS_NEGATIVE(x) (x < 0) ? 1 : 0
#define IS_ZERO(x) (x== 0) ? 1 : 0








/*-------------------------DECODE HELPER FUNCTION------------------------*/

instruction decode(uint32_t machine_code) {
    // R-Format
    uint32_t r_opcode = (machine_code >> 21) & 0x7FF;
    uint32_t r_shamt = (machine_code >> 10) & 0x3F;
    switch (r_opcode) {
        case 0x458:
            return ADD;
        case 0x459:
            return ADD;
        case 0x558:
            return ADDS;
        case 0x559:
            return ADDS;
        case 0x450:
            return AND;
        case 0x750:
            return ANDS;
        case 0x6B0:
            return BR;
        case 0x650:
            return EOR;
        case 0x4D8:
             if (r_shamt != 0x1F) { break;}
             return MUL;
        case 0x550:
            return ORR;
        case 0x4D6:
            if (r_shamt == 0x8) {
                return LSL;
            } else if (r_shamt == 0x9) {
                return LSR;
            } else if (r_shamt == 0x3) {
                return SDIV;
            } else {
                return UDIV;
            }
        case 0x658:
            return SUB;
        case 0x659:
            return SUB;
        case 0x758:
            return (machine_code & 0x1F == 0x1F) ? CMP : SUBS;
        case 0x759:
            return (machine_code & 0x1F == 0x1F) ? CMP : SUBS;
        case 0x69A:
            return (r_shamt != 0x3F) ? LSLI : LSRI;
        case 0x69B:
            return (r_shamt != 0x3F) ? LSLI : LSRI;
        case 0x6a2:
            return HLT;
    }

    // I-Format
    uint32_t i_opcode = (machine_code >> 22) & 0x3FF;
    switch (i_opcode) {
        case 0x244:
            return ADDI;
        case 0x2C4:
            return ADDIS;
        case 0x344:
            return SUBI;
        case 0x3C4:
            return SUBIS;
    }

    // D-format
    uint32_t d_opcode = (machine_code >> 21) & 0x7FF;
    switch(d_opcode) {
        case 0x7C2:
            return LDUR;
        case 0x7C0:
            return STUR;
        case 0x1C2:
            return LDURB;
        case 0x3C2:
            return LDURH;
        case 0x1C0:
            return STURB;
        case 0x3C0:
            return STURH;
        case 0x5C0:
            return STURW;
    }

    // B-Format
    uint32_t b_opcode = (machine_code >> 26) & 0x3F;
    switch(b_opcode) {
        case 0x5:
            return B;
        case 0x25:
            return BL;
    }

    //CB-Format
    uint32_t cb_opcode = (machine_code >> 24) & 0xFF;
    switch(cb_opcode) {
        case 0xb5:
            return CBNZ;
        case 0xb4:
            return CBZ;
        case 0x54:
            switch(machine_code & 0x1F) {
                case 0x0:
                    return BEQ;
                case 0x1:
                    return BNE;
                case 0xa:
                    return BGE;
                case 0xb:
                    return BLT;
                case 0xc:
                    return BGT;
                case 0xd:
                    return BLE;
            }
    }

    //IW-Format
    uint32_t iw_opcode = (machine_code >> 23) & 0x1FF;
    switch (iw_opcode) {
        case 0x1A5:
            return MOVZ;
    }
    return NO_INSTRUCTION;
}

void print_instruction(instruction i, uint32_t machine_code) {
    printf("fetched address: %08x\n", machine_code);
    if (i == NO_INSTRUCTION) {
        printf("NO INSTRUCTION\n");
    }
    if (i <= UDIV) {
        printf("format: R_FORMAT\n");
        printf("type: %d\n", i);
        printf("opcode: %08x\n", (machine_code >> 21) & 0x7FF);
        printf("Rm: %08x\n", (machine_code >> 16) & 0x1F);
        printf("shamt: %08x\n",(machine_code >> 10) & 0x3F);
        printf("Rn: %08x\n", (machine_code >> 5) & 0x1F);
        printf("Rd: %08x\n\n", machine_code & 0x1F);
    } else if (i <= BLE) {
        printf("format: CB_FORMAT\n");
        printf("type: %d\n", i);
        printf("opcode: %08x\n", (machine_code >> 24) & 0xFF);
        printf("COND_BR_address: %08x\n", (machine_code >> 5) & 0x7FFFF);
        printf("Rt: %08x\n\n", machine_code & 0x1F);
    } else if (i <= STURW) {
        printf("format: D_FORMAT\n");
        printf("type: %d\n", i);
        printf("opcode: %08x\n", (machine_code >> 21) & 0x7FF);
        printf("DT_address: %08x\n", (machine_code >> 12) & 0x1FF);
        printf("op: %08x\n", (machine_code >> 10) & 0x3);
        printf("Rn: %08x\n", (machine_code >> 5) & 0x1F);
        printf("Rt: %08x\n\n", machine_code & 0x1F);
    } else if (i <= SUBIS) {
        printf("format: I_FORMAT\n");
        printf("type: %d\n", i);
        printf("opcode: %08x\n", (machine_code >> 22) & 0x3FF);
        printf("ALU_immediate: %08x\n", (machine_code >> 10) & 0xFFF);
        printf("Rn: %08x\n", (machine_code >> 5) & 0x1F);
        printf("Rd: %08x\n\n", machine_code & 0x1F);
    } else if (i <= LSR) {
        printf("format: SHIFT_FORMAT\n");
        printf("type: %d\n", i);
        printf("opcode: %08x\n", (machine_code >> 22) & 0x3FF);
        printf("immr: %08x\n", (machine_code >> 16) & 0x3F);
        printf("imms: %08x\n", (machine_code >> 10) & 0x3F);
        printf("Rn: %08x\n", (machine_code >> 5) & 0x1F);
        printf("Rd: %08x\n\n", machine_code & 0x1F);
    } else if (i <= BL) {
        printf("format: B_FORMAT\n");
        printf("type: %d\n", i);
        printf("opcode: %08x\n", (machine_code >> 26) & 0x3F);
        printf("BR_address: %08x\n\n", machine_code & 0x3FFFFFF);
    } else if (i <= MOVZ) {
        printf("format: IW_FORMAT\n");
        printf("type: %d\n", i);
        printf("opcode: %08x\n", (machine_code >> 23) & 0x1FF);
        printf("op2: %08x\n", (machine_code >> 21) & 0x3);
        printf("MOV_immediate: %08x\n", (machine_code >> 5) & 0xFFFF);
        printf("Rd: %08x\n\n", machine_code & 0x1F);
    } else {
        printf("Halted\n\n");
    }
}

/* ----------------------------GLOBAL VARIABLES-------------------------*/

CPU_State CURRENT_STATE;

// Pipe_Reg_IFtoID PREGISTER_IF_ID;
// Pipe_Reg_IDtoEX PREGISTER_ID_EX;
// Pipe_Reg_EXtoMEM PREGISTER_EX_MEM;
// Pipe_Reg_MEMtoWB PREGISTER_MEM_WB;

void pipe_init()
{
    memset(&CURRENT_STATE, 0, sizeof(CPU_State));
    memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
    memset(&PREGISTER_ID_EX, 0, sizeof(PREGISTER_ID_EX));
    memset(&PREGISTER_EX_MEM, 0, sizeof(PREGISTER_EX_MEM));
    memset(&PREGISTER_MEM_WB, 0, sizeof(PREGISTER_MEM_WB));


    bpt_init();
    CURRENT_STATE.PC = 0x00400000;
}

void pipe_cycle() {
    pipe_stage_wb();
    pipe_stage_mem();
    pipe_stage_execute();
    pipe_stage_decode();
    pipe_stage_fetch();
}

/*---------------------------------FETCH--------------------------------*/

void pipe_stage_fetch()
{
    // printf("Fetch Instruction");
    // print_instruction(decode(mem_read_32(CURRENT_STATE.PC)), mem_read_32(CURRENT_STATE.PC));
    // printf("\n****************************************************************************************************************\n\n");
    if (PREGISTER_EX_MEM.hazard_detected || PREGISTER_EX_MEM.should_squash_fetch) {
        PREGISTER_EX_MEM.hazard_detected = 0;
        PREGISTER_EX_MEM.should_squash_fetch = 0;
        // printf("Hazard Detected in Fetch\n\n");
        return;
    }

    if (PREGISTER_MEM_WB.is_unconditional) {
        bp_predict(CURRENT_STATE.PC, 1);

        // printf("is_unconditional");
        PREGISTER_MEM_WB.is_unconditional--;
        return;
    }

    if (!PREGISTER_IF_ID.did_set_flags) {
        PREGISTER_IF_ID.flag_n = CURRENT_STATE.FLAG_N;
        PREGISTER_IF_ID.flag_z = CURRENT_STATE.FLAG_Z;
        PREGISTER_IF_ID.flag_v = CURRENT_STATE.FLAG_V;
        PREGISTER_IF_ID.flag_n = CURRENT_STATE.FLAG_C;
        PREGISTER_IF_ID.did_set_flags = 0;
    }
    
    PREGISTER_IF_ID.IR_d = mem_read_32(CURRENT_STATE.PC);
    PREGISTER_IF_ID.PC_d = CURRENT_STATE.PC;
    if (PREGISTER_IF_ID.detected_halt != 1) {
        bp_predict(CURRENT_STATE.PC, 0);
        // printf("is conditional");
        // CURRENT_STATE.PC += 4;
    }

    
}

/*--------------------------------DECODE--------------------------------*/

void pipe_stage_decode()
{

    if (PREGISTER_EX_MEM.hazard_detected == 1) {
        // printf("Hazard Detected in Decode\n");
        memset(&PREGISTER_ID_EX,0,sizeof(PREGISTER_ID_EX));
        return;
    }
    uint32_t machine_code = PREGISTER_IF_ID.IR_d;
    instruction i = decode(machine_code);
    // printf("Decode Instruction: ");
    // print_instruction(i, machine_code);
    if (i == NO_INSTRUCTION) { 
        memset(&PREGISTER_ID_EX, 0, sizeof(PREGISTER_ID_EX));
        return;
    }
    // R-Format
    if (i <= UDIV) {
        uint32_t shamt = (machine_code >> 10) & 0x3F;
        uint32_t Rn = (machine_code >> 5) & 0x1F; // Rn
        uint32_t Rm = (machine_code >> 16) & 0x1F; // Rm
        uint32_t shift_bits = (((machine_code) >> 22) & 0x3) << 6;

        PREGISTER_ID_EX.register_Rn1 = Rn;
        PREGISTER_ID_EX.register_Rm2 = Rm;
        PREGISTER_ID_EX.A_e = CURRENT_STATE.REGS[Rn];
        PREGISTER_ID_EX.B_e = CURRENT_STATE.REGS[Rm];
        PREGISTER_ID_EX.Imm_e = shamt | shift_bits;
        PREGISTER_ID_EX.write_register_number = machine_code & 0x1F;

    }
    // CB-Format
    else if (i <= BLE) {
        uint32_t read_register_1 = (machine_code >> 5) & 0x1F; //Rt
        uint32_t COND_BR_address = (machine_code >> 5) & 0x7FFFF;

        PREGISTER_ID_EX.A_e = CURRENT_STATE.REGS[read_register_1];
        PREGISTER_ID_EX.Imm_e = COND_BR_address & 0x00000000FFFFFFFF;
        PREGISTER_ID_EX.write_register_number = 32;
        PREGISTER_ID_EX.register_Rn1 = read_register_1;
        PREGISTER_ID_EX.register_Rm2 = 32;

        // no need for writing to register
    }
    // D-Format
    else if (i <= STURW) {
        uint32_t Rn = (machine_code >> 5) & 0x1F; //Rn
        // printf("Register Rn: %08x\n", Rn);
        uint32_t immediate = (machine_code >> 10) & 0x7FF;
        // the immediate is the dt address and the op in one
        PREGISTER_ID_EX.A_e = CURRENT_STATE.REGS[Rn];
        PREGISTER_ID_EX.Imm_e = immediate;
        
        if (i == LDUR || i == LDURB || i == LDURH) {
            PREGISTER_ID_EX.write_register_number = machine_code & 0x1F;
            PREGISTER_ID_EX.register_Rn1 = Rn;
            PREGISTER_ID_EX.register_Rm2 = 32;
        } else {

            PREGISTER_ID_EX.register_Rn1 = Rn;
            PREGISTER_ID_EX.register_Rm2 = machine_code & 0x1F;
            // printf("Register Rt: %08x\n", machine_code & 0x1F);
            PREGISTER_ID_EX.B_e = CURRENT_STATE.REGS[machine_code & 0x1F];
            PREGISTER_ID_EX.write_register_number = 32;
        }
    }
    // I-Format
    else if (i <= SUBIS) {
        uint32_t Rn = (machine_code >> 5) & 0x1F; // Rn
        uint32_t ALU_immediate = (machine_code >> 10) & 0xFFF;

        PREGISTER_ID_EX.write_register_number = machine_code & 0x1F;
        PREGISTER_ID_EX.register_Rn1 = Rn;
        PREGISTER_ID_EX.register_Rm2 = 32;
        PREGISTER_ID_EX.A_e = CURRENT_STATE.REGS[Rn];
        PREGISTER_ID_EX.Imm_e = ALU_immediate;
    }
    // Shift-Type
    else if (i <= LSR) {

      
        uint32_t Rn = (machine_code >> 5) & 0x1F;
        uint32_t Rm = (machine_code >> 16) & 0x1F;
        uint32_t immediate = (machine_code >> 16) & 0x3F;
        // immediate for shift format is Rm + shamt of R format
       
        PREGISTER_ID_EX.A_e = CURRENT_STATE.REGS[Rn];
        if (i == LSL || i == LSR) {
            PREGISTER_ID_EX.B_e = CURRENT_STATE.REGS[Rm];
        }
        
        PREGISTER_ID_EX.Imm_e = immediate;

        PREGISTER_ID_EX.register_Rn1 = Rn;
        PREGISTER_ID_EX.register_Rm2 = Rm;
        PREGISTER_ID_EX.write_register_number = machine_code & 0x1F;
    }
    // B-Format
    else if (i <= BL) {
        uint32_t immediate = machine_code & 0x3FFFFFF;
        PREGISTER_ID_EX.Imm_e = immediate;
        // no need for write register
        PREGISTER_ID_EX.write_register_number = 32;
        PREGISTER_ID_EX.register_Rn1 = 32;
        PREGISTER_ID_EX.register_Rm2 = 32;

        uint32_t BR_address = PREGISTER_ID_EX.Imm_e;
        int64_t BR_address64 = (BR_address >> 25) ? (0xFFFFFFFFF0000000 | ((int64_t)BR_address << 2)) : 0 | (BR_address << 2);
        
        int nPC_m = PREGISTER_IF_ID.PC_d + BR_address64;
        int is_correct_branch;
        bp_update(PREGISTER_IF_ID.PC_d, i, nPC_m,1,&is_correct_branch);
        PREGISTER_MEM_WB.is_unconditional = 1;
        CURRENT_STATE.PC = nPC_m;
        // printf("PC SET IN DECODE\n");
        memset(&PREGISTER_IF_ID,0, sizeof(PREGISTER_IF_ID));

    }
    //IW-Format
    else if (i <= MOVZ) {
        uint32_t MOV_immediate = (machine_code >> 5) & 0x3FFFF;
        PREGISTER_ID_EX.Imm_e = MOV_immediate;
        PREGISTER_ID_EX.write_register_number = machine_code & 0x1F;
    } else {
        //if HLT
        
        CURRENT_STATE.PC += 4;
        // printf("PC SET IN DECODE HALT\n");
        memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
        PREGISTER_IF_ID.detected_halt = 1;
        // PREGISTER_IF_ID.halt_detected = 1;
    }

    PREGISTER_ID_EX.flag_n = PREGISTER_IF_ID.flag_n;
    PREGISTER_ID_EX.flag_z = PREGISTER_IF_ID.flag_z;
    PREGISTER_ID_EX.flag_v = PREGISTER_IF_ID.flag_v;
    PREGISTER_ID_EX.flag_c = PREGISTER_IF_ID.flag_c;

    PREGISTER_ID_EX.PC_e = PREGISTER_IF_ID.PC_d;
    
    PREGISTER_ID_EX.i = i;
    PREGISTER_ID_EX.machine_code = machine_code;

}

/*--------------------------------EXECUTE--------------------------------*/

void pipe_stage_execute_r_instruction(instruction i) {
    uint32_t shamt = (PREGISTER_ID_EX.Imm_e & 0x3F);
    uint32_t shift = ((PREGISTER_ID_EX.Imm_e >> 6) & 0x3);

    int64_t op1 = PREGISTER_ID_EX.A_e;  //1st register source operand Rn's value
    int64_t op2 = PREGISTER_ID_EX.B_e;  //2nd register source operand Rm's value

    switch (i) {
        case ADD:
            PREGISTER_EX_MEM.Aout_m = op1 + op2;
            break;
        case ADDS:
            PREGISTER_EX_MEM.flag_n = IS_NEGATIVE(op1 + op2);
            PREGISTER_EX_MEM.flag_z = IS_ZERO(op1 + op2);
            PREGISTER_EX_MEM.flag_c = (op1 + op2 < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM.flag_v = ((op1 > 0 && op2 > 0 && op1 + op2 < op1) || (op1 < 0 && op2 < 0 && op1 + op2 > 0)) ? 1 : 0;

            PREGISTER_EX_MEM.Aout_m = op1 + op2;
            break;
        case AND:
            if (shift == 0) { // LSL
                 PREGISTER_EX_MEM.Aout_m = op1 & (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 PREGISTER_EX_MEM.Aout_m = op1 & ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if (shift == 2) {  // ASR
                 PREGISTER_EX_MEM.Aout_m = op1 & (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 PREGISTER_EX_MEM.Aout_m = op1 & (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }
            break;
        case ANDS:
            PREGISTER_EX_MEM.flag_c = 0;
            PREGISTER_EX_MEM.flag_v = 0;
            PREGISTER_EX_MEM.flag_z = IS_ZERO(op1 & op2);
            PREGISTER_EX_MEM.flag_n = IS_NEGATIVE(op1 & op2);

            if (shift == 0) { // LSL
                 PREGISTER_EX_MEM.Aout_m = op1 & (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 PREGISTER_EX_MEM.Aout_m = op1 & ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if ( shift == 2) {  // ASR
                 PREGISTER_EX_MEM.Aout_m = op1 & (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 PREGISTER_EX_MEM.Aout_m = op1 & (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }  
            break;
        case BR:
            PREGISTER_EX_MEM.nPC_m = op1;
            break;
        case EOR: ;
            if (shift == 0) { // LSL
                 PREGISTER_EX_MEM.Aout_m = op1 ^ (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 PREGISTER_EX_MEM.Aout_m = op1 ^ ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if (shift == 2) {  // ASR
                 PREGISTER_EX_MEM.Aout_m = op1 ^ (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 PREGISTER_EX_MEM.Aout_m = op1 ^ (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }
            break;
        case MUL:
            PREGISTER_EX_MEM.Aout_m = op1 * op2;
            break;
        case ORR:
            if (shift == 0) { // LSL
                 PREGISTER_EX_MEM.Aout_m = op1 | (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 PREGISTER_EX_MEM.Aout_m = op1 | ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if ( shift == 2) {  // ASR
                 PREGISTER_EX_MEM.Aout_m = op1 | (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 PREGISTER_EX_MEM.Aout_m = op1 | (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64 - shamt)));
            }
            break;
        case SDIV:
            PREGISTER_EX_MEM.Aout_m = (op2 != 0) ? ((int64_t) ((double) op1 / (double) op2)) : 0;
            break;
        case SUB:
            PREGISTER_EX_MEM.Aout_m = op1 - op2;
            break;
        case SUBS:

            PREGISTER_EX_MEM.flag_n = IS_NEGATIVE(op1 - op2);
            PREGISTER_EX_MEM.flag_z = IS_ZERO(op1 - op2);
            PREGISTER_EX_MEM.flag_c = (op1 - op2 < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM.flag_v = ((op1 > 0 && op2 < 0 && op1 - op2 < op1) || (op1 < 0 && op2 < 0 && op1 - op2 > 0)) ? 1 : 0;

            PREGISTER_EX_MEM.Aout_m = op1 - op2;
            break;
        case CMP:
            PREGISTER_EX_MEM.flag_n = IS_NEGATIVE(op1 - op2);
            PREGISTER_EX_MEM.flag_z = IS_ZERO(op1 - op2);
            PREGISTER_EX_MEM.flag_c = (op1 - op2 < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM.flag_v = ((op1 > 0 && op2 < 0 && op1 - op2 < op1) || (op1 < 0 && op2 < 0 && op1 - op2 > 0)) ? 1 : 0;

            PREGISTER_EX_MEM.Aout_m = op1 - op2;

            break;
        case UDIV:
            PREGISTER_EX_MEM.Aout_m = (op2 != 0) ? ((int64_t) ((unsigned long long) op1 / (unsigned long long) op2)) : 0;
            break;
    }

    PREGISTER_EX_MEM.will_write = 1;
    PREGISTER_EX_MEM.write_register_number = PREGISTER_ID_EX.write_register_number;

}
           

void pipe_stage_execute_cb_instruction(instruction i) {

    int64_t current_state_pc = PREGISTER_ID_EX.PC_e;
    int64_t value_in_reg_rt = PREGISTER_ID_EX.A_e;
    uint32_t COND_BR_address = PREGISTER_ID_EX.Imm_e;

    int64_t CondBranchAddr = ((COND_BR_address >> 18) != 0) ? (COND_BR_address << 2) | 0xFFFFFFFFFFE00000 : (CondBranchAddr = 0 | (COND_BR_address << 2));
    int64_t branch_address = current_state_pc + CondBranchAddr;
    int64_t next_address = current_state_pc + 4;

    uint32_t N = PREGISTER_ID_EX.flag_n;
    uint32_t Z = PREGISTER_ID_EX.flag_z;
    uint32_t V = PREGISTER_ID_EX.flag_v;
    uint32_t C = PREGISTER_ID_EX.flag_c;

    switch(i) {
        case CBNZ:
            PREGISTER_EX_MEM.nPC_m = (value_in_reg_rt != 0) ? branch_address : next_address;
            break;
        case CBZ:
            PREGISTER_EX_MEM.nPC_m = (value_in_reg_rt == 0) ? branch_address : next_address;
            break;
        case BEQ:
            PREGISTER_EX_MEM.nPC_m = (Z == 1) ? branch_address : next_address;
            break;
        case BNE:
            PREGISTER_EX_MEM.nPC_m = (Z == 0) ? branch_address : next_address;
            break;
        case BGT:
            PREGISTER_EX_MEM.nPC_m = (Z == 0 && N == V) ? branch_address : next_address;
            break;
        case BLT:
            PREGISTER_EX_MEM.nPC_m = (N != V) ? branch_address : next_address;
            break;
        case BGE:
            PREGISTER_EX_MEM.nPC_m = (N == V) ? branch_address : next_address;
            break;
        case BLE:
            PREGISTER_EX_MEM.nPC_m = (!(Z == 0 && N == V)) ? branch_address : next_address;
            break;
    }

    PREGISTER_EX_MEM.will_write = 0;
}

void pipe_stage_execute_d_instruction(instruction i) {
    uint32_t DT_address = ((PREGISTER_ID_EX.Imm_e >> 2) & 0x1FF);
    uint32_t Rn_value = PREGISTER_ID_EX.A_e;
    int64_t address_to_rw = Rn_value + DT_address;

    PREGISTER_EX_MEM.Aout_m = address_to_rw;
    PREGISTER_EX_MEM.nPC_m = PREGISTER_ID_EX.PC_e;
    PREGISTER_EX_MEM.B_m = PREGISTER_ID_EX.B_e;
    PREGISTER_EX_MEM.will_write = 0;
}

void pipe_stage_execute_i_instruction(instruction i) {
    int64_t immediate = PREGISTER_ID_EX.Imm_e;
    int64_t op1 = PREGISTER_ID_EX.A_e; //Rn 1st register source operand

    switch(i) {
        case ADDI:
            PREGISTER_EX_MEM.Aout_m = op1 + immediate;
            break;
        case ADDIS:
            PREGISTER_EX_MEM.flag_c = (op1 + immediate < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM.flag_n = IS_NEGATIVE(op1 + immediate);
            PREGISTER_EX_MEM.flag_z = IS_ZERO(op1 + immediate);
            PREGISTER_EX_MEM.flag_v = ((op1 > 0 && immediate > 0 && op1 + immediate < op1) || (op1 < 0 && immediate < 0 && op1 + immediate > 0)) ? 1 : 0;

            PREGISTER_EX_MEM.Aout_m = op1 + immediate;
            break;
        case SUBI:
            PREGISTER_EX_MEM.Aout_m = op1 - immediate;
            break;
        case SUBIS:
            PREGISTER_EX_MEM.flag_c = (op1 - immediate < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            PREGISTER_EX_MEM.flag_n = IS_NEGATIVE(op1 - immediate);
            PREGISTER_EX_MEM.flag_z = IS_ZERO(op1 - immediate);
            PREGISTER_EX_MEM.flag_v = ((op1 > 0 && immediate < 0 && op1 - immediate < 0) || (op1 < 0 && immediate > 0 && op1 - immediate > 0)) ? 1 : 0;

            PREGISTER_EX_MEM.Aout_m = op1 - immediate;
            break;
    }
    PREGISTER_EX_MEM.will_write = 1;

}

void pipe_stage_execute_shift_instruction(instruction i) {

    uint32_t immr = PREGISTER_ID_EX.Imm_e; // for immediate shifts // (machine_code >> 16) & 0x3F;
    int64_t op1 = PREGISTER_ID_EX.A_e; //Rn
    int64_t op2 = PREGISTER_ID_EX.B_e; // Rm for shift registers
    uint32_t shamt;
    if (i == LSLI) {
        shamt = (immr ^ 0x3F) + 1;
    } else { // if LSRI
        shamt = immr;
    }

    switch(i) { 
        case LSL:
            PREGISTER_EX_MEM.Aout_m = op1 << op2;
            break;
        case LSR:
            PREGISTER_EX_MEM.Aout_m = op1 >> op2;
            break;
        case LSLI:
            PREGISTER_EX_MEM.Aout_m = op1 << shamt;
            break;
        case LSRI:
            PREGISTER_EX_MEM.Aout_m = op1 >> shamt;
            break;
    }

    PREGISTER_EX_MEM.will_write = 1;
}

void pipe_stage_execute_b_instruction(instruction i) {
   
    uint32_t BR_address = PREGISTER_ID_EX.Imm_e;
    int64_t BR_address64 = (BR_address >> 25) ? (0xFFFFFFFFF0000000 | ((int64_t)BR_address << 2)) : 0 | (BR_address << 2);

    switch(i) {
        case B:
            PREGISTER_EX_MEM.nPC_m = PREGISTER_ID_EX.PC_e + BR_address64;
            return;
        case BL:
            PREGISTER_EX_MEM.nPC_m = PREGISTER_ID_EX.PC_e + BR_address64;
            return;
    }

    PREGISTER_EX_MEM.will_write = 0;

}

void pipe_stage_execute_iw_instruction(instruction i) {

    uint32_t MOV_immediate = PREGISTER_ID_EX.Imm_e & 0xFFFF;
    uint32_t op2 = (PREGISTER_ID_EX.Imm_e >> 16) & 0x3;
    if (i == MOVZ) {
        PREGISTER_EX_MEM.Aout_m = MOV_immediate << (op2 * 16);
    }
    PREGISTER_EX_MEM.will_write = 1;
}

void pipe_stage_execute() {

    if (PREGISTER_EX_MEM.hazard_detected == 1) {
        // printf("Hazard Detected in Execute\n");
        return;
    }
    instruction i = PREGISTER_ID_EX.i;
    // printf("Execute Instruction:\n");
    // print_instruction(i,PREGISTER_ID_EX.machine_code);
    if (i == NO_INSTRUCTION) {
        memset(&PREGISTER_EX_MEM,0,sizeof(PREGISTER_EX_MEM));
        return;
    } else if (i <= UDIV) { // R-Format
        pipe_stage_execute_r_instruction(i);
    } else if (i <= BLE) { // CB Format
        pipe_stage_execute_cb_instruction(i);
    } else if (i <= STURW) { // D Format //IMM has DT addr + op2
        pipe_stage_execute_d_instruction(i);
    } else if (i <= SUBIS) { //I Format
        pipe_stage_execute_i_instruction(i);
    } else if (i <= LSR) { // Shift Format
        pipe_stage_execute_shift_instruction(i);
    } else if (i <= BL) { // B Format
        pipe_stage_execute_b_instruction(i);
    } else if (i <= MOVZ) { // IW Format
        pipe_stage_execute_iw_instruction(i);
    }

    if (i == ADDS || i == ANDS || i == SUBS || i == CMP || i == ADDIS || i == SUBIS) {
        PREGISTER_IF_ID.did_set_flags = 1;
        PREGISTER_ID_EX.flag_n = PREGISTER_EX_MEM.flag_n;
        PREGISTER_IF_ID.flag_n = PREGISTER_EX_MEM.flag_n;
        PREGISTER_ID_EX.flag_z = PREGISTER_EX_MEM.flag_z;
        PREGISTER_IF_ID.flag_z = PREGISTER_EX_MEM.flag_z;
        PREGISTER_ID_EX.flag_c = PREGISTER_EX_MEM.flag_c;
        PREGISTER_IF_ID.flag_c = PREGISTER_EX_MEM.flag_c;
        PREGISTER_ID_EX.flag_v = PREGISTER_EX_MEM.flag_v;
        PREGISTER_IF_ID.flag_v = PREGISTER_EX_MEM.flag_v;
    }

    if (i <= BLE && i >= CBNZ) {
        int is_correct_branch;
        if (PREGISTER_EX_MEM.nPC_m != PREGISTER_ID_EX.PC_e + 4) {
            bp_update(PREGISTER_ID_EX.PC_e, i, PREGISTER_EX_MEM.nPC_m,1, &is_correct_branch);
            if (is_correct_branch) {
                // printf("CORRECT BRANCH OCCURRED. GOOD JOB!: 0x%" PRIx64"\n", CURRENT_STATE.PC);
            } else {
                CURRENT_STATE.PC = PREGISTER_EX_MEM.nPC_m;
                PREGISTER_EX_MEM.should_squash_fetch = 1;
                // printf("INCORRECT BRANCH OCCURRED. GOOD JOB!: 0x%" PRIx64"\n", CURRENT_STATE.PC);
                memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
                memset(&PREGISTER_ID_EX,0, sizeof(PREGISTER_ID_EX));
            }
            
            
        } else {
            bp_update(PREGISTER_ID_EX.PC_e, i, PREGISTER_EX_MEM.nPC_m,0, &is_correct_branch);
            
            // printf("PC SET\n");
            // printf("Address to jump to: 0x%" PRIx64 "\t", PREGISTER_EX_MEM.nPC_m);
            // printf("next address: 0x%" PRIx64 "\n", PREGISTER_ID_EX.PC_e + 4);
            CURRENT_STATE.PC = PREGISTER_EX_MEM.nPC_m;
            if (!is_correct_branch) {
                // printf("not correct branch\n");
                PREGISTER_EX_MEM.should_squash_fetch = 1;
                memset(&PREGISTER_ID_EX, 0, sizeof(PREGISTER_IF_ID));
                memset(&PREGISTER_IF_ID,0, sizeof(PREGISTER_IF_ID));
            }
            // memset(&PREGISTER_ID_EX, 0, sizeof(PREGISTER_IF_ID));
            //     memset(&PREGISTER_IF_ID,0, sizeof(PREGISTER_IF_ID));
        }
    } else if (i == BR || i == B || i == BL) {
       // bp_update(PREGISTER_ID_EX.PC_e, i, PREGISTER_EX_MEM.nPC_m,1, NULL);
        // printf("BR/B/BL Instruction\n");
        //stat_inst_retire++;
        // PREGISTER_MEM_WB.is_unconditional = 1;
        // if (!is_correct_branch) {
        //     CURRENT_STATE.PC
        // }
        //CURRENT_STATE.PC = PREGISTER_EX_MEM.nPC_m;
    }

    PREGISTER_EX_MEM.write_register_number = PREGISTER_ID_EX.write_register_number;

    PREGISTER_EX_MEM.i = i;

    PREGISTER_EX_MEM.machine_code = PREGISTER_ID_EX.machine_code;


}

/*------------------------------MEMORY ACCESS------------------------------*/
void pipe_stage_mem()
{
    instruction i = PREGISTER_EX_MEM.i;
    // printf("Mem Instruction:\n");
    // print_instruction(i, PREGISTER_EX_MEM.machine_code);

    if (i == NO_INSTRUCTION) { 
        memset(&PREGISTER_MEM_WB,0,sizeof(PREGISTER_MEM_WB));
        return; 
    }
    if (PREGISTER_EX_MEM.is_previous_load) {
        PREGISTER_EX_MEM.is_previous_load = 0;
        stat_inst_retire--;
        return;
    }
    if (PREGISTER_EX_MEM.will_write) {
        if (PREGISTER_EX_MEM.write_register_number < 31) {

            if (PREGISTER_EX_MEM.write_register_number == PREGISTER_ID_EX.register_Rn1) {
                PREGISTER_ID_EX.A_e = PREGISTER_EX_MEM.Aout_m;
            }
            if (PREGISTER_EX_MEM.write_register_number == PREGISTER_ID_EX.register_Rm2) {
                PREGISTER_ID_EX.B_e = PREGISTER_EX_MEM.Aout_m;
            }
        }
    }

    if (i == LDUR || i == LDURH || i == LDURB) {
        if (PREGISTER_EX_MEM.write_register_number == PREGISTER_ID_EX.register_Rn1 ||
            PREGISTER_EX_MEM.write_register_number == PREGISTER_ID_EX.register_Rm2) {
            // stall the pipeline
            PREGISTER_EX_MEM.is_previous_load = 1;
            PREGISTER_EX_MEM.hazard_detected = 1;
        }
    }
    
    if (i <= STURW && i >= LDUR) {// if load or store
        int64_t address_to_rw = PREGISTER_EX_MEM.Aout_m;
        int64_t write_data = PREGISTER_EX_MEM.B_m;
        uint32_t half_byte_1 = 0, half_byte_2 = 0;
        int64_t full_value = 0;
        switch(i) {
            case LDUR: ;
                half_byte_1 = mem_read_32(address_to_rw);
                half_byte_2 = mem_read_32(address_to_rw + 0x4);
                full_value = half_byte_1 | ((int64_t) half_byte_2 << 32);
                PREGISTER_MEM_WB.MDR_w = full_value;
                PREGISTER_MEM_WB.Aout_w = address_to_rw;
                // printf("LDUR address read: %" PRId64 "\n", full_value);
                break;
            case LDURB: ;
                full_value = mem_read_32(address_to_rw) & 0xFF;
                PREGISTER_MEM_WB.MDR_w = full_value;
                PREGISTER_MEM_WB.Aout_w = address_to_rw;
                break;
            case LDURH: ;
                full_value = mem_read_32(address_to_rw) & 0xFFFF;
                PREGISTER_MEM_WB.MDR_w = full_value;
                PREGISTER_MEM_WB.Aout_w = address_to_rw;
                break;
            case STUR: ;
                half_byte_1 = write_data & 0xFFFFFFFF;
                half_byte_2 = (write_data & 0xFFFFFFFF00000000) >> 32;
                // printf("The first half value to store is: %08x\n", half_byte_1);
                // printf("The second half value to store is: %08x\n", half_byte_2);
                // printf("THe address to store it at is: 0x%" PRIx64 "\n", address_to_rw);
                mem_write_32(address_to_rw, half_byte_1);
                mem_write_32(address_to_rw + 0x4, half_byte_2);
                break;
            case STURB:
                // printf("The value to store is: %d\n", ((uint32_t) write_data) & 0xFF);
                // printf("THe address to store it at is: 0x%" PRIx64 "\n", address_to_rw);
                mem_write_32(address_to_rw, ((uint32_t) write_data) & 0xFF);
                break;
            case STURH:
                mem_write_32(address_to_rw, ((uint32_t) write_data) & 0xFFFF);
                break;
            case STURW:
                mem_write_32(address_to_rw, ((uint32_t) write_data) & 0xFFFFFFFF);
                break;
        }
    } else if (i == BL) {
        PREGISTER_MEM_WB.Aout_w = PREGISTER_EX_MEM.nPC_m + 4;
    } else {
        PREGISTER_MEM_WB.Aout_w = PREGISTER_EX_MEM.Aout_m;
        PREGISTER_MEM_WB.write_register_number = PREGISTER_EX_MEM.write_register_number;
        if (i == ADDS || i == ANDS || i == SUBS || i == CMP || i == ADDIS || i == SUBIS) {
            // printf("MEM: FLAG N: %d\n", PREGISTER_EX_MEM.flag_n);
            // printf("MEM: FLAG Z: %d\n", PREGISTER_EX_MEM.flag_z);
            // printf("MEM: FLAG V: %d\n", PREGISTER_EX_MEM.flag_v);
            // printf("MEM: FLAG C: %d\n", PREGISTER_EX_MEM.flag_c);
            PREGISTER_MEM_WB.flag_n = PREGISTER_EX_MEM.flag_n;
            PREGISTER_MEM_WB.flag_z = PREGISTER_EX_MEM.flag_z;
            PREGISTER_MEM_WB.flag_v = PREGISTER_EX_MEM.flag_v;
            PREGISTER_MEM_WB.flag_c = PREGISTER_EX_MEM.flag_c;
        }
    }
    if (i == HLT) {
        RUN_BIT = 0;
    }

    PREGISTER_MEM_WB.write_register_number = PREGISTER_EX_MEM.write_register_number;
    PREGISTER_MEM_WB.i = i;
    PREGISTER_MEM_WB.will_write = PREGISTER_EX_MEM.will_write;
    PREGISTER_MEM_WB.machine_code = PREGISTER_EX_MEM.machine_code;
}

/*-------------------------------WRITE BACK-------------------------------*/
void pipe_stage_wb() {

    instruction i = PREGISTER_MEM_WB.i;
    // printf("WriteBack Instruction");
    // print_instruction(i, PREGISTER_MEM_WB.machine_code);
    if (i == NO_INSTRUCTION) { return; }

    uint32_t write_register = PREGISTER_MEM_WB.write_register_number;

    if (PREGISTER_MEM_WB.will_write && write_register != 31 && ((i <= UDIV) || (i <= SUBIS && i >= ADDI) || (i <= LSR && i >= LSL) || (i == MOVZ))) {
         //if (!(PREGISTER_EX_MEM.will_write && PREGISTER_EX_MEM.write_register_number < 31 && (PREGISTER_EX_MEM.write_register_number != PREGISTER_ID_EX.register_Rn1))) {
        if (write_register == PREGISTER_ID_EX.register_Rn1) {
            PREGISTER_ID_EX.A_e = PREGISTER_MEM_WB.Aout_w;
        }
        //}
        // if (!(PREGISTER_EX_MEM.will_write && PREGISTER_EX_MEM.write_register_number < 31 && (PREGISTER_EX_MEM.write_register_number != PREGISTER_ID_EX.register_Rm2))) {
        if (write_register == PREGISTER_ID_EX.register_Rm2) {
            PREGISTER_ID_EX.B_e = PREGISTER_MEM_WB.Aout_w;
        }
        //}
    }

    if ((i == LDUR || i == LDURB || i == LDURH) && write_register < 31) {
        if (write_register == PREGISTER_ID_EX.register_Rn1) {
            // printf("Detected a Forward Load\n");
            // printf("The value in Aout_w is: %" PRId64 "\n", PREGISTER_MEM_WB.MDR_w);
            PREGISTER_ID_EX.A_e = PREGISTER_MEM_WB.MDR_w;
        }
        if (write_register == PREGISTER_ID_EX.register_Rm2) {
            PREGISTER_ID_EX.B_e = PREGISTER_MEM_WB.MDR_w;
        }
    }
    

    if (((i <= UDIV) || (i <= SUBIS && i >= ADDI) || (i <= LSR && i >= LSL) || (i == MOVZ)) && write_register < 31) {
        // R-Instruction, I-Instruction, Shift-Type, and IW-Type
        CURRENT_STATE.REGS[write_register] = PREGISTER_MEM_WB.Aout_w;
    } else if ((i == LDUR || i == LDURB || i == LDURH) && write_register < 31){
        CURRENT_STATE.REGS[write_register] = PREGISTER_MEM_WB.MDR_w;
    } else if (i == BL) {
        CURRENT_STATE.REGS[30] = PREGISTER_MEM_WB.Aout_w;
    } 

    if (i == ADDS || i == ANDS || i == SUBS || i == CMP || i == ADDIS || i == SUBIS) {
        // printf("MEM: FLAG N: %d\n", PREGISTER_MEM_WB.flag_n);
        // printf("MEM: FLAG Z: %d\n", PREGISTER_MEM_WB.flag_z);
        // printf("MEM: FLAG V: %d\n", PREGISTER_MEM_WB.flag_v);
        // printf("MEM: FLAG C: %d\n", PREGISTER_MEM_WB.flag_c);
        CURRENT_STATE.FLAG_N = PREGISTER_MEM_WB.flag_n;
        CURRENT_STATE.FLAG_Z = PREGISTER_MEM_WB.flag_z;
        CURRENT_STATE.FLAG_V = PREGISTER_MEM_WB.flag_v;
        CURRENT_STATE.FLAG_C = PREGISTER_MEM_WB.flag_c;
    }

     if ((i == B || i ==BL || i == BR) && stat_cycles) {
        stat_cycles--;
         // printf("\n\n\n\n----------------------INCREMENTING COUNTER---------------------------\n\n\n\n");
         // exit(1);
     }

    stat_inst_retire++;

}