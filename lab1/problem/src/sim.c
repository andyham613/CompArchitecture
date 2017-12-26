//CMSC 22200 LAB 1 BY: ANDY HAM, BIJAN OVIEDO
#include <stdio.h>
#include <stdlib.h>
#include "shell.h"

#define IS_NEGATIVE(x) (x < 0) ? 1 : 0
#define IS_ZERO(x) (x== 0) ? 1 : 0

uint32_t fetch() {
    return mem_read_32(CURRENT_STATE.PC);
}

uint32_t decode(uint32_t machine_code){
    // R-Format
    uint32_t r_op, i_op, d_op, b_op, cb_op, iw_op;
    r_op = (machine_code >> 21) & 0x7FF;
    i_op = (machine_code >> 22) & 0x3FF;
    d_op = (machine_code >> 21) & 0x7FF;
    b_op = (machine_code >> 26) & 0x3F;
    cb_op = (machine_code >> 24) & 0xFF;
    iw_op = (machine_code >> 23) & 0x1FF;

    uint32_t r_shift = (machine_code >> 10) & 0x3F;
    switch (r_op) {
        case 0x458:
            return 0;
        case 0x459:
            return 0;
        case 0x558:
            return 1;
        case 0x559:
            return 1;
        case 0x450:
            return 2;
        case 0x750:
            return 3;
        case 0x6B0:
            return 4;
        case 0x650:
            return 5;
        case 0x4D8:
             if (r_shift != 0x1F) { break;}
             return 6;
        case 0x550:
            return 7;
        case 0x4D6:
            return (r_shift == 0x3) ? 8 : 12;
        case 0x658:
            return 9;
        case 0x659:
            return 9;
        case 0x758:
            return 10;
        case 0x759:
            return 10;
        case 0x69A:
            return (r_shift != 0x3F) ? 32 : 33;
        case 0x69B:
            return (r_shift != 0x3F) ? 32 : 33;
        case 0x6a2:
            return 37;
    }

    // I-Format
    switch (i_op) {
        case 0x244:
            return 28;
        case 0x2C4:
            return 29;
        case 0x344:
            return 30;
        case 0x3C4:
            return 31;
    }

    //  D-format
    switch(d_op) {
        case 0x7C2:
            return 21;
        case 0x7C0:
            return 22;
        case 0x1C2:
            return 23;
        case 0x3C2:
            return 24;
        case 0x1C0:
            return 25;
        case 0x3C0:
            return 26;
        case 0x5C0:
            return 27;
    }

    // B-Format
    switch(b_op) {
        case 0x5:
            return 34;
        case 0x25:
            return 35;
    }

    //CB-Format
    switch(cb_op) {
        case 0xb5:
            return 13;
        case 0xb4:
            return 14;
        case 0x54:
            switch(machine_code & 0x1F) {
                case 0x0:
                    return 15;
                case 0x1:
                    return 16;
                case 0xa:
                    return 19;
                case 0xb:
                    return 18;
                case 0xc:
                    return 17;
                case 0xd:
                    return 20;
            }
    }

    //IW-Format
    switch (iw_op) {
        case 0x1A5:
            return 36;
    }
    return 0;
}

void execute_r_instruction(uint32_t instruction, uint32_t machine_code) {
    uint32_t shamt, Rm, Rn, Rd;
    shamt = (machine_code >> 10) & 0x3F;
    Rm = (machine_code >> 16) & 0x1F;
    Rn = (machine_code >> 5) & 0x1F;
    Rd = machine_code & 0x1F;

    uint32_t shift = (machine_code >> 22) & 0x3;

    int64_t op1 = CURRENT_STATE.REGS[Rn], op2 = CURRENT_STATE.REGS[Rm];

    switch (instruction) {
        case 0:
            NEXT_STATE.REGS[Rd] = op1 + op2;
            break;
        case 1:
            NEXT_STATE.FLAG_N = IS_NEGATIVE(op1 + op2);
            NEXT_STATE.FLAG_Z = IS_ZERO(op1 + op2);
            NEXT_STATE.FLAG_C = (op1 + op2 < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            NEXT_STATE.FLAG_V = ((op1 > 0 && op2 > 0 && op1 + op2 < op1) || (op1 < 0 && op2 < 0 && op1 + op2 > 0)) ? 1 : 0;

            NEXT_STATE.REGS[Rd] = op1 + op2;
            break;
        case 2:
            if (shift == 0) { // LSL
                 NEXT_STATE.REGS[Rd] = op1 & (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 NEXT_STATE.REGS[Rd] = op1 & ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if ( shift == 2) {  // ASR
                 NEXT_STATE.REGS[Rd] = op1 & (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 NEXT_STATE.REGS[Rd] = op1 & (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }
            break;
        case 3:
            NEXT_STATE.FLAG_C = 0;
            NEXT_STATE.FLAG_V = 0;
            NEXT_STATE.FLAG_Z = IS_ZERO(op1 & op2);
            NEXT_STATE.FLAG_N = IS_NEGATIVE(op1 & op2);

            if (shift == 0) { // LSL
                 NEXT_STATE.REGS[Rd] = op1 & (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 NEXT_STATE.REGS[Rd] = op1 & ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if ( shift == 2) {  // ASR
                 NEXT_STATE.REGS[Rd] = op1 & (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 NEXT_STATE.REGS[Rd] = op1 & (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }
            break;

            NEXT_STATE.REGS[Rd] = op1 & op2;
        case 4:
            NEXT_STATE.PC = op1 - 4;
            break;
        case 5:
            if (shift == 0) { // LSL
                 NEXT_STATE.REGS[Rd] = op1 ^ (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 NEXT_STATE.REGS[Rd] = op1 ^ ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if ( shift == 2) {  // ASR
                 NEXT_STATE.REGS[Rd] = op1 ^ (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 NEXT_STATE.REGS[Rd] = op1 ^ (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }
            break;
        case 6:
            NEXT_STATE.REGS[Rd] = op1 * op2;
            break;
        case 7:
            if (shift == 0) { // LSL
                 NEXT_STATE.REGS[Rd] = op1 | (op2 << shamt);
            } else if (shift == 1) {  // LSR
                 NEXT_STATE.REGS[Rd] = op1 | ((op2 >> shamt) & ~(0x8000000000000000 >> shamt));
            } else if ( shift == 2) {  // ASR
                 NEXT_STATE.REGS[Rd] = op1 | (op2 >> shamt);
            } else { // ROR rotate right. bits on right side move over to left side
                 NEXT_STATE.REGS[Rd] = op1 | (((op2 >> shamt) & ~(0x8000000000000000 >> shamt)) | (op2 << (64-shamt)));
            }
            break;
        case 8:
            if (op2) {
                double real = (double) op1 / (double) op2;
                NEXT_STATE.REGS[Rd] = (int64_t) real;
            } else {
                NEXT_STATE.REGS[Rd] = 0;
            }
            break;
        case 9:
            NEXT_STATE.REGS[Rd] = op1 - op2;
            break;
        case 10:
            NEXT_STATE.FLAG_N = IS_NEGATIVE(op1 - op2);
            NEXT_STATE.FLAG_Z = IS_ZERO(op1 - op2);
            NEXT_STATE.FLAG_C = (op1 - op2 < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            NEXT_STATE.FLAG_V = ((op1 > 0 && op2 < 0 && op1 - op2 < 0) || (op1 < 0 && op2 > 0 && op1 - op2 > 0)) ? 1 : 0;

            if (Rd != 0x1F) {
                // If SUBS instead of CMP
                NEXT_STATE.REGS[Rd] = op1 - op2;
            }
            break;
        case 12:
            if (op2 != 0) {
                double unsigned_quotient = (unsigned long long) op1 / (unsigned long long) op2;
                NEXT_STATE.REGS[Rd] = (int64_t) unsigned_quotient;
            } else {
                NEXT_STATE.REGS[Rd] = 0;
            }
    }
    NEXT_STATE.PC += 4;
}

void execute_shift_instruction(uint32_t instruction, uint32_t machine_code) {
    uint32_t immr, imms, Rn, Rd;
    immr = (machine_code >> 16) & 0x3F;
    imms = (machine_code >> 10) & 0x3F;
    Rn = (machine_code >> 5) & 0x1F;
    Rd = machine_code & 0x1F;
    uint32_t shamt = (~immr & 0x3F) + 1;

    switch(instruction) {
        case 32:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] << ((~immr & 0x3F) + 1);
            break;
        case 33:
            NEXT_STATE.REGS[Rd] = ((CURRENT_STATE.REGS[Rn] >> ((~immr & 0x3F) + 1)) & (~(0x8000000000000000 >> shamt)));
            break;
    }
    NEXT_STATE.PC += 4;
}

void execute_i_instruction(uint32_t instruction, uint32_t machine_code) {
    uint32_t immediate, Rn, Rd;
    immediate = (machine_code >> 10) & 0xFFF;
    Rn = (machine_code >> 5) & 0x1F;
    Rd = machine_code & 0x1F;

    int64_t op1 = CURRENT_STATE.REGS[Rn];

    switch(instruction) {
        case 28:
            NEXT_STATE.REGS[Rd] = op1 + immediate;
            break;
        case 29:
            NEXT_STATE.FLAG_C = (op1 + immediate < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            NEXT_STATE.FLAG_N = IS_NEGATIVE(op1 + immediate);
            NEXT_STATE.FLAG_Z = IS_ZERO(op1 + immediate);
            NEXT_STATE.FLAG_V = ((op1 > 0 && immediate > 0 && op1 + immediate < op1) || (op1 < 0 && immediate < 0 && op1 + immediate > 0)) ? 1 : 0;

            NEXT_STATE.REGS[Rd] = op1 + immediate;
            break;
        case 30:
            NEXT_STATE.REGS[Rd] = op1 - immediate;
            break;
        case 31:
            NEXT_STATE.FLAG_C = (op1 - immediate < (int64_t) 0xFFFFFFFFFFFFFFFF) ? 1 : 0;
            NEXT_STATE.FLAG_N = IS_NEGATIVE(op1 - immediate);
            NEXT_STATE.FLAG_Z = IS_ZERO(op1 - immediate);
            NEXT_STATE.FLAG_V = ((op1 > 0 && immediate < 0 && op1 - immediate < 0) || (op1 < 0 && immediate > 0 && op1 - immediate > 0)) ? 1 : 0;

            NEXT_STATE.REGS[Rd] = op1 - immediate;
            break;
    }
    NEXT_STATE.PC += 4;
}

execute_d_instruction(uint32_t instruction, uint32_t machine_code) {
    uint32_t DT_address, op, Rn, Rt;
    DT_address = (machine_code >> 12) & 0x1FF;
    op = (machine_code >> 10) & 0x3;
    Rn = (machine_code >> 5) & 0x1F;
    Rt = machine_code & 0x1F;

    int64_t address_to_rw = CURRENT_STATE.REGS[Rn] + DT_address;

    switch(instruction) {
        case 21: ;
            uint32_t half_byte_1 = mem_read_32(address_to_rw);
            uint32_t half_byte_2 = mem_read_32(address_to_rw + 0x4);
            int64_t full_value = half_byte_1 | ((int64_t) half_byte_2 << 32);
            NEXT_STATE.REGS[Rt] = full_value;
            break;
        case 22: ;
            uint32_t first_half1 = CURRENT_STATE.REGS[Rt] & 0xFFFFFFFF;
            uint32_t second_half1 = (CURRENT_STATE.REGS[Rt] & 0xFFFFFFFF00000000) >> 32;
            mem_write_32(address_to_rw, first_half1);
            mem_write_32(address_to_rw + 0x4, second_half1);
            break;
        case 23:
            NEXT_STATE.REGS[Rt] = mem_read_32(address_to_rw) & 0xFF;
            break;
        case 24:
            NEXT_STATE.REGS[Rt] = mem_read_32(address_to_rw) & 0xFFFF;
            break;
        case 25:
            mem_write_32(address_to_rw, (uint32_t)CURRENT_STATE.REGS[Rt] & 0xFF);
            break;
        case 26:
            mem_write_32(address_to_rw, (uint32_t)CURRENT_STATE.REGS[Rt] & 0xFFFF);
            break;
        case 27: ;
            uint32_t val_to_write = CURRENT_STATE.REGS[Rt] & 0xFFFFFFFF;
            mem_write_32(address_to_rw, val_to_write);
            break;

    }
    NEXT_STATE.PC += 4;
}

execute_b_instruction(uint32_t instruction, uint32_t machine_code) {
    uint32_t BR_address = machine_code & 0x3FFFFFF;
    int64_t BR_address64 = (BR_address >> 25) ? (0xFFFFFFFFF0000000 | ((int64_t)BR_address << 2)) : 0 | (BR_address << 2);

    switch(instruction) {
        case 34:
            NEXT_STATE.PC = CURRENT_STATE.PC + BR_address64;
            return;
        case 35:
            NEXT_STATE.REGS[30] = CURRENT_STATE.PC + 4;
            NEXT_STATE.PC = CURRENT_STATE.PC + BR_address64;
            return;
    }
}

execute_cb_instruction(uint32_t instruction, uint32_t machine_code) {

    uint32_t COND_BR_address, Rt;
    COND_BR_address = (machine_code >> 5) & 0x7FFFF;
    Rt = machine_code & 0x1F;

    int64_t CondBranchAddr = ((COND_BR_address >> 18) != 0) ? (COND_BR_address << 2) | 0xFFFFFFFFFFE00000 : (CondBranchAddr = 0 | (COND_BR_address << 2));

    uint32_t N = CURRENT_STATE.FLAG_N, Z = CURRENT_STATE.FLAG_Z, V = CURRENT_STATE.FLAG_V, C = CURRENT_STATE.FLAG_C;
    int64_t branch_address = CURRENT_STATE.PC + CondBranchAddr;
    int64_t next_address = NEXT_STATE.PC + 4;

    switch (instruction) {
        case 13:
            NEXT_STATE.PC = (CURRENT_STATE.REGS[Rt] != 0) ? branch_address : next_address;
            break;
        case 14:
            NEXT_STATE.PC = (CURRENT_STATE.REGS[Rt] == 0) ? branch_address : next_address;
            break;
        case 15:
            NEXT_STATE.PC = (Z == 1) ? branch_address : next_address;
            break;
        case 16:
            NEXT_STATE.PC = (Z == 0) ? branch_address : next_address;
            break;
        case 17:
            NEXT_STATE.PC = (Z == 0 && N == V) ? branch_address : next_address;
            break;
        case 18:
            NEXT_STATE.PC = (N != V) ? branch_address : next_address;
            break;
        case 19:
            NEXT_STATE.PC = (N == V) ? branch_address : next_address;
            break;
        case 20:
            NEXT_STATE.PC = (!(Z == 0 && N == V)) ? branch_address : next_address;
            break;
    }
}

execute_iw_instruction(uint32_t instruction, uint32_t machine_code) {
    uint32_t op2, immediate, Rd;
    op2 = (machine_code >> 21) & 0x3;
    immediate = (machine_code >> 5) & 0xFFFF;
    Rd = machine_code & 0x1F;

    switch(instruction) {
        case 36:
            NEXT_STATE.REGS[Rd] = immediate << (op2 * 16);
            NEXT_STATE.PC += 4;
            break;
    }

}

void execute(uint32_t instruction, uint32_t machine_code)
{
    if (instruction < 13) { // R-Format
        execute_r_instruction(instruction, machine_code);
    } else if (instruction < 21) {
        execute_cb_instruction(instruction, machine_code);
    } else if (instruction < 28) {
        execute_d_instruction(instruction, machine_code);
    } else if (instruction < 32) {
        execute_i_instruction(instruction, machine_code);
    } else if (instruction < 34) {
        execute_shift_instruction(instruction, machine_code);
    } else if (instruction < 36) {
        execute_b_instruction(instruction, machine_code);
    } else if (instruction < 37) {
        execute_iw_instruction(instruction, machine_code);
    } else {
        NEXT_STATE.PC += 4;
        RUN_BIT = 0;
    }
}

void process_instruction() {
    uint32_t machine_code = fetch();
    execute(decode(machine_code),machine_code);
}
