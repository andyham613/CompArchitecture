/*
 * CMSC 22200, Fall 2016
 *
 * ARM pipeline timing simulator
 *
 * Reza Jokar, Gushu Li, 2016
 */


#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include <limits.h>

//typedef int bool;
#define TRUE 1
#define FALSE 0

typedef enum instruction {
    NO_INSTRUCTION,//0
    //R type
    ADD,//1
    ADDS,//2
    AND,//3
    ANDS,//4
    BR,//5
    EOR,//6
    MUL,//7
    ORR,//8
    SDIV,//9
    SUB,//10
    SUBS,//11
    CMP,//12
    UDIV,//13
    //CB-type
    CBNZ,//14
    CBZ,//15
    BEQ,//16
    BNE,//17
    BGT,//18
    BLT,//19
    BGE,//20
    BLE,//21
    // D-type
    LDUR,//22
    LDURB,//23
    LDURH,//24
    STUR,//25
    STURB,//26
    STURH,//27
    STURW,//28
    // I-type
    ADDI,//29
    ADDIS,//30
    SUBI,//31
    SUBIS,//32
    // Shift-Type
    LSL, //33
    LSLI,//34
    LSRI,//35
    LSR,//36
    // B-type
    B,//37
    BL,//38
    // IW-type
    MOVZ,//39
    // HLT
    HLT,//40
    //ERET
    ERET//41
} instruction;

typedef struct Pipe_Reg_IFtoID {
    int64_t PC_d;
    uint32_t IR_d;
    uint32_t flag_n;
    uint32_t flag_z;
    uint32_t flag_v;
    uint32_t flag_c;
 //   uint32_t detected_halt;
   // uint32_t did_set_flags;
    // uint32_t num_instructions_to_squash;
    // uint32_t should_squash_decode;
} Pipe_Reg_IFtoID;

typedef struct Pipe_Reg_IDtoEX {
    int64_t PC_e;
    int64_t A_e;
    int64_t B_e;
    uint32_t Imm_e;
    uint32_t write_register_number;
    uint32_t flag_n;
    uint32_t flag_z;
    uint32_t flag_v;
    uint32_t flag_c;
    uint32_t register_Rn1;
    uint32_t register_Rm2;
    uint32_t machine_code;
    instruction i;
} Pipe_Reg_IDtoEX;

typedef struct Pipe_Reg_EXtoMEM {
    int64_t nPC_m;
    int64_t Aout_m;
    int64_t B_m;
    uint32_t write_register_number;
    uint32_t flag_n;
    uint32_t flag_z;
    uint32_t flag_v;
    uint32_t flag_c;
    uint32_t will_write;
    uint32_t machine_code;
    instruction i;
} Pipe_Reg_EXtoMEM;

typedef struct Pipe_Reg_MEMtoWB {
    int64_t MDR_w;
    int64_t Aout_w;
    uint32_t write_register_number;
    uint32_t flag_n;
    uint32_t flag_z;
    uint32_t flag_v;
    uint32_t flag_c;
    uint32_t will_write;
    uint32_t machine_code;
    instruction i;
} Pipe_Reg_MEMtoWB;

typedef struct CPU_State {
    /* register file state */
    int64_t REGS[ARM_REGS];
    int FLAG_N;        /* flag N */
    int FLAG_Z;        /* flag Z */
    int FLAG_V;        /* flag V */
    int FLAG_C;        /* flag C */
    /* program counter in fetch stage */
    uint64_t PC;
} CPU_State;

typedef struct global_vars {
    uint32_t dcache_num_to_stall;
    uint32_t icache_num_to_squash;
    uint32_t machine_code;
    int64_t dcache_stored_value;
    bool is_unconditional;
    bool should_squash_fetch;
    bool detected_load_hazard;
    bool detected_halt;
    bool did_forward_flags;
    bool should_squash_decode;
    bool should_update_i_cache;
    bool should_update_d_cache;
} global_vars;

//TODO: RUN_BIT should have 3 more for diff cpus too?
int RUN_BIT, RUN_BIT_1, RUN_BIT_2, RUN_BIT_3;

Pipe_Reg_IFtoID PREGISTER_IF_ID, PREGISTER_IF_ID_1, PREGISTER_IF_ID_2, PREGISTER_IF_ID_3;
Pipe_Reg_IDtoEX PREGISTER_ID_EX, PREGISTER_ID_EX_1, PREGISTER_ID_EX_2, PREGISTER_ID_EX_3;
Pipe_Reg_EXtoMEM PREGISTER_EX_MEM, PREGISTER_EX_MEM_1, PREGISTER_EX_MEM_2, PREGISTER_EX_MEM_3;
Pipe_Reg_MEMtoWB PREGISTER_MEM_WB, PREGISTER_MEM_WB_1, PREGISTER_MEM_WB_2, PREGISTER_MEM_WB_3;

global_vars globals, globals_1, globals_2, globals_3;
/* global variable -- pipeline state */
extern CPU_State CURRENT_STATE, CURRENT_STATE_1, CURRENT_STATE_2, CURRENT_STATE_3;

/* called during simulator startup */
void pipe_init();

/* this function calls the others */
void pipe_cycle();

/* each of these functions implements one stage of the pipeline */
void pipe_stage_fetch();
void pipe_stage_decode();
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

void pipe_stage_fetch_1();
void pipe_stage_decode_1();
void pipe_stage_execute_1();
void pipe_stage_mem_1();
void pipe_stage_wb_1();

void pipe_stage_fetch_2();
void pipe_stage_decode_2();
void pipe_stage_execute_2();
void pipe_stage_mem_2();
void pipe_stage_wb_2();

void pipe_stage_fetch_3();
void pipe_stage_decode_3();
void pipe_stage_execute_3();
void pipe_stage_mem_3();
void pipe_stage_wb_3();

#endif

/*
LAB 5 
Here's whats up.

MULTICORE SYSTEM

System has 4 CPUs (0 - 3)
All CPUs have separate/private states, (registers,PC,branch predict,L1 cache)
CPUs only share memory. No need to worry about cache coherence
CPU 0 runs only first on single threaded program.
CPUs simulated in ascending order; CPU 0 cycle 0 first, then CPU 1 cycle 0, etc

TO USE DIFFERENT CPU
call ERET
    machine code = 0xd69f03e0
    1101 0110 1001 1111 0000 0011 1110 0000
whatever CPU that calls this code, the value in its X30 register denotes which new CPU
the NEW CPU is set to PC + 4 of the OLD cpu 
the NEW CPU's X29 is set to 1, the OLD CPU's X29 set to 0
ERET instruction serializes the pipeline
    when ERET is in decode, must stall until any instr in ex, mem, wb finish
    when ERET is in ex, mem, wb, the two instr FOLLOWING must stall in if, de until ERET leaves
    ERET's actions takes EFFECT at WRITEBACK stage

SHELL changes
go: all CPUS run until halted
    HLT instr itself halts only the current CPU that is executed on
i X Y: set register X on CPU 0 to value Y
rdump: Dump register state for all four CPUs.
*/
