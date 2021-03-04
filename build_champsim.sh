#!/bin/sh
# ChampSim configuration
BRANCH=$1           # branch/*.bpred
L1D_PREFETCHER=$2   # prefetcher/*.l1d_pref
L2C_PREFETCHER=$3   # prefetcher/*.l2c_pref
LLC_REPLACEMENT=$4  # replacement/*.llc_repl
NUM_CORE=$5         # tested up to 8-core system
CRC2_CONF=$6

############## Some useful macros ###############
BOLD=$(tput bold)
NORMAL=$(tput sgr0)

embed_newline()
{
   local p="$1"
   shift
   for i in "$@"
   do
      p="$p\n$i"         # Append
   done
   echo -e "$p"          # Use -e
}
#################################################

# Sanity check
if [ ! -f ./branch/${BRANCH}.bpred ] || [ ! -f ./prefetcher/${L1D_PREFETCHER}.l1d_pref ] || [ ! -f ./prefetcher/${L2C_PREFETCHER}.l2c_pref ] || [ ! -f ./replacement/${LLC_REPLACEMENT}.llc_repl ]; then
	echo "${BOLD}Possible Branch Predictor: ${NORMAL}"
	LIST=$(ls branch/*.bpred | cut -d '/' -f2 | cut -d '.' -f1)
	p=$( embed_newline $LIST )
	echo "$p"

	echo "${BOLD}Possible L1D Prefetcher: ${NORMAL}"
	LIST=$(ls prefetcher/*.l1d_pref | cut -d '/' -f2 | cut -d '.' -f1)
	p=$( embed_newline $LIST )
	echo "$p"

	echo
	echo "${BOLD}Possible L2C Prefetcher: ${NORMAL}"
	LIST=$(ls prefetcher/*.l2c_pref | cut -d '/' -f2 | cut -d '.' -f1)
	p=$( embed_newline $LIST )
	echo "$p"

	echo
	echo "${BOLD}Possible LLC Replacement: ${NORMAL}"
	LIST=$(ls replacement/*.llc_repl | cut -d '/' -f2 | cut -d '.' -f1)
	p=$( embed_newline $LIST )
	echo "$p"
	exit
fi

# Check for multi-core
if [ "$NUM_CORE" != "1" ]
then
    if [ "$NUM_CORE" != "8" ]
    then
        echo "${BOLD}Building multi-core ChampSim...${NORMAL}"
        sed -i.bak 's/\<NUM_CPUS 1\>/NUM_CPUS '${NUM_CORE}'/g' inc/champsim.h
	sed -i.bak 's/\<DRAM_CHANNELS 1\>/DRAM_CHANNELS 2/g' inc/champsim.h
	sed -i.bak 's/\<LOG2_DRAM_CHANNELS 0\>/LOG2_DRAM_CHANNELS 1/g' inc/champsim.h
    else
        echo "${BOLD}Building 8-core ChampSim...${NORMAL}"
        sed -i.bak 's/\<NUM_CPUS 1\>/NUM_CPUS '${NUM_CORE}'/g' inc/champsim.h
        sed -i.bak 's/\<DRAM_CHANNELS 1\>/DRAM_CHANNELS 4/g' inc/champsim.h
        sed -i.bak 's/\<LOG2_DRAM_CHANNELS 0\>/LOG2_DRAM_CHANNELS 2/g' inc/champsim.h
    fi
else
    echo "${BOLD}Building single-core ChampSim...${NORMAL}"
fi
echo

# Check for crc2 config
if [ ! -z "$CRC2_CONF" ]
then
    echo "${BOLD}CRC2 config ${CRC2_CONF}...${NORMAL}"
    sed -i.bak 's/\<CRC2_CONFIG 1\>/CRC2_CONFIG '${CRC2_CONF}'/g' inc/champsim.h
fi
echo

# Change prefetchers and replacement policy
cp branch/${BRANCH}.bpred branch/branch_predictor.cc
cp prefetcher/${L1D_PREFETCHER}.l1d_pref prefetcher/l1d_prefetcher.cc
cp prefetcher/${L2C_PREFETCHER}.l2c_pref prefetcher/l2c_prefetcher.cc
cp replacement/${LLC_REPLACEMENT}.llc_repl replacement/llc_replacement.cc

# Build
mkdir -p bin
rm -f bin/champsim
make clean

if [ -z "$CRC2_CONF" ]
then
	make
else
	make lib
fi

# Sanity check
echo ""
if [ -z "$CRC2_CONF" ]
then
	if [ ! -f bin/champsim ]; then
		echo "${BOLD}ChampSim build FAILED!${NORMAL}"
		echo ""
	else
		echo "${BOLD}ChampSim is successfully built"
		echo "Branch Predictor: ${BRANCH}"
		echo "L1D Prefetcher: ${L1D_PREFETCHER}"
		echo "L2C Prefetcher: ${L2C_PREFETCHER}"
		echo "LLC Replacement: ${LLC_REPLACEMENT}"
		echo "Cores: ${NUM_CORE}"
		BINARY_NAME="${BRANCH}-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_REPLACEMENT}-${NUM_CORE}core"
		echo "Binary: bin/${BINARY_NAME}${NORMAL}"
		echo ""
		mv bin/champsim bin/${BINARY_NAME}
	fi
else
	if [ ! -f bin/champsim.a ]; then
		echo "${BOLD}ChampSim build FAILED!${NORMAL}"
		echo ""
	else
		echo "${BOLD}ChampSim is successfully built"
		echo ""
	fi
fi

# Restore to the default configuration
sed -i.bak 's/\<NUM_CPUS '${NUM_CORE}'\>/NUM_CPUS 1/g' inc/champsim.h
sed -i.bak 's/\<DRAM_CHANNELS 2\>/DRAM_CHANNELS 1/g' inc/champsim.h
sed -i.bak 's/\<DRAM_CHANNELS 4\>/DRAM_CHANNELS 1/g' inc/champsim.h
sed -i.bak 's/\<LOG2_DRAM_CHANNELS 1\>/LOG2_DRAM_CHANNELS 0/g' inc/champsim.h
sed -i.bak 's/\<LOG2_DRAM_CHANNELS 2\>/LOG2_DRAM_CHANNELS 0/g' inc/champsim.h

if [ ! -z "$CRC2_CONF" ]
then
    sed -i.bak 's/\<CRC2_CONFIG '${CRC2_CONF}'\>/CRC2_CONFIG 1/g' inc/champsim.h
fi

cp branch/bimodal.bpred branch/branch_predictor.cc
cp prefetcher/no.l1d_pref prefetcher/l1d_prefetcher.cc
cp prefetcher/no.l2c_pref prefetcher/l2c_prefetcher.cc
cp replacement/lru.llc_repl replacement/llc_replacement.cc
