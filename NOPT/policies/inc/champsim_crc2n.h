////////////////////////////////////////////
//                                        //
//        DO NOT CHANGE THIS FILE         //
//                                        //
//      ChampSim interface for CRC-2      //
//     Jinchun Kim, cienlux@tamu.edu      //
//                                        //
////////////////////////////////////////////

#ifndef CHAMPSIM_CRC2_H
#define CHAMPSIM_CRC2_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <string>

// CACHE ACCESS TYPE
#define LOAD      0
#define RFO       1
#define PREFETCH  2
#define WRITEBACK 3
#define NUM_TYPES 4

using namespace std;

class BLOCK {
  public:
    uint8_t valid,
            prefetch,
            dirty,
            used;

    int delta,
        depth,
        signature,
        confidence;

    uint64_t address,
             full_addr,
             tag,
             data,
             cpu,
             instr_id;

    // replacement state
    uint32_t lru;

    BLOCK() {
        valid = 0;
        prefetch = 0;
        dirty = 0;
        used = 0;

        delta = 0;
        depth = 0;
        signature = 0;
        confidence = 0;

        address = 0;
        full_addr = 0;
        tag = 0;
        data = 0;
        cpu = 0;
        instr_id = 0;

        lru = 0;
    };
};

void InitReplacementState(),
     UpdateReplacementState(uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit),
     PrintStats_Heartbeat(),
     PrintStats();

uint32_t GetVictimInSet(uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type);
uint64_t get_cycle_count(), 
         get_instr_count(uint32_t cpu),
         get_config_number();

bool is_block_in_L2(uint32_t cpu, uint64_t full_addr);

#endif
