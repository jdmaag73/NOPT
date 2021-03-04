////////////////////////////////////////////
//                                        //
//     SRRIP [Jaleel et al. ISCA' 10]     //
//     Jinchun Kim, cienlux@tamu.edu      //
//                                        //
////////////////////////////////////////////

#include "../inc/champsim_crc2.h"

#ifndef NUM_CORE
#define NUM_CORE 4
#endif
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16

#ifndef maxRRPV
#define maxRRPV 3
#endif

uint32_t rrpv[LLC_SETS][LLC_WAYS];

// initialize replacement state
void InitReplacementState()
{
    cout << "Initialize SRRIP state, maxRRPV " << maxRRPV << endl;

    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            rrpv[i][j] = maxRRPV;
        }
    }
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    // look for the maxRRPV line
    while (1)
    {
        for (int i=0; i<LLC_WAYS; i++)
            if (rrpv[set][i] == maxRRPV)
                return i;

        for (int i=0; i<LLC_WAYS; i++)
            rrpv[set][i]++;
    }

    // WE SHOULD NOT REACH HERE
    assert(0);
    return 0;
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
	// Write-backs do not change rrpv
	if (type == WRITEBACK) return; 

    if (hit)
        rrpv[set][way] = 0;
    else
        rrpv[set][way] = maxRRPV-1;
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{

}
