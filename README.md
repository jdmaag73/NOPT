<p align="center">
  <h1 align="center"> NOPT </h1>
  <p> This repository holds all source code related to the research paper "Near-optimal replacement policies for shared caches in multicore processors", authored by Javier Díaz, Pablo Ibáñez, Teresa Monreal, Víctor Viñals and José M. Llabería, published by Springer Nature in The Journal of Supercomputing, DOI 10.1007/s11227-021-03736-1 <p>
</p>

# Repository content

This repository is basically a fork of https://github.com/ChampSim/ChampSim. Some changes have been made to the ChampSim simulator to add features required to conduct the research.

The NOPT folder is new, and includes the following folder structure:

- policies: Stores several Linux x64 binaries that can be directly run to reproduce the mentioned work. It also includes a script (make.sh) to build those binaries starting from the ChampSim libreries and the source code.
- policies/pseudocode: Pseudo-code for NOPTb-miss.
- policies/src: Source code for all policies simulated in the work.
- policies/inc: Header files.
- policies/lib: Intermediate ChampSim libraries
- workloads: Composition of the 4-core and 8-core workload set used to perform the evaluations in the paper. The set of traces is the same used for the 2nd Cache Replacement Championship (CRC-2), that can be found from this link. (http://bit.ly/2t2nkUj) 

# Reproducing the work

The Linux x64 binaries in /NOPT/policies can be directly run to reproduce the results. Files with "8c" in the name simulate an 8-core processor with 16 MB of SLLC, used in Section 9 of the paper. The rest simulate a 4-core processor with 8 MB of SLLC.
To obtain the required components, clone this repo and download the traces from the above link.

Here are some sample executions for several scenarios. 

4-core/8 MB NOPTb-miss, Iteration 0, using MISSES, mix 0:
```
export FICH_OPT_ACCESSES=void
export FICH_OPT_ACCESSES_SIG=noptb-miss_fall_c3n.sim0.mix-0.OPT_accesses
export TRACE_INSTRUCTIONS=1000000000
export WARMUP_INSTRUCTIONS=200000000
export SIMULATION_INSTRUCTIONS=800000000
export EXTRA_INSTRUCTIONS=1200000000
export SIMULANDO=0
export MIX=0
export TRAZAS="-traces sjeng_358B.trace.gz calculix_2670B.trace.gz astar_163B.trace.gz sphinx3_2520B.trace.gz"
NOPT/policies/noptb-miss_fall_c3n.exe -warmup_instructions ${WARMUP_INSTRUCTIONS} -simulation_instructions ${SIMULATION_INSTRUCTIONS} -extra_instructions ${EXTRA_INSTRUCTIONS} ${TRAZAS}
```

4-core/8 MB NOPTb-miss, Iteration 1 continuing from previous one, mix 0:
```
export FICH_OPT_ACCESSES=noptb-miss_fall_c3n.sim0.mix-0.OPT_accesses
export FICH_OPT_ACCESSES_SIG=noptb-miss_fall_c3n.sim1.mix-0.OPT_accesses
export TRACE_INSTRUCTIONS=1000000000
export WARMUP_INSTRUCTIONS=200000000
export SIMULATION_INSTRUCTIONS=800000000
export EXTRA_INSTRUCTIONS=1200000000
export SIMULANDO=1
export MIX=0
export TRAZAS="-traces sjeng_358B.trace.gz calculix_2670B.trace.gz astar_163B.trace.gz sphinx3_2520B.trace.gz"
NOPT/policies/noptb-miss_fall_c3n.exe -warmup_instructions ${WARMUP_INSTRUCTIONS} -simulation_instructions ${SIMULATION_INSTRUCTIONS} -extra_instructions ${EXTRA_INSTRUCTIONS} ${TRAZAS}
```

8-core/16 MB NOPTb-miss, Iteration 0, using SRRIP, mix 1:
```
export FICH_OPT_ACCESSES=void
export FICH_OPT_ACCESSES_SIG=noptb-miss_8c3n.sim0.mix-0.OPT_accesses
export TRACE_INSTRUCTIONS=1000000000
export WARMUP_INSTRUCTIONS=200000000
export SIMULATION_INSTRUCTIONS=800000000
export EXTRA_INSTRUCTIONS=1200000000
export SIMULANDO=0
export MIX=1
export TRAZAS="-traces sjeng_358B.trace.gz omnetpp_340B.trace.gz astar_163B.trace.gz perlbench_53B.trace.gz namd_400B.trace.gz calculix_2670B.trace.gz astar_163B.trace.gz lbm_94B.trace.gz"
NOPT/policies/noptb-miss_8c3n.exe -warmup_instructions ${WARMUP_INSTRUCTIONS} -simulation_instructions ${SIMULATION_INSTRUCTIONS} -extra_instructions ${EXTRA_INSTRUCTIONS} ${TRAZAS}
```

4-core/8 MB NOPTb-fair, mix 0, phase 1 followed by phase 2:
```
export FICH_OPT_ACCESSES=noptb-fair_c3n.sim0.mix-0.OPT_accesses
export TRACE_INSTRUCTIONS=1000000000
export WARMUP_INSTRUCTIONS=200000000
export SIMULATION_INSTRUCTIONS=800000000
export SIMULANDO=0
export MIX=0
export TRAZAS="-traces sjeng_358B.trace.gz calculix_2670B.trace.gz astar_163B.trace.gz sphinx3_2520B.trace.gz"
NOPT/policies/noptb-fair_c3n.exe -warmup_instructions ${WARMUP_INSTRUCTIONS} -simulation_instructions ${SIMULATION_INSTRUCTIONS} ${TRAZAS}
export SIMULANDO=1
NOPT/policies/noptb-fair_c3n.exe -warmup_instructions ${WARMUP_INSTRUCTIONS} -simulation_instructions ${SIMULATION_INSTRUCTIONS} ${TRAZAS}
```

4-core/8 MB Random policy, mix 0:
```
export WARMUP_INSTRUCTIONS=200000000
export SIMULATION_INSTRUCTIONS=800000000
export TRAZAS="-traces sjeng_358B.trace.gz calculix_2670B.trace.gz astar_163B.trace.gz sphinx3_2520B.trace.gz"
NOPT/policies/random_c3n.exe -warmup_instructions ${WARMUP_INSTRUCTIONS} -simulation_instructions ${SIMULATION_INSTRUCTIONS} ${TRAZAS}
```
