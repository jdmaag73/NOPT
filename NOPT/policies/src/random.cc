////////////////////////////////////////////
//                                        //
// RANDOM
//                                        //
////////////////////////////////////////////

#include "../inc/champsim_crc2.h"

#define LLC_WAYS 16

uint64_t misses = 0, hits = 0;

// initialize replacement state
void InitReplacementState()
{
    cout << "Initialize RANDOM, no state" << endl;
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    // get a random victim 
    uint32_t way = (misses ^ hits) % LLC_WAYS;
	return way;
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    if (hit)
        hits++;
    else
        misses++;
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{

}
