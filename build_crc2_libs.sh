#!/bin/sh
./build_champsim.sh gshare no no crc2 1 1
mv bin/champsim.a lib/config1n.a

./build_champsim.sh gshare next_line ip_stride crc2 1 2
mv bin/champsim.a lib/config2n.a

./build_champsim.sh gshare no no crc2 4 3
mv bin/champsim.a lib/config3n.a

./build_champsim.sh gshare next_line ip_stride crc2 4 4
mv bin/champsim.a lib/config4n.a

./build_champsim.sh gshare no no crc2 1 5
mv bin/champsim.a lib/config5n.a

./build_champsim.sh gshare next_line ip_stride crc2 1 6
mv bin/champsim.a lib/config6n.a

./build_champsim.sh gshare no no crc2 8 3
mv bin/champsim.a lib/config8c3n.a

./build_champsim.sh gshare next_line ip_stride crc2 8 4
mv bin/champsim.a lib/config8c4n.a

