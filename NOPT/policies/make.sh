# Libraries in lib subdir are built using ChampSim
# NOPTb require Boost (https://www.boost.org/)

# NOPTb-miss
g++ -O2 -Wall -I${BOOST_DIR}/include --std=c++0x -o noptb-miss_c3n.exe -DCRC2N -DBYPASS=1 -DNUM_CORE=4 src/noptb-miss.cc lib/config3n.a ${BOOST_DIR}/lib/libboost_serialization.a
g++ -O2 -Wall -I${BOOST_DIR}/include --std=c++0x -o noptb-miss_fall_c3n.exe -DCRC2N -DBYPASS=1 -DNUM_CORE=4 -DSIM0_TODO_FALLOS src/noptb-miss.cc lib/config3n.a ${BOOST_DIR}/lib/libboost_serialization.a
g++ -O2 -Wall -I${BOOST_DIR}/include --std=c++0x -o noptb-miss_rnd_c3n.exe -DCRC2N -DBYPASS=1 -DNUM_CORE=4 -DSIM0_RANDOM src/noptb-miss.cc lib/config3n.a ${BOOST_DIR}/lib/libboost_serialization.a
g++ -O2 -Wall -I${BOOST_DIR}/include --std=c++0x -o noptb-miss_8c3n.exe -DCRC2N -DBYPASS=1 -DNUM_CORE=8 src/noptb-miss.cc lib/config8c3n.a ${BOOST_DIR}/lib/libboost_serialization.a
g++ -O2 -Wall -I${BOOST_DIR}/include --std=c++0x -o noptb-miss_fall_8c3n.exe -DCRC2N -DBYPASS=1 -DNUM_CORE=8 -DSIM0_TODO_FALLOS src/noptb-miss.cc lib/config8c3n.a ${BOOST_DIR}/lib/libboost_serialization.a
g++ -O2 -Wall -I${BOOST_DIR}/include --std=c++0x -o noptb-miss_rnd_8c3n.exe -DCRC2N -DBYPASS=1 -DNUM_CORE=8 -DSIM0_RANDOM src/noptb-miss.cc lib/config8c3n.a ${BOOST_DIR}/lib/libboost_serialization.a

# NOPTb-fair
g++ -O2 -Wall -I${BOOST_DIR}/include --std=c++0x -o noptb-fair_c3n.exe -DCRC2N -DBYPASS=1 -DNUM_CORE=4 src/noptb-fair.cc lib/config3n.a ${BOOST_DIR}/lib/libboost_serialization.a
g++ -O2 -Wall -I${BOOST_DIR}/include --std=c++0x -o noptb-fair_8c3n.exe -DCRC2N -DBYPASS=1 -DNUM_CORE=8 src/noptb-fair.cc lib/config8c3n.a ${BOOST_DIR}/lib/libboost_serialization.a

# Other
g++ -O2 -Wall --std=c++0x -o lru_c3n.exe src/lru-8MB.cc lib/config3n.a
g++ -O2 -Wall --std=c++0x -o srrip_v02_c3n.exe src/srrip_v02-8MB.cc lib/config3n.a
g++ -O2 -Wall --std=c++0x -o random_c3n.exe src/random.cc lib/config3n.a
g++ -O2 -Wall --std=c++0x -o ship++_c3n.exe src/ship++.cc lib/config3n.a
g++ -O2 -Wall --std=c++0x -o hawkeye_c3n.exe src/hawkeye_final_8MB.cc lib/config3n.a
g++ -O2 -Wall --std=c++0x -o red_crc2_c3n.exe example/red.cc lib/config3n.a

g++ -O2 -Wall --std=c++0x -o random_8c3n.exe src/random.cc lib/config8c3n.a
g++ -O2 -Wall --std=c++0x -o lru_8c3n.exe -DNUM_CORE=8 src/lru-8MB.cc lib/config8c3n.a
g++ -O2 -Wall --std=c++0x -o srrip_v02_8c3n.exe -DNUM_CORE=8 src/srrip_v02-8MB.cc lib/config8c3n.a
g++ -O2 -Wall --std=c++0x -o ship++_8c3n.exe -DNUM_CORE=8 src/ship++.cc lib/config8c3n.a
g++ -O2 -Wall --std=c++0x -o hawkeye_8c3n.exe -DNUM_CORE=8 src/hawkeye_final_8MB.cc lib/config8c3n.a
g++ -O2 -Wall --std=c++0x -o red_crc2_8c3n.exe -DNUM_CORE=8 src/red.cc lib/config8c3n.a

