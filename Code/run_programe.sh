#! /bin/bash
rm -r build
mkdir build && cd build
cmake ..
make -j4
cd build
cd test
./diana_debug
