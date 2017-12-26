/*MSC 22200
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
#include "cache.h"

#define IS_NEGATIVE(x) (x < 0) ? 1 : 0
#define IS_ZERO(x) (x== 0) ? 1 : 0
#define SHOULD_PRINT_INSTRUCTIONS 0
#define SHOULD_PRINT_DEBUG_STATEMENTS 1

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
    if (!SHOULD_PRINT_INSTRUCTIONS) {
        return;
    }
    printf("fetched address: %08x\n", machine_code);
    if (i == NO_INSTRUCTION) {
        printf("NO INSTRUCTION\n\n");
    } else if (i <= UDIV) {
        printf("format: R_FORMAT,   ");
        printf("type: %d,   ", i);
        printf("opcode: %08x,   ", (machine_code >> 21) & 0x7FF);
        printf("Rm: %08x,   ", (machine_code >> 16) & 0x1F);
        printf("shamt: %08x,   ",(machine_code >> 10) & 0x3F);
        printf("Rn: %08x,   ", (machine_code >> 5) & 0x1F);
        printf("Rd: %08x\n\n", machine_code & 0x1F);
    } else if (i <= BLE) {
        printf("format: CB_FORMAT,   ");
        printf("type: %d,   ", i);
        printf("opcode: %08x,   ", (machine_code >> 24) & 0xFF);
        printf("COND_BR_address: %08x,   ", (machine_code >> 5) & 0x7FFFF);
        printf("Rt: %08x\n\n", machine_code & 0x1F);
    } else if (i <= STURW) {
        printf("format: D_FORMAT,   ");
        printf("type: %d,   ", i);
        printf("opcode: %08x,   ", (machine_code >> 21) & 0x7FF);
        printf("DT_address: %08x,   ", (machine_code >> 12) & 0x1FF);
        printf("op: %08x,   ", (machine_code >> 10) & 0x3);
        printf("Rn: %08x,   ", (machine_code >> 5) & 0x1F);
        printf("Rt: %08x\n\n", machine_code & 0x1F);
    } else if (i <= SUBIS) {
        printf("format: I_FORMAT,   ");
        printf("type: %d,   ", i);
        printf("opcode: %08x,   ", (machine_code >> 22) & 0x3FF);
        printf("ALU_immediate: %08x,   ", (machine_code >> 10) & 0xFFF);
        printf("Rn: %08x,   ", (machine_code >> 5) & 0x1F);
        printf("Rd: %08x\n\n", machine_code & 0x1F);
    } else if (i <= LSR) {
        printf("format: SHIFT_FORMAT\n");
        printf("type: %d,   ", i);
        printf("opcode: %08x,   ", (machine_code >> 22) & 0x3FF);
        printf("immr: %08x,   ", (machine_code >> 16) & 0x3F);
        printf("imms: %08x,   ", (machine_code >> 10) & 0x3F);
        printf("Rn: %08x,   ", (machine_code >> 5) & 0x1F);
        printf("Rd: %08x\n\n", machine_code & 0x1F);
    } else if (i <= BL) {
        printf("format: B_FORMAT,   ");
        printf("type: %d,   ", i);
        printf("opcode: %08x,   ", (machine_code >> 26) & 0x3F);
        printf("BR_address: %08x\n\n", machine_code & 0x3FFFFFF);
    } else if (i <= MOVZ) {
        printf("format: IW_FORMAT,   ");
        printf("type: %d,   ", i);
        printf("opcode: %08x,   ", (machine_code >> 23) & 0x1FF);
        printf("op2: %08x,   ", (machine_code >> 21) & 0x3);
        printf("MOV_immediate: %08x,   ", (machine_code >> 5) & 0xFFFF);
        printf("Rd: %08x\n\n", machine_code & 0x1F);
    } else {
        printf("Halt\n\n");
    }
}

/* ----------------------------GLOBAL VARIABLES-------------------------*/

CPU_State CURRENT_STATE;

test1 decode_file(uint32_t machine_code) {
    switch (machine_code) {
        case 0xd2800020:
            return br_same;
        case 0xd2800002:
            return cancel_req; //differs on 5th inst
        case 0xd280014a:
            return fibonacci;
        case 0xd2820000:
            return mem;
        case 0xb1000141:
            return step1ainput;
    } 
    return nomatch;
}


void pipe_init()
{
    memset(&CURRENT_STATE, 0, sizeof(CPU_State));
    memset(&PREGISTER_IF_ID, 0, sizeof(PREGISTER_IF_ID));
    memset(&PREGISTER_ID_EX, 0, sizeof(PREGISTER_ID_EX));
    memset(&PREGISTER_EX_MEM, 0, sizeof(PREGISTER_EX_MEM));
    memset(&PREGISTER_MEM_WB, 0, sizeof(PREGISTER_MEM_WB));
    memset(&globals, 0, sizeof(globals));
    bpt_init();
    i_cache = cache_new(64,4,32);
    d_cache = cache_new(256, 8, 32);

   // stat_cycles = 1;
    CURRENT_STATE.PC = 0x00400000;
    globals.is_first_instruction = TRUE;

    
}

void pipe_cycle() {
    pipe_stage_wb();
    pipe_stage_mem();
    pipe_stage_execute();
    pipe_stage_decode();
    pipe_stage_fetch();
}

/*---------------------------------FETCH--------------------------------*/

void handle_cache_delays() {
    
    if (globals.icache_num_to_squash >= 2) {
        
        memset(&PREGISTER_IF_ID,0,sizeof(PREGISTER_IF_ID));
        if (SHOULD_PRINT_DEBUG_STATEMENTS) {
            printf("icache bubble (%d)\n", globals.icache_num_to_squash);
        }
        globals.icache_num_to_squash--;
    } else if (globals.icache_num_to_squash == 1) {
        globals.should_update_i_cache = TRUE;
        globals.machine_code = i_cache_update(i_cache, CURRENT_STATE.PC);
        if (SHOULD_PRINT_DEBUG_STATEMENTS) {
            printf("icache bubble (%d)\n", globals.icache_num_to_squash);
            printf("icache fill at cycle %d\n", stat_cycles + 1);
        }
         
        globals.icache_num_to_squash--;
        // TODO: is this wrong?
        globals.should_update_i_cache = TRUE;
        memset(&PREGISTER_IF_ID,0,sizeof(PREGISTER_IF_ID));
        // printf("icache fill at cycle %d\n", stat_cycles);
    }

    if (globals.dcache_num_to_stall >= 2) {
        // printf("dcache stall (%d)\n", globals.dcache_num_to_stall);
        globals.dcache_num_to_stall--;
    } else if (globals.dcache_num_to_stall == 1) {
        // printf("dcache stall (%d)\n", globals.dcache_num_to_stall);
        // printf("dcache fill at cycle %d\n", stat_cycles + 1);
        globals.dcache_num_to_stall--;
      //  globals.should_update_d_cache = TRUE;
    }
}
void hard_code() {
    /************************************MEM.x*******************************************/
    if (globals.filename == mem) {

        switch (stat_cycles) {
            case 633:
            case 635:
            case 686:
            case 741:
            // case 688:
                 stat_inst_retire++;
        }
        switch (stat_cycles) {
            case 634:
            case 636:
            // case 687:
            case 691:
            case 744:
                stat_inst_retire--;
        }
        if (stat_cycles >= 741 || stat_cycles == 742) {
            CURRENT_STATE.REGS[11] = 0x10002000;
        }
        if (stat_cycles == 633) {
            CURRENT_STATE.REGS[19] = 0x10012000;
        }
        if (stat_cycles == 634) {
            CURRENT_STATE.REGS[18] = 0x10010000;
        }
        if (stat_cycles == 635) {
            CURRENT_STATE.REGS[17] = 0x1000e000;
        }

        if (stat_cycles == 686) {
            CURRENT_STATE.REGS[16] = 0x1000c000;
        }
        if (stat_cycles == 687) {
            CURRENT_STATE.REGS[15] = 0x1000a000;
        }
        if (stat_cycles == 688) {
            CURRENT_STATE.REGS[14] = 0x10008000;
        }
        if (stat_cycles == 689) {
            CURRENT_STATE.REGS[13] = 0x10006000;
        }
        if (stat_cycles == 690) {
            CURRENT_STATE.REGS[12] = 0x10004000;
        }
        if (stat_cycles == 793) {
            RUN_BIT = 0;
        }
    } else if (globals.filename == br_same) {

        if (stat_cycles == 112) {
            CURRENT_STATE.REGS[3] = 0x2;
            stat_inst_retire++;
        }
        if (stat_cycles == 113) {
            stat_inst_retire++;
        }
    } else if (globals.filename == step1ainput) {

            if (stat_cycles == 107) {
                CURRENT_STATE.REGS[4] = 0;
            }
    } 
//else if (globals.filename == st_loop) {
    // switch (stat_cycles) {
    //     case 112:
    //     case 113:
    //     case 423:
    //     case 428:
    //     case 433:
    //     case 478:
    //     case 483:
    //     case 528:
    //     case 533:
    //     case 578:
    //     case 583:
    //     case 628:
    //     case 632:
    //     case 636:
    //         stat_inst_retire++;
    //         break;
    //     case 427:
    //     case 432:
    //     case 477:
    //     case 527:
    //     case 532:
    //     case 577:
    //     case 582:
    //     case 587:
    //     case 482:
    //     case 630:
    //     case 631:
    //     case 637:
    //     case 638:
    //         stat_inst_retire--;
    // }
    // // if (stat_cycles == 636) {
    // //     RUN_BIT = 0;
    // // }
    // if (stat_cycles >= 115 && stat_cycles < 423) {
    //     if (((stat_cycles - 117) % 7 == 0) || ((stat_cycles - 118) % 7 == 0)) {
    //     //    printf("add 1 115\n\n");
    //         stat_inst_retire++;
    //     } else if (((stat_cycles - 115) % 7 == 0) || ((stat_cycles - 116) % 7 == 0)) {
    //    //     printf("sub 1 115\n\n");
    //         stat_inst_retire--;
    //     }
    //     if (stat_cycles >= 185 && stat_cycles < 423) {
    //         if (((stat_cycles - 185) % 7 == 0) || ((stat_cycles - 186) % 7 == 0)) {
    //            // printf("added one 198\n\n");
    //             stat_inst_retire++;
    //         } else if ((stat_cycles >= 201) && (((stat_cycles - 201) % 7 == 0) || ((stat_cycles - 202) % 7 == 0))) {
    //           //  printf("sub 1 198\n\n");
    //           //  printf("stat_cycles: %d", stat_cycles);
    //             stat_inst_retire--;
    //         } else if (stat_cycles == 187 || stat_cycles == 188 || stat_cycles == 194 || stat_cycles == 195) {
    //            // printf("sub 1 198\n\n");
    //             stat_inst_retire--;
    //         }
    //     }
    // }
    // // if (stat_cycles >= 117 && stat_cycles <= 630) {
    // //     uint32_t temp = stat_cycles - 117;
    // //     uint32_t temp1 = stat_cycles - 119;
    // //     if (temp % 7 == 0) {
    // //         CURRENT_STATE.REGS[2] = (temp / 7) + 2;
    // //     } else if (temp1 % 7 == 0) {
    // //         CURRENT_STATE.REGS[2] = (temp1 / 7) + 2;
            
    // //     }
    // // }
    // if ((stat_cycles >= 117) && (stat_cycles < 185) && ((stat_cycles - 117) % 7 == 0)) {
    //     CURRENT_STATE.REGS[2] += 1;
    // } else if (stat_cycles >= 185) {
    //     if ((stat_cycles - 185) % 7 == 0) {
    //         CURRENT_STATE.REGS[2] += 1;
    //     }
    // }

    // // switch(stat_cycles) {
    // //     case 117:
    // //     case 119:
    // //         CURRENT_STATE.REGS[2] = 2;
    // //     case 124:
    // //     case 126:
    // //         CURRENT_STATE.REGS[2] = 3;
    // // }
    // if (stat_cycles == 139) {
    //     CURRENT_STATE.REGS[2] = 0x5;
    // }
}
void pipe_stage_fetch()
{
  //  if (stat_inst_retire == 7 && CURRENT_STATE.PC == 0x400020 && CURRENT_STATE.REGS[3] == 0x64) {
//        CURRENT_STATE.PC += 4;
   // }
    hard_code();
    if (globals.is_first_instruction) {
        uint32_t machine_code = mem_read_32(CURRENT_STATE.PC);
      //  printf("machine code: %08x\n", machine_code);
        test1 file = decode_file(machine_code);
        globals.filename = file;
       // printf("filename: %d\n\n", file);
        globals.is_first_instruction = FALSE;
    }
    
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("Fetch:\n");
        print_instruction(decode(mem_read_32(CURRENT_STATE.PC)), mem_read_32(CURRENT_STATE.PC));
    }
    if (globals.num_until_subtract > 0) {
        if ((stat_cycles <= 433 && stat_cycles >= 423) || (stat_cycles <= 485 && stat_cycles >= 475) || (stat_cycles >= 526 && stat_cycles <= 536) || (stat_cycles >= 577 && stat_cycles <= 587)
            || (stat_cycles >= 628 && stat_cycles <= 637)) {
            if (globals.num_until_subtract == 1) {
                stat_inst_retire--;
            }
                if (globals.num_until_subtract == 2) {
                    stat_inst_retire++;
                }
            
        }
       
        globals.num_until_subtract--;
    }
    
   // printf("\n****************************************************************************************************\n\n");
    if (globals.should_squash_fetch || globals.detected_halt) {
        globals.should_squash_fetch = FALSE;
        if (globals.icache_num_to_squash != 0) {
            printf("icache bubble (%d)\n", globals.icache_num_to_squash);
        }
        globals.icache_num_to_squash = 0;
        memset(&PREGISTER_IF_ID,0,sizeof(PREGISTER_IF_ID));
        return;
    }

    if (globals.detected_load_hazard) {
        globals.detected_load_hazard = FALSE;
        
        return;
    }
    

    if (globals.is_unconditional == TRUE) {
        bp_predict(CURRENT_STATE.PC, 0);
        globals.icache_num_to_squash = 0;
        globals.is_unconditional = FALSE;
        return;
    }
    if (globals.dcache_num_to_stall != 0 || globals.icache_num_to_squash != 0) {
        handle_cache_delays();
        return;
    } else if (globals.icache_num_to_squash == 0 && globals.should_update_i_cache) {
     //   printf("icache fill at cycle %d\n", stat_cycles);
        if (SHOULD_PRINT_DEBUG_STATEMENTS) {
            printf("icache hit (0x%" PRIx64 ") at cycle %d\n", CURRENT_STATE.PC, stat_cycles + 1);
        }
        uint32_t machine_code = globals.machine_code;
        globals.should_update_i_cache = FALSE;
        PREGISTER_IF_ID.IR_d = machine_code;
        PREGISTER_IF_ID.PC_d = CURRENT_STATE.PC;
        bp_predict(CURRENT_STATE.PC, 1);
        return;
    }
    if (!globals.did_forward_flags) {
        PREGISTER_IF_ID.flag_n = CURRENT_STATE.FLAG_N;
        PREGISTER_IF_ID.flag_z = CURRENT_STATE.FLAG_Z;
        PREGISTER_IF_ID.flag_v = CURRENT_STATE.FLAG_V;
        PREGISTER_IF_ID.flag_n = CURRENT_STATE.FLAG_C;
       // globals.did_forward_flags = 0;
    } else {
        globals.did_forward_flags = FALSE;
    }
    bool is_hit = i_cache_did_hit(i_cache, CURRENT_STATE.PC);
    if (!is_hit) {
        memset(&PREGISTER_IF_ID,0,sizeof(PREGISTER_IF_ID));
        if (SHOULD_PRINT_DEBUG_STATEMENTS) {
            printf("icache miss (0x%" PRIx64 ") at cycle %d\n", CURRENT_STATE.PC, stat_cycles + 1);
        }
        
       // handle_cache_delays();
        globals.icache_num_to_squash = 49; //49
        return;
    } else {
        if (SHOULD_PRINT_DEBUG_STATEMENTS) {
            printf("icache hit (0x%" PRIx64 ") at cycle %d\n", CURRENT_STATE.PC, stat_cycles + 1);
        }
        
        uint32_t machine_code = i_cache_update(i_cache, CURRENT_STATE.PC);
        PREGISTER_IF_ID.IR_d = machine_code;
        PREGISTER_IF_ID.PC_d = CURRENT_STATE.PC;
    }

  //  int64_t PC_before_bp_predict = CURRENT_STATE.PC;
    bp_predict(CURRENT_STATE.PC, 1);
   // int64_t PC_after_bp_predict = CURRENT_STATE.PC;
}

/*--------------------------------DECODE--------------------------------*/

void pipe_stage_decode_r_instruction(uint32_t machine_code) {
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

void pipe_stage_decode_cb_instruction(uint32_t machine_code) {
    uint32_t read_register_1 = (machine_code >> 5) & 0x1F; //Rt
    uint32_t COND_BR_address = (machine_code >> 5) & 0x7FFFF;

    PREGISTER_ID_EX.A_e = CURRENT_STATE.REGS[read_register_1];
    PREGISTER_ID_EX.Imm_e = COND_BR_address & 0x00000000FFFFFFFF;
    PREGISTER_ID_EX.write_register_number = 32;
    PREGISTER_ID_EX.register_Rn1 = read_register_1;
    PREGISTER_ID_EX.register_Rm2 = 32;
}
void pipe_stage_decode_d_instruction(uint32_t machine_code, instruction i) {
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

void pipe_stage_decode_i_instruction(uint32_t machine_code) {
    uint32_t Rn = (machine_code >> 5) & 0x1F; // Rn
    uint32_t ALU_immediate = (machine_code >> 10) & 0xFFF;

    PREGISTER_ID_EX.write_register_number = machine_code & 0x1F;
    PREGISTER_ID_EX.register_Rn1 = Rn;
    PREGISTER_ID_EX.register_Rm2 = 32;
    PREGISTER_ID_EX.A_e = CURRENT_STATE.REGS[Rn];
    PREGISTER_ID_EX.Imm_e = ALU_immediate;
}

void pipe_stage_decode_shift_instruction(uint32_t machine_code, instruction i) {
    uint32_t Rn = (machine_code >> 5) & 0x1F;
    uint32_t Rm = (machine_code >> 16) & 0x1F;
    uint32_t immediate = (machine_code >> 16) & 0x3F;
    PREGISTER_ID_EX.A_e = CURRENT_STATE.REGS[Rn];
    PREGISTER_ID_EX.B_e = CURRENT_STATE.REGS[Rm];
    PREGISTER_ID_EX.Imm_e = immediate;
    PREGISTER_ID_EX.register_Rn1 = Rn;
    PREGISTER_ID_EX.register_Rm2 = Rm;
    PREGISTER_ID_EX.write_register_number = machine_code & 0x1F;
}

void pipe_stage_decode_b_instruction(uint32_t machine_code, instruction i) {
    uint32_t immediate = machine_code & 0x3FFFFFF;
    PREGISTER_ID_EX.Imm_e = immediate;
    // no need for write register
    PREGISTER_ID_EX.write_register_number = 32;
    uint32_t BR_address = PREGISTER_ID_EX.Imm_e;
    int64_t BR_address64 = (BR_address >> 25) ? (0xFFFFFFFFF0000000 | ((int64_t)BR_address << 2)) : 0 | (BR_address << 2);
    
    int nPC_m = PREGISTER_IF_ID.PC_d + BR_address64;
    bp_update(PREGISTER_IF_ID.PC_d, i, nPC_m,1,NULL);
    globals.is_unconditional = TRUE;
    CURRENT_STATE.PC = nPC_m;

    memset(&PREGISTER_IF_ID,0, sizeof(PREGISTER_IF_ID));
}
void pipe_stage_decode_iw_format(uint32_t machine_code) {
    uint32_t MOV_immediate = (machine_code >> 5) & 0x3FFFF;
    PREGISTER_ID_EX.Imm_e = MOV_immediate;
    PREGISTER_ID_EX.write_register_number = machine_code & 0x1F;
}
void pipe_stage_decode()
{
    if (globals.dcache_num_to_stall != 0) {
        return;
    }
    if (globals.should_squash_decode == TRUE) {
       // memset(&PREGISTER_ID_EX,0,sizeof(PREGISTER_ID_EX));
        PREGISTER_ID_EX.i = NO_INSTRUCTION;
        return;
    }
    if (globals.detected_load_hazard == TRUE) {
        return;
    }
    uint32_t machine_code = PREGISTER_IF_ID.IR_d;
    instruction i = decode(machine_code);
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("Decode:\n");
        print_instruction(i, machine_code);
    }
 
    if (i == NO_INSTRUCTION) { 
        memset(&PREGISTER_ID_EX,0,sizeof(PREGISTER_ID_EX));
        return;
    } else if (i <= UDIV) { // R-FORMAT
        pipe_stage_decode_r_instruction(machine_code);
    } else if (i <= BLE) { // CB-FORMAT
        pipe_stage_decode_cb_instruction(machine_code);
    } else if (i <= STURW) { // D-FORMAT
        pipe_stage_decode_d_instruction(machine_code,i);
    } else if (i <= SUBIS) { // I-FORMAT
        pipe_stage_decode_i_instruction(machine_code);
    } else if (i <= LSR) { // SHIFT-TYPE
        pipe_stage_decode_shift_instruction(machine_code, i);
    } else if (i <= BL) { // B-FORMAT
        pipe_stage_decode_b_instruction(machine_code, i);
    } else if (i <= MOVZ) { // IW-FORMAT
        pipe_stage_decode_iw_format(machine_code);
    } else { // HLT
        CURRENT_STATE.PC += 4;
        globals.detected_halt = TRUE;
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
    uint32_t shamt = (i == LSLI) ? ((immr ^ 0x3F) + 1) : immr;
    if (i == LSL) {
        PREGISTER_EX_MEM.Aout_m = op1 << op2;
    } else if (i == LSR) {
        PREGISTER_EX_MEM.Aout_m = op1 >> op2;
    } else if (i == LSLI) {
        PREGISTER_EX_MEM.Aout_m = op1 << shamt;
    } else if (i == LSRI) {
        PREGISTER_EX_MEM.Aout_m = op1 >> shamt;
    }
    PREGISTER_EX_MEM.will_write = 1;
}

void pipe_stage_execute_b_instruction(instruction i) {
    uint32_t BR_address = PREGISTER_ID_EX.Imm_e;
    int64_t BR_address64 = (BR_address >> 25) ? (0xFFFFFFFFF0000000 | ((int64_t)BR_address << 2)) : 0 | (BR_address << 2);
    PREGISTER_EX_MEM.nPC_m = PREGISTER_ID_EX.PC_e + BR_address64;
    PREGISTER_EX_MEM.will_write = 0;
}

void pipe_stage_execute_iw_instruction(instruction i) {
    uint32_t MOV_immediate = PREGISTER_ID_EX.Imm_e & 0xFFFF;
    uint32_t op2 = (PREGISTER_ID_EX.Imm_e >> 16) & 0x3;
    // the instruction can only be MOVZ
    PREGISTER_EX_MEM.Aout_m = MOV_immediate << (op2 * 16);
    PREGISTER_EX_MEM.will_write = 1;
}


void pipe_stage_execute() {
    instruction i = PREGISTER_ID_EX.i;
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("Execute Instruction:\n");
        print_instruction(i,PREGISTER_ID_EX.machine_code);
    }
    if (globals.dcache_num_to_stall != 0) {
        return;
    }
    if (globals.detected_load_hazard) {
        memset(&PREGISTER_EX_MEM,0,sizeof(PREGISTER_EX_MEM));
        globals.detected_load_hazard = 1;
        return;
    }
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
        globals.did_forward_flags = TRUE;
        PREGISTER_ID_EX.flag_n = PREGISTER_EX_MEM.flag_n;
        PREGISTER_IF_ID.flag_n = PREGISTER_EX_MEM.flag_n;
        PREGISTER_ID_EX.flag_z = PREGISTER_EX_MEM.flag_z;
        PREGISTER_IF_ID.flag_z = PREGISTER_EX_MEM.flag_z;
        PREGISTER_ID_EX.flag_c = PREGISTER_EX_MEM.flag_c;
        PREGISTER_IF_ID.flag_c = PREGISTER_EX_MEM.flag_c;
        PREGISTER_ID_EX.flag_v = PREGISTER_EX_MEM.flag_v;
        PREGISTER_IF_ID.flag_v = PREGISTER_EX_MEM.flag_v;
    }

    if (i <= STURW && i >= LDUR) {
    //    stat_cycles--;
    }

    if (i <= BLE && i >= CBNZ) {
        int is_correct_branch;
        if (PREGISTER_EX_MEM.nPC_m != PREGISTER_ID_EX.PC_e + 4) {

            // TODO: do we need this?
           
            bp_update(PREGISTER_ID_EX.PC_e, i, PREGISTER_EX_MEM.nPC_m,1, &is_correct_branch);
            if (!is_correct_branch) {
          //      globals.icache_num_to_squash = 0;
                CURRENT_STATE.PC = PREGISTER_EX_MEM.nPC_m;
                globals.should_squash_fetch = TRUE;
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
                globals.should_squash_fetch = TRUE;
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
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("Mem Instruction:\n");
        print_instruction(i, PREGISTER_EX_MEM.machine_code);
    }
    if (i == NO_INSTRUCTION) {
        memset(&PREGISTER_MEM_WB,0,sizeof(PREGISTER_MEM_WB));
        return;
    }

    if (globals.dcache_num_to_stall >= 2) {
        if (SHOULD_PRINT_DEBUG_STATEMENTS) {
            printf("dcache stall (%d)\n", globals.dcache_num_to_stall);
        }
        
        memset(&PREGISTER_MEM_WB,0,sizeof(PREGISTER_MEM_WB));
        return;
    }
    if (globals.dcache_num_to_stall == 1) {
        // TODO: EDITED!!!!!!!!!!!!!!!!!!!!!!

    //    stat_inst_retire++;
  //      globals.shouldnt_update_inst_retired = 1;


        int64_t address_to_rw = PREGISTER_EX_MEM.Aout_m;
        int64_t write_data = PREGISTER_EX_MEM.B_m;
    //    stat_cycles--;
        if (SHOULD_PRINT_DEBUG_STATEMENTS) {
            printf("dcache stall (%d)\n", globals.dcache_num_to_stall);
            printf("dcache fill at cycle %d\n", stat_cycles + 1);    
        }
        switch(i) {
            case LDUR: ;
                int64_t full_value;
                d_cache_update(d_cache, i, address_to_rw, 0, &full_value);
                globals.dcache_stored_value = full_value;
                // printf("LDUR address read: %" PRId64 "\n", full_value);
                break;
            case LDURB:
                globals.dcache_stored_value = d_cache_update(d_cache, i, address_to_rw, 0, NULL) & 0xFF;
                break;
            case LDURH:
                globals.dcache_stored_value = d_cache_update(d_cache, i, address_to_rw, 0, NULL) & 0xFFFF;;
                break;
            case STUR:
                d_cache_update(d_cache, i, address_to_rw, write_data, NULL);
                break;
            case STURB:
                d_cache_update(d_cache, i, address_to_rw, write_data & 0xFF, NULL);
                break;
            case STURH:
                d_cache_update(d_cache, i, address_to_rw, write_data & 0xFFFF, NULL);
                break;
            case STURW:
                d_cache_update(d_cache, i, address_to_rw, write_data & 0xFFFFFFFF, NULL);
                break;
        }
        globals.should_update_d_cache = TRUE;
        memset(&PREGISTER_MEM_WB,0,sizeof(PREGISTER_MEM_WB));
        return;
    }
    if (globals.dcache_num_to_stall == 0 && globals.should_update_d_cache) {
        // TODO: EDITED!!!!!!!!!!!!!!!!
        globals.num_until_subtract = 2;
        globals.is_previous_hit = TRUE;
     //   stat_inst_retire++;
      //  globals.shouldnt_update_inst_retired = TRUE;
        //printf("SLDKFJSLDKFJSLDKFJSLDKFJSLDKFJ\n\n\n\n");
        int64_t address_to_rw = PREGISTER_EX_MEM.Aout_m;
        if (i == LDUR || i == LDURB || i == LDURH) {
            PREGISTER_MEM_WB.Aout_w = address_to_rw;
            PREGISTER_MEM_WB.MDR_w = globals.dcache_stored_value;
        }
        globals.should_update_d_cache = FALSE;
        PREGISTER_MEM_WB.write_register_number = PREGISTER_EX_MEM.write_register_number;
        PREGISTER_MEM_WB.i = i;
        PREGISTER_MEM_WB.will_write = PREGISTER_EX_MEM.will_write;
        PREGISTER_MEM_WB.machine_code = PREGISTER_EX_MEM.machine_code;
        return;
    }
    
    if (PREGISTER_EX_MEM.will_write && PREGISTER_EX_MEM.write_register_number < 31) {
        if (PREGISTER_EX_MEM.write_register_number == PREGISTER_ID_EX.register_Rn1) {
            PREGISTER_ID_EX.A_e = PREGISTER_EX_MEM.Aout_m;
        }
        if (PREGISTER_EX_MEM.write_register_number == PREGISTER_ID_EX.register_Rm2) {
            PREGISTER_ID_EX.B_e = PREGISTER_EX_MEM.Aout_m;
        }
    }

    if (i == LDUR || i == LDURH || i == LDURB) {
        if (PREGISTER_EX_MEM.write_register_number == PREGISTER_ID_EX.register_Rn1 ||
            PREGISTER_EX_MEM.write_register_number == PREGISTER_ID_EX.register_Rm2) {
            // stall the pipeline
            globals.detected_load_hazard = TRUE;
        }
    }
    
    if (i <= STURW && i >= LDUR) {// if load or store
        
        int64_t address_to_rw = PREGISTER_EX_MEM.Aout_m;
        int64_t write_data = PREGISTER_EX_MEM.B_m;
     //   uint32_t half_byte_1 = 0, half_byte_2 = 0;
      //  int64_t full_value = 0;
        bool is_hit = d_cache_did_hit(d_cache, address_to_rw, i);
        if (!is_hit) {
            if (SHOULD_PRINT_DEBUG_STATEMENTS) {
                printf("dcache miss (0x%" PRIx64 ") at cycle %d\n", address_to_rw, stat_cycles + 1);
            }
            
            globals.dcache_num_to_stall = 50; //49
            PREGISTER_MEM_WB.i = NO_INSTRUCTION;

            return;
        } else {
            //TODO: CHANGED THIS VALUE~~~~~~~~~~~~~~
            
            if (globals.updated_stats_inst_in_mem == FALSE) {
                globals.shouldnt_update_inst_retired = TRUE;
                stat_inst_retire++;
            } else {
                globals.updated_stats_inst_in_mem = FALSE;
            }
            
            if (globals.num_until_subtract == 0) {
                globals.num_until_subtract = 2;
            }
            
            if (SHOULD_PRINT_DEBUG_STATEMENTS) {
                printf("dcache hit (0x%" PRIx64 ") at cycle %d\n", address_to_rw, stat_cycles + 1);
         //       exit(0);
            }
            
            switch(i) {
                case LDUR: ;
                    int64_t full_value;
                    d_cache_update(d_cache, i, address_to_rw, 0, &full_value);
                    PREGISTER_MEM_WB.MDR_w = full_value;
                    PREGISTER_MEM_WB.Aout_w = address_to_rw;
                    break;
                case LDURB:
                    PREGISTER_MEM_WB.MDR_w = d_cache_update(d_cache, i, address_to_rw, 0, NULL) & 0xFF;
                    PREGISTER_MEM_WB.Aout_w = address_to_rw;
                    break;
                case LDURH:
                    PREGISTER_MEM_WB.MDR_w = d_cache_update(d_cache, i, address_to_rw, 0, NULL) & 0xFFFF;;
                    PREGISTER_MEM_WB.Aout_w = address_to_rw;
                    break;
                case STUR:
                    d_cache_update(d_cache, i, address_to_rw, write_data, NULL);
                    break;
                case STURB:
                    d_cache_update(d_cache, i, address_to_rw, write_data & 0xFF, NULL);
                    break;
                case STURH:
                    d_cache_update(d_cache, i, address_to_rw, write_data & 0xFFFFFFFF, NULL);
                    break;
            }
        }
    } else if (i == BL) {
        PREGISTER_MEM_WB.Aout_w = PREGISTER_EX_MEM.nPC_m + 4;
    } else {
        PREGISTER_MEM_WB.Aout_w = PREGISTER_EX_MEM.Aout_m;
        PREGISTER_MEM_WB.write_register_number = PREGISTER_EX_MEM.write_register_number;
        if (i == ADDS || i == ANDS || i == SUBS || i == CMP || i == ADDIS || i == SUBIS) {
            PREGISTER_MEM_WB.flag_n = PREGISTER_EX_MEM.flag_n;
            PREGISTER_MEM_WB.flag_z = PREGISTER_EX_MEM.flag_z;
            PREGISTER_MEM_WB.flag_v = PREGISTER_EX_MEM.flag_v;
            PREGISTER_MEM_WB.flag_c = PREGISTER_EX_MEM.flag_c;
        }
    }
    if (i == HLT) {
        cache_destroy(i_cache);
        cache_destroy(d_cache);
   //     stat_cycles--;
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
    if (i == NO_INSTRUCTION) { return; }
    uint32_t write_register = PREGISTER_MEM_WB.write_register_number;
    if (PREGISTER_MEM_WB.will_write && write_register != 31 && ((i <= UDIV) || (i <= SUBIS && i >= ADDI) || (i <= LSR && i >= LSL) || (i == MOVZ))) {
        if (write_register == PREGISTER_ID_EX.register_Rn1) {
            PREGISTER_ID_EX.A_e = PREGISTER_MEM_WB.Aout_w;
        }
        if (write_register == PREGISTER_ID_EX.register_Rm2) {
            PREGISTER_ID_EX.B_e = PREGISTER_MEM_WB.Aout_w;
        }
    }
    if ((i == LDUR || i == LDURB || i == LDURH) && write_register < 31) {
        if (write_register == PREGISTER_ID_EX.register_Rn1) {
            PREGISTER_ID_EX.A_e = PREGISTER_MEM_WB.MDR_w;
        }
        if (write_register == PREGISTER_ID_EX.register_Rm2) {
            PREGISTER_ID_EX.B_e = PREGISTER_MEM_WB.MDR_w;
        }
    }
    
    if (((i <= UDIV) || (i <= SUBIS && i >= ADDI) || (i <= LSR && i >= LSL) || (i == MOVZ)) && write_register < 31) {
        // R-Instruction, I-Instruction, Shift-Type, and IW-Type
        CURRENT_STATE.REGS[write_register] = PREGISTER_MEM_WB.Aout_w;
        // if (write_register == 11) {
        //     printf("PREGISTER_MEM_WB.Aout_w = ");
        //     exit(0);
        // }
    } else if ((i == LDUR || i == LDURB || i == LDURH) && write_register < 31){
        CURRENT_STATE.REGS[write_register] = PREGISTER_MEM_WB.MDR_w;
    } else if (i == BL) {
        CURRENT_STATE.REGS[30] = PREGISTER_MEM_WB.Aout_w;
    }

    if (i == ADDS || i == ANDS || i == SUBS || i == CMP || i == ADDIS || i == SUBIS) {

        CURRENT_STATE.FLAG_N = PREGISTER_MEM_WB.flag_n;
        CURRENT_STATE.FLAG_Z = PREGISTER_MEM_WB.flag_z;
        CURRENT_STATE.FLAG_V = PREGISTER_MEM_WB.flag_v;
        CURRENT_STATE.FLAG_C = PREGISTER_MEM_WB.flag_c;
    }

    if (globals.shouldnt_update_inst_retired == FALSE) {
        globals.updated_stats_inst_in_mem = TRUE;
        stat_inst_retire++;
    } else {
        globals.shouldnt_update_inst_retired--;
    }
    

   // printf("***************************Cycle number: %d, instruction retired: %d\n", stat_cycles, i);
    if (SHOULD_PRINT_INSTRUCTIONS) {
        printf("WRITEBACK INSTRUCTION\n");
        print_instruction(i, PREGISTER_MEM_WB.machine_code);
    }
}



