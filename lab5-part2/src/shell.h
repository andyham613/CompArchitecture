/***************************************************************/
/*                                                             */
/*   ARM Instruction Level Simulator                       */
/*                                                             */
/*   CMSC-22200 Computer Architecture                                            */
/*   University of Chicago                                */
/*                                                             */
/***************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*          DO NOT MODIFY THIS FILE!                            */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#ifndef _SIM_SHELL_H_
#define _SIM_SHELL_H_

#include <stdint.h>

#define FALSE 0
#define TRUE  1
#define ARM_REGS 32

/* only the cache touches these functions */
uint32_t mem_read_32(uint64_t address);
void     mem_write_32(uint64_t address, uint32_t value);

/* statistics */
extern uint32_t stat_cycles, stat_inst_retire, stat_inst_fetch, stat_squash;
//, stat_cycles_1, stat_inst_retire_1, stat_inst_fetch_1, stat_squash_1, stat_cycles_2, stat_inst_retire_2, stat_inst_fetch_2, stat_squash_2, stat_cycles_3, stat_inst_retire_3, stat_inst_fetch_3, stat_squash_3;

#endif
