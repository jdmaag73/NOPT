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

The Linux x64 binaries in /NOPT/policies can be directly run to reproduce the results. Files with "8c" in the name simulate an 8-core processor, used in Section 9 of the paper. The rest simulate a 4-core processor.

Here are some sample executions for several scenarios. Downloading and uncompressing the traces is a prerequisite. 

