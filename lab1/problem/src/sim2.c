#include <stdio.h>
#include <stdlib.h>
#include "shell.h"

#define IS_NEGATIVE(x) (x < 0) ? 1 : 0
#define IS_ZERO(x) (x== 0) ? 1 : 0

typedef enum instruction {
    //R type
    ADD,//0
    ADDS,
    AND,
    ANDS,
    BR,
    EOR,
    MUL,
    ORR,
    SDIV,
    SUB,
    SUBS,
    UDIV,
    // Shift-Type
    LSL, //12
    LSR,
    // I-type
    ADDI,//14
    ADDIS,
    SUBI,
    SUBIS,
    // D-type
    LDUR,//18
    STUR,
    LDURB,
    LDURH,
    STURB,
    STURH,
    STURW,
    // B-type
    B,//25
    BL,
    //CB-type
    CBNZ,//27
    CBZ,
    // IW-type
    MOVZ,//29
    // Other-Type
    BEQ,//30
    BNE,
    BGT,
    BLT,
    BGE,
    BLE,
    HLT,//implemented
    CMP
} instruction;



uint32_t fetch() {
    uint32_t hex_instruction = mem_read_32(CURRENT_STATE.PC);
    return hex_instruction;
}

instruction decode(uint32_t machine_code){
    // R-Format
    uint32_t r_opcode = (machine_code >> 21) & 0x7FF;
    uint32_t r_shamt = (machine_code >> 10) & 0x3F;
    switch (r_opcode) {
        case 0x458:
            return ADD;
        case 0x558:
            return ADDS;
        case 0x450:
            return AND;
        case 0x750:
            return ANDS;
        case 0x6B0:
            return BR;
        case 0x650:
            return EOR;
        case 0x550:
            return ORR;
        case 0x658:
            return SUB;
        case 0x758:
            return SUBS;
        case 0x4D8:
             if (r_shamt == 0x1F) {
                return MUL;
             }
        case 0x4D6:
            return (r_shamt == 0x3) ? SDIV : UDIV;
        case 0x69A:
        case 0x69B:
            return (r_shamt == 0x3F) ? LSR : LSL;
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

    //  D-format
    uint32_t d_opcode = (machine_code >> 21) & 0x7FF;
    switch(d_opcode) {
        case 0x7C2:
            return LDUR;
        case 0x1C2:
            return LDURB;
        case 0x3C2:
            return LDURH;
        case 0x7C0:
            return STUR;
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
    }

    //IW-Format
    uint32_t iw_opcode = (machine_code >> 23) & 0x1FF;
    switch (iw_opcode) {
        case 0x1A5:
            return MOVZ;
    }
    return 0;
}

void execute_r_instruction(instruction i, uint32_t machine_code) {
    uint32_t opcode, shamt, Rm, Rn, Rd;
    opcode = (machine_code >> 21) & 0x7FF;
    shamt = (machine_code >> 10) & 0x3F;
    Rm = (machine_code >> 16) & 0x1F;
    Rn = (machine_code >> 5) & 0x1F;
    Rd = machine_code & 0x1F;

    printf("format: R_FORMAT\n");
    printf("type: %d\n", i);
    printf("opcode: %08x\n", opcode);
    printf("Rm: %08x\n", Rm);
    printf("shamt: %08x\n",shamt);
    printf("Rn: %08x\n", Rn);
    printf("Rd: %08x\n\n", Rd);

    int64_t first_operand = CURRENT_STATE.REGS[Rn];
    int64_t second_operand = CURRENT_STATE.REGS[Rm];

    switch (i) {
        case ADD:
            NEXT_STATE.REGS[Rd] = first_operand + second_operand;
            break;
        case ADDS:
            NEXT_STATE.FLAG_C = (((first_operand & 1<<31) ^ (second_operand & 1<<31)) != 0);
            NEXT_STATE.FLAG_N = IS_NEGATIVE(first_operand + second_operand);
            NEXT_STATE.FLAG_V = ((first_operand > 0) && (second_operand > 0xFFFFFFFFFFFFFFFF - first_operand));
            NEXT_STATE.FLAG_Z = IS_ZERO(first_operand + second_operand);

            NEXT_STATE.REGS[Rd] = first_operand + second_operand;

            break;
        case AND:
            NEXT_STATE.REGS[Rd] = first_operand & second_operand;
            break;
        case ANDS:
            NEXT_STATE.FLAG_C = (((first_operand & 1<<31) ^ (second_operand & 1 << 31)) != 0);
            NEXT_STATE.FLAG_N = IS_NEGATIVE(first_operand & second_operand);
            NEXT_STATE.FLAG_V = ((first_operand > 0 && (second_operand > 0xFFFFFFFFFFFFFFFF - first_operand)));
            NEXT_STATE.FLAG_Z = IS_ZERO(first_operand & second_operand);

            NEXT_STATE.REGS[Rd] = first_operand & second_operand;
        case BR:
            NEXT_STATE.PC = CURRENT_STATE.PC + CURRENT_STATE.REGS[first_operand];
            break;
        case EOR:
            NEXT_STATE.REGS[Rd] = first_operand ^ second_operand;
            break;
        case MUL:
            NEXT_STATE.REGS[Rd] = first_operand * second_operand;
            break;
        case ORR:
            NEXT_STATE.REGS[Rd] = first_operand | second_operand;
            break;
        case SDIV:
            if (second_operand) {
                NEXT_STATE.REGS[Rd] = first_operand / second_operand;
            } else {
              NEXT_STATE.REGS[Rd] = 0;
            }
            break;
        case SUB:
            NEXT_STATE.REGS[Rd] = first_operand - second_operand;
            break;
        case SUBS:
            NEXT_STATE.FLAG_C = (((first_operand & 1<<31) == (second_operand & 1<<31)) && ((first_operand & 1<<31) != 0));
            NEXT_STATE.FLAG_N = IS_NEGATIVE(first_operand - second_operand);
            NEXT_STATE.FLAG_V = ((first_operand < 0) && (second_operand < 0 - first_operand));
            NEXT_STATE.FLAG_Z = IS_ZERO(first_operand - second_operand);

            NEXT_STATE.REGS[Rd] = first_operand - second_operand;
        case UDIV:
            if (second_operand) {
                NEXT_STATE.REGS[Rd] = first_operand / second_operand;
            } else {
              NEXT_STATE.REGS[Rd] = 0;
            }
    }
    NEXT_STATE.PC += 4;

}

void execute_shift_instruction(i, machine_code) {
    uint32_t opcode, immr, imms, Rn, Rd;
    opcode = (machine_code >> 22) & 0x3FF;
    immr = (machine_code >> 16) & 0x3F;
    imms = (machine_code >> 10) & 0x3F;
    Rn = (machine_code >> 5) & 0x1F;
    Rd = machine_code & 0x1F;

    printf("format: SHIFT_FORMAT\n");
    printf("type: %d\n", i);
    printf("opcode: %08x\n", opcode);
    printf("immr: %08x\n", immr);
    printf("imms: %08x\n", imms);
    printf("Rn: %08x\n", Rn);
    printf("Rd: %08x\n\n", Rd);

    switch(i) {
        case LSL:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] << ((~immr  & 0x3F)+ 1);
            break;
        case LSR:
            NEXT_STATE.REGS[Rd] = (CURRENT_STATE.REGS[Rn] >> ((~immr & 0x3f) + 1)) & ~(0x8000000000000000 >> ((~immr & 0x3f) + 1));
            break;
    }

    NEXT_STATE.PC += 4;
}

void execute_i_instruction(instruction i, uint32_t machine_code) {
    uint32_t opcode, ALU_immediate, Rn, Rd;
    opcode = (machine_code >> 22) & 0x3FF;
    ALU_immediate = (machine_code >> 10) & 0xFFF;
    Rn = (machine_code >> 5) & 0x1F;
    Rd = machine_code & 0x1F;

    printf("format: I_FORMAT\n");
    printf("type: %d\n", i);
    printf("opcode: %08x\n", opcode);
    printf("ALU_immediate: %08x\n", ALU_immediate);
    printf("Rn: %08x\n", Rn);
    printf("Rd: %08x\n\n", Rd);

    int64_t first_operand = CURRENT_STATE.REGS[Rn];

    switch(i) {
        case ADDI:
            NEXT_STATE.REGS[Rd] = first_operand + ALU_immediate;
            break;
        case ADDIS:
            NEXT_STATE.FLAG_C = (((first_operand & 1 << 31) ^ ALU_immediate & 1 << 31) != 0);
            NEXT_STATE.FLAG_N = IS_NEGATIVE(first_operand + ALU_immediate);
            NEXT_STATE.FLAG_V = ((first_operand > 0) && (ALU_immediate > 0xFFFFFFFFFFFFFFFF - first_operand));
            NEXT_STATE.FLAG_Z = IS_ZERO(first_operand + ALU_immediate);

            NEXT_STATE.REGS[Rd] = first_operand + ALU_immediate;
            break;

        case SUBI:
            NEXT_STATE.REGS[Rd] = first_operand - ALU_immediate;
            break;
        case SUBIS:
            NEXT_STATE.FLAG_C = (((first_operand & 1<<31) == (ALU_immediate & 1<<31)) && ((first_operand & 1<<31) != 0));
            NEXT_STATE.FLAG_N = IS_NEGATIVE(first_operand - ALU_immediate);
            NEXT_STATE.FLAG_V = ((first_operand < 0) && (ALU_immediate < 0 - first_operand));
            NEXT_STATE.FLAG_Z = IS_ZERO(first_operand - ALU_immediate);

            NEXT_STATE.REGS[Rd] = first_operand - ALU_immediate;
            break;
    }
    NEXT_STATE.PC += 4;
}

execute_d_instruction(instruction i, uint32_t machine_code) {

    uint32_t opcode, DT_address, op, Rn, Rt;
    opcode = (machine_code >> 21) & 0x7FF;
    DT_address = (machine_code >> 12) & 0x1FF;
    op = (machine_code >> 10) & 0x3;
    Rn = (machine_code >> 5) & 0x1F;
    Rt = machine_code & 0x1F;

    printf("format: D_FORMAT\n");
    printf("type: %d\n", i);
    printf("opcode: %08x\n", opcode);
    printf("DT_address: %08x\n", DT_address);
    printf("op: %08x\n", op);
    printf("Rn: %08x\n", Rn);
    printf("Rt: %08x\n\n", Rt);

    int64_t x;
    uint32_t y = mem_read_32(CURRENT_STATE.REGS[Rn] + DT_address);

    switch(i) {
        case LDUR: ;
            uint32_t first_half = mem_read_32(CURRENT_STATE.REGS[Rn] + DT_address);
            uint32_t second_half = mem_read_32(CURRENT_STATE.REGS[Rn] + DT_address + 0x4);
            int64_t full_value = first_half | ((int64_t)second_half << 32);
            NEXT_STATE.REGS[Rt] = full_value;
            break;
        case LDURB:

            NEXT_STATE.REGS[Rt] = mem_read_32(CURRENT_STATE.REGS[Rn] + DT_address) & 0xFF;
            break;
        case LDURH:
            NEXT_STATE.REGS[Rt] = mem_read_32(CURRENT_STATE.REGS[Rn] + DT_address) & 0xFFFF;
            break;
        case STUR: ;
            uint32_t first_half1 = CURRENT_STATE.REGS[Rt] & 0xFFFFFFFF;
            uint32_t second_half1 = (CURRENT_STATE.REGS[Rt] & 0xFFFFFFFF00000000) >> 32;
            mem_write_32(CURRENT_STATE.REGS[Rn] + DT_address, first_half1);
            mem_write_32(CURRENT_STATE.REGS[Rn] + DT_address + 0x4, second_half1);
            break;
        case STURB:
            mem_write_32(CURRENT_STATE.REGS[Rn] + DT_address, (uint32_t)CURRENT_STATE.REGS[Rt] & 0xFF);

            break;
        case STURH:
            mem_write_32(CURRENT_STATE.REGS[Rn] + DT_address, (uint32_t)CURRENT_STATE.REGS[Rt] & 0xFFFF);
            break;
        case STURW:
            printf("CURRENT_STATE.REGS[Rn]: %" PRIx64 "\n", CURRENT_STATE.REGS[Rn]);
            uint32_t val_to_write = CURRENT_STATE.REGS[Rt] & 0xFFFFFFFF;
            mem_write_32(CURRENT_STATE.REGS[Rn] + DT_address, 32);
            break;

    }
    NEXT_STATE.PC += 4;
}

execute_b_instruction(instruction i, uint32_t machine_code) {
    uint32_t opcode, BR_address;
    opcode = (machine_code >> 26) & 0x3F;
    BR_address = machine_code & 0x3FFFFFF;

    printf("format: B_FORMAT\n");
    printf("type: %d\n", i);
    printf("opcode: %08x\n", opcode);
    printf("BR_address: %08x\n\n", BR_address);

    int64_t BR_address64 = (BR_address >> 25) ? (0xFFFFFFFFF0000000 | ((int64_t)BR_address << 2)) : 0 | (BR_address << 2);

    switch(i) {
        case B:
            NEXT_STATE.PC = CURRENT_STATE.PC + BR_address64;
            break;
        case BL:
            NEXT_STATE.REGS[30] = CURRENT_STATE.PC + 4;
            NEXT_STATE.PC = CURRENT_STATE.PC + BR_address64;
            break;
    }

    NEXT_STATE.PC += 4;

}

execute_cb_instruction(instruction i, uint32_t machine_code) {
    uint32_t opcode, COND_BR_address, Rt;

    opcode = (machine_code >> 24) & 0xFF;
    COND_BR_address = (machine_code >> 5) & 0x7FFFF;
    Rt = machine_code & 0x1F;

    printf("format: CB_FORMAT\n");
    printf("type: %d\n", i);
    printf("opcode: %08x\n", opcode);
    printf("COND_BR_address: %08x\n", COND_BR_address);
    printf("Rt: %08x\n\n", Rt);

    int64_t CondBranchAddr;
    if (COND_BR_address >> 18) {
        CondBranchAddr = 0xFFFFFFFFFFE00000 | (COND_BR_address << 2);
    } else {
        CondBranchAddr = 0 | (COND_BR_address << 2);
    }
    switch (i) {
        case CBNZ:
            if (CURRENT_STATE.REGS[Rt]) {
                NEXT_STATE.PC = CURRENT_STATE.PC + CondBranchAddr;
            }
            break;
        case CBZ:
            if (CURRENT_STATE.REGS[Rt] == 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + CondBranchAddr;
            }
            break;
    }

    NEXT_STATE.PC += 4;
}

execute_iw_instruction(instruction i, uint32_t machine_code) {
    uint32_t opcode, op2, MOV_immediate, Rd;
    opcode = (machine_code >> 23) & 0x1FF;
    op2 = (machine_code >> 21) & 0x3;
    MOV_immediate = (machine_code >> 5) & 0xFFFF;
    Rd = machine_code & 0x1F;

    printf("format: IW_FORMAT\n");
    printf("type: %d\n", i);
    printf("opcode: %08x\n", opcode);
    printf("op2: %08x\n", op2);
    printf("MOV_immediate: %08x\n", MOV_immediate);
    printf("Rd: %08x\n\n", Rd);
    switch(i) {
        case MOVZ:
            NEXT_STATE.REGS[Rd] = MOV_immediate << (op2 * 16);
            break;
    }
    NEXT_STATE.PC += 4;
}
execute_other_instruction(instruction i, uint32_t machine_code) {
    switch(i) {
        case HLT:
            RUN_BIT = 0;
            break;

    }
}

void execute(instruction i, uint32_t machine_code)
{
    if (i < 12) {
        // R-Format
        execute_r_instruction(i, machine_code);
    } else if (i < 14) {
        // Shift-Format
        execute_shift_instruction(i, machine_code);

    } else if (i < 18) {
        //I-Format
        execute_i_instruction(i, machine_code);
    } else if (i < 25) {
        // D-Format
        execute_d_instruction(i, machine_code);
    } else if (i < 27) {
        // B-format
        execute_b_instruction(i, machine_code);
    } else if (i < 29) {
        // CB-Format
        execute_cb_instruction(i, machine_code);
    } else if (i < 30) {
        // IW-Format
        execute_iw_instruction(i, machine_code);
    } else {
        // Other-Format
        execute_other_instruction(i, machine_code);
    }

}

void process_instruction() {
    printf("fetched address: %08x\n", fetch());

    uint32_t machine_code = fetch();
    instruction i = decode(machine_code);
    execute(i,machine_code);
}
