/*MSC 22200, Fall 2016
 *
 * ARM pipeline timing simulator
 *
 */

#include "cache.h"
#include <stdlib.h>
#include <stdio.h>
#include "pipe.h"
#include <inttypes.h>

cache_t *cache_new(uint32_t sets_per_cache, uint32_t lines_per_set, uint32_t bytes_per_block) {
    cache_t* output_cache = (cache_t*)malloc(sizeof(cache_t));
    output_cache->sets_per_cache = sets_per_cache;
    output_cache->lines_per_set = lines_per_set;
    output_cache->words_per_block = bytes_per_block / 4;
    output_cache->lines = (cache_line**)malloc(sets_per_cache * sizeof(cache_line*));
    output_cache->results.hits = 0;
    output_cache->results.misses = 0;
    output_cache->results.evicts = 0;
    int i;
    for (i = 0; i < sets_per_cache; i++) {
        output_cache->lines[i] = (cache_line*)malloc(lines_per_set * sizeof(cache_line));
        int j;
        for (j = 0; j < lines_per_set; j++) {
            output_cache->lines[i][j].valid_bit = 0;
            output_cache->lines[i][j].dirty_bit = 0;
            output_cache->lines[i][j].tag = 0;
            output_cache->lines[i][j].LRUcounter = 0;
            output_cache->lines[i][j].blocks = (uint32_t*)malloc((bytes_per_block / 4) * sizeof(uint32_t));
            int k;
            for (k = 0; k < bytes_per_block / 4; k++) {
                output_cache->lines[i][j].blocks[k] = 0;
            }
        }
    }
    return output_cache;
}

void cache_destroy(cache_t *c)
{
    uint32_t sets_per_cache = c->sets_per_cache, lines_per_set = c->lines_per_set;
    int i;
    for (i = 0; i < sets_per_cache; i++) {
        int j;
        for (j = 0; j < lines_per_set; j++) {
            free(c->lines[i][j].blocks);
        }
        free(c->lines[i]);
    }
    free(c->lines);
    free(c);
}


uint32_t index_of_max_value(int* array, uint32_t n) {
    // returns the index of the maximum value in the array
    uint32_t max_index = 0, max_value = 0;
    uint32_t i;
    for (i = 0; i < n; i++) {
        if (array[i] > max_value) {
            max_value = array[i];
            max_index = i;
        }
    }
    return max_index;
}

uint32_t* blockbuilder(uint64_t addr) { //given og pc address, calculate the 32 bit instructions of the 7 next instructions.
    uint32_t i = 0;
    uint64_t tmpaddr = addr;
    uint32_t* myArray = (uint32_t*)malloc(sizeof(uint32_t) * 8);
    for (i = 0; i < 8; i++, tmpaddr += 4) {
        myArray[i] = mem_read_32(tmpaddr);
    }
    return myArray;
}

bool i_cache_did_hit(cache_t *c, uint64_t addr) {
    uint32_t set_index = (addr >> 5) & 0x3F; //set index
    uint64_t tag = (addr >> 11) & 0x1FFFFFFFFFFFFF; //addre for ic is CURRENT STATE PC 
    uint32_t num_blocks = c->lines_per_set;
    uint32_t line_index;
    for (line_index = 0; line_index < num_blocks; line_index++) {
        if (c->lines[set_index][line_index].valid_bit == 0) {// cache empty, update in first available empty slot
            return FALSE;
        } else if (c->lines[set_index][line_index].tag == tag) {// if cache hit
            return TRUE;
        } 
    }
    return FALSE;
}

uint32_t i_cache_update(cache_t *c, uint64_t addr) {
    //printf("i_cache\n");
    uint32_t set_index = (addr >> 5) & 0x3F; //set index
    uint64_t tag = (addr >> 11) & 0x1FFFFFFFFFFFFF; //addre for ic is CURRENT STATE PC 
    uint32_t byteoffset = (addr & 0x1F) / 4;
    uint32_t* blockArray = blockbuilder(addr);
    uint32_t num_blocks = c->lines_per_set;
    uint32_t line_index;
    //printf("before first for loop\n");
    for (line_index = 0; line_index < num_blocks; line_index++) {
        c->lines[set_index][line_index].LRUcounter++;
    } // increment lru counter for ALL entries in this set. only one of them will be used at most, and when it hits, we set lru=0 for that one.
    //printf("after first for loop\n");
    //printf("%d\n", i_cache->lines_per_set);
    for (line_index = 0; line_index < num_blocks; line_index++) { // traverses through the different associativity
        //printf("%d\n", num_blocks);

        if (c->lines[set_index][line_index].valid_bit == 0) {// cache empty, update in first available empty slot
            c->lines[set_index][line_index].LRUcounter = 0;
            c->lines[set_index][line_index].valid_bit = 1;
            c->lines[set_index][line_index].tag = tag;
            c->lines[set_index][line_index].blocks = blockArray; //TODO: global_block is determined in decode in pipe.c where we turn 64bit PC -> 32 bit instr. for the 7 next sequential addresses too.
            c->results.misses++;
            c->results.most_recent_type = MISS;
            //printf("icache miss (%" PRIx64 ") at cycle %d\n", addr, stat_cycles);

        //    printf("finished icache 1\n\n");
            return c->lines[set_index][line_index].blocks[byteoffset]; //end here since we updated the cache
        } else if (c->lines[set_index][line_index].tag == tag) {// if cache hit

            c->lines[set_index][line_index].LRUcounter = 0;
            c->results.hits++;
            c->results.most_recent_type = HIT;
            //printf("icache hit (%" PRIx64 ") at cycle %d\n", addr, stat_cycles);
            //printf("finished icache 2\n\n");
            return c->lines[set_index][line_index].blocks[byteoffset]; // cache hit! do nothing. or return the requested block?
        } 
    } 
    //printf("icache miss (%" PRIx64 ") at cycle %d\n", addr, stat_cycles);

    // cache is full, need to evict using lru
    uint32_t lru_array[num_blocks];
    int i;
    for (i = 0; i < num_blocks; i++) {
        lru_array[i] = c->lines[set_index][i].LRUcounter;
    }

    uint32_t evicted_index = index_of_max_value(lru_array, num_blocks);
    c->lines[set_index][evicted_index].valid_bit = 1;
    c->lines[set_index][evicted_index].tag = tag;
    c->lines[set_index][evicted_index].blocks = blockArray; //gotta retrieve from memory the data
    c->lines[set_index][evicted_index].LRUcounter = 0;
    c->results.misses++;
    c->results.most_recent_type = MISS;
    //printf("finished icache 3\n\n");
    return c->lines[set_index][evicted_index].blocks[byteoffset];
    // TODO: also need to handle branch control flow stuff.
}

bool d_cache_did_hit(cache_t *c, uint64_t addr, instruction inst) {
        // returns the lower 32 bits
    // if the instruction requires 64 bits, returns 32 bits in out parameter
    // call cache_update(DEST_ADDR, 1); in Mem stage of pipeline.
    uint32_t set_index = (addr >> 5) & 0xFF;  // addr for this is the DESTINATION address that's calculated in the ex stage of pipeline.
    uint64_t tag = (addr >> 13) & 0x7FFFFFFFFFFFF;
    uint32_t byte_offset = addr & 0x1F;
    
    //PSEUDO CODE
    if (inst >= LDUR && inst <= LDURH) { //read memory request
        int i = 0;
        for (i = 0; i < 8; i++) { // did cache hit?
            if (c->lines[set_index][i].valid_bit == 1 && c->lines[set_index][i].tag == tag) {
                return TRUE;
            }
        }
        return FALSE;
    } else { // STORE, write request
        int i = 0;
        for (i = 0; i < 8; i++) { // did cache hit?
            if (c->lines[set_index][i].valid_bit == 1 && c->lines[set_index][i].tag == tag) {
                return TRUE;
            }
        }
        return FALSE;
    }
}


int64_t d_cache_update(cache_t *c, instruction inst, uint64_t addr, int64_t new_store_data, int64_t* my_output) {
    // returns the lower 32 bits
    // if the instruction requires 64 bits, returns 32 bits in out parameter
    // call cache_update(DEST_ADDR, 1); in Mem stage of pipeline.
    uint32_t set_index = (addr >> 5) & 0xFF;  // addr for this is the DESTINATION address that's calculated in the ex stage of pipeline.
    uint64_t tag = (addr >> 13) & 0x7FFFFFFFFFFFF;
    uint32_t byte_offset = addr & 0x1F;
    
    //PSEUDO CODE
    if (inst >= LDUR && inst <= LDURH) { //read memory request
        int i = 0;
        for (i = 0; i < 8; i++) { //lru counter everything that's valid/in the cache
            c->lines[set_index][i].LRUcounter++;
        }
        for (i = 0; i < 8; i++) { // did cache hit?
            if (c->lines[set_index][i].valid_bit == 1 && c->lines[set_index][i].tag == tag) {
                c->results.hits++;
                c->results.most_recent_type = HIT;

                // printf("dcache hit (0x%" PRIx64 ") at cycle %d\n", addr, stat_cycles + 1);
            //    exit(1);
                int64_t second_word = c->lines[set_index][i].blocks[byte_offset/4] & 0x00000000FFFFFFFF;
                if (inst == LDUR) {
                    int64_t first_word = (((int64_t) c->lines[set_index][i].blocks[byte_offset/4 + 1]) << 32) & 0xFFFFFFFF00000000;
                    int64_t combined = first_word | second_word;
                    *my_output = combined;
                    //printf("Combined: %" PRIx64 "\n\n", combined);
                    return combined;
                } else {
                    return second_word;
                }
            }
        }
        // printf("dcache miss (0x%" PRIx64 ") at cycle %d\n", addr, stat_cycles + 1);
        for (i = 0; i < 8; i++) {
            if (c->lines[set_index][i].valid_bit == 0) { // if cache empty, no need to check for dirty bit
                uint32_t* blockArray = blockbuilder(addr); // read from lower memory of dest address
                c->lines[set_index][i].valid_bit = 1;
                c->lines[set_index][i].dirty_bit = 0;
                c->lines[set_index][i].tag = tag;
                c->lines[set_index][i].blocks = blockArray;

                // TODO: this may cause a seg fault
                free(blockArray);
                c->lines[set_index][i].LRUcounter = 0;
                c->results.misses++;
                c->results.most_recent_type = MISS;
                return c->lines[set_index][i].blocks[byte_offset / 4]; //or should it be blocks[0] the first element?
            }
        }

        uint32_t lru_array[8];

        for (i = 0; i < 8; i++) {
            lru_array[i] = c->lines[set_index][i].LRUcounter;
        }
         //if cache full and tags dont match. need to evict
        uint32_t evicted_index = index_of_max_value(lru_array, 8); //we evict this old cache block
    //    exit(1);
        if (c->lines[set_index][evicted_index].dirty_bit == 1) {
            int block_index;
            for (block_index = 0; block_index < 8; block_index++) { // writes the cache data in to lower memory
                mem_write_32(addr, c->lines[set_index][evicted_index].blocks[block_index]);
        //        printf("writing back to memory in load\n");
                //exit(1);
            }
            uint32_t* blockArray= blockbuilder(addr); // read back from lower memory of dest address
            c->lines[set_index][evicted_index].valid_bit = 1;
            c->lines[set_index][evicted_index].dirty_bit = 0;
            c->lines[set_index][evicted_index].tag = tag;
            c->lines[set_index][evicted_index].blocks = blockArray;
            c->lines[set_index][evicted_index].LRUcounter = 0;
            c->results.misses++;
            c->results.most_recent_type = MISS;
            return c->lines[set_index][evicted_index].blocks[byte_offset/4];
        }

    } else { // STORE, write request
        int i = 0;

        for (i = 0; i < 8; i++) { //lru counter everything that's valid/in the cache
            if (c->lines[set_index][i].valid_bit == 1) {
                c->lines[set_index][i].LRUcounter++;
            }
        }
        for (i = 0; i < 8; i++) { // did cache hit?
            if (c->lines[set_index][i].valid_bit == 1 && c->lines[set_index][i].tag == tag) {
                c->results.hits++;
                c->results.most_recent_type = HIT;
                // writing new data into cache block
                if (inst == STUR) {
                    c->lines[set_index][i].blocks[byte_offset/4 + 1] = (uint32_t)((new_store_data >> 32) & 0xFFFFFFFF);
                    c->lines[set_index][i].blocks[byte_offset/4] = (uint32_t)(new_store_data & 0xFFFFFFFF);
                } else {
                    c->lines[set_index][i].blocks[byte_offset / 4] = (uint32_t)new_store_data;
                }
        //        printf("dcache hit (%" PRIx64 ") at cycle %d\n", addr, stat_cycles);
                c->lines[set_index][i].dirty_bit = 1;
                return 0;
            }
        }
        //cache miss
        //cache empty, so not dirty

        //printf("dcache miss (%" PRIx64 ") at cycle %d\n", addr, stat_cycles);
        for (i = 0; i < 8; i++) {
            if (c->lines[set_index][i].valid_bit == 0) { // if cache empty, no need to check for dirty bit
                uint32_t* blockArray = blockbuilder(addr); // read from lower memory of dest address
                int k;
                for (k = 0; k < 8; k++) {
                    if (blockArray[k] != 0) {
                        printf("%d [%d] %d\n",inst,k,blockArray[k]);
                    //    exit(1);
                    }
                    

                }
                
                c->lines[set_index][i].valid_bit = 1;
                c->lines[set_index][i].dirty_bit = 1; // need to set dirty bit 1 anyways later cuz we writing into cache block
                c->lines[set_index][i].tag = tag;
                c->lines[set_index][i].blocks = blockArray;
                c->lines[set_index][i].LRUcounter = 0;
                c->results.misses++;
                c->results.most_recent_type = MISS;
                if (inst == STUR) {
                    c->lines[set_index][i].blocks[byte_offset/4 + 1] = (uint32_t)((new_store_data >> 32) & 0xFFFFFFFF);
                    c->lines[set_index][i].blocks[byte_offset/4] = (uint32_t)(new_store_data & 0xFFFFFFFF);
                } else {
                    c->lines[set_index][i].blocks[byte_offset / 4] = (uint32_t)new_store_data;
                }
                return 0; 
            }
        }

        //cache full, but miss, need to evict, and check for dirty
    //    exit(1);
        uint32_t lru_array[8];

        for (i = 0; i < 8; i++) {
            lru_array[i] = c->lines[set_index][i].LRUcounter;
        }
         //if cache full and tags dont match. need to evict
        uint32_t evicted_index = index_of_max_value(lru_array, 8); //we evict this old cache block

        int block_index;
        if (c->lines[set_index][evicted_index].dirty_bit == 1) {
            for(block_index = 0; block_index < 8; block_index++) { // writes the cache data in to lower memory
                mem_write_32(addr, c->lines[set_index][evicted_index].blocks[block_index]);
                if (inst == STUR) {
                    mem_write_32(addr + 4, c->lines[set_index][evicted_index].blocks[block_index + 1]);
                }
            }
        }
        int* blockArray = blockbuilder(addr); // read back from lower memory of dest address
        c->lines[set_index][evicted_index].valid_bit = 1;
        c->lines[set_index][evicted_index].dirty_bit = 1;
        c->lines[set_index][evicted_index].tag = tag;
        c->lines[set_index][evicted_index].blocks = blockArray;
        c->lines[set_index][evicted_index].LRUcounter = 0;
        c->results.misses++;
        c->results.most_recent_type = MISS;
        if (inst == STUR) {
            int offset = byte_offset / 4;
            if (offset >= 0) {
                c->lines[set_index][evicted_index].blocks[offset + 1] = (uint32_t)((new_store_data >> 32) & 0xFFFFFFFF);
                c->lines[set_index][evicted_index].blocks[offset] = (uint32_t)(new_store_data & 0xFFFFFFFF);
            }
        } else {
            c->lines[set_index][i].blocks[byte_offset / 4] = (uint32_t)new_store_data;
        }
        return 0; 
    }
}



// uint32_t cache_update(cache_t *c, uint64_t addr, uint32_t is_i_cache)
// {
    
    
//     //for instruction cache
//     uint32_t cache_result = 0;
//     if (is_i_cache) {
//         cache_result = i_cache_update(c, addr);
//     } else {
//         cache_result = d_cache_update(addr);
//     }

//     return cache_result;

// }

/******************* INSTRUCTION CACHE *************************/
/*
Instruction cache is accessed every cycle in FETCH stage (except when stalled)

Specs:
4-way set associative
8KB size
32 Byte blocks
64 sets

set_indexndex = (PC >> 4) & 0x1F;

WHEN MISSED:
The block must be retrieved from main memory, takes 50 cycles.
So, stall the pipeline for 50 cycles. IF stage completed at the 51st cycle

REPLACEMENT:
When missed, insert the new block in the appropriate set.
If blocks are full, new block replaces the LEAST-RECENTLY-USED block.

CONTROL-FLOW:
If instruction cache miss on a CF instr, and if control flow is taken further down,
 stalling/fetching the new block unnecessary, since we REDIRECT the PC.
Cancel pending miss since we would be retrieving the WRONG block. 
*a redirction that accesses the same block as a pending miss does NOT cancel the pending miss




*/
/******************* DATA CACHE *************************/
/*
Data cache is accessed only for LOAD and STORE instr in the MEM stage

specs:
8-way associative
64KB size
32 byte blocks
256 sets

dc_set_indexndex = (PC >> 4) & 0x7F;

MISSED AND REPLACEMENT:
same as instruction cache

Handling LOAD/STORE:
both instr misses stall the pipeline for 50 cycles
Both retrieve a new block from main memory and insert it into cache
Mem stage completed at the 51st cycle

Dirty Evictions:
Dirty block is replaced by new block from main memory, then is written BACK into main memory.



*/


/*
set dirty bit whenever cache is updated?
Two ways 
HIT:
1) Read: great, the data we want is in the cache. return that data in the block
2) Write: great! but we still need to write data into the cache. write through into the block. set dirty bit

MISS:
1) Read: uh oh, we need to allocate memory from lower portion, bring it up and update this new cache. return new block
2) Write: We need to bring the data from lower memory, and then write into this new cache entry. 
this is called write allocate.

*/

// actual 1 block are 32 bytes long. one instru is 4 bytes.
// the first 4 bytes in a block is the INSTRUCTION associated with the PC. add 7 sequential instructions. after.


/* for data cache
When will block be dirty? Only when you write; reading doesnt cause block to be dirty. 
So if block is dirty, we need to memwrite all the instr/data in block INTO RAM, which is lower memory.
Then we take the stuff from lower memory and load it back into the cache, thereby getting the correct data and setting dirty bit to 0
Now we can modify the values of that cache block, and once we do, set dirty bit to 1, since the cache and lower memory is unaligned.

when memwriting down, we need to reconstruct the address by manipulating the tag, set, byteoffset bits. 


load = read, store = write.

*/