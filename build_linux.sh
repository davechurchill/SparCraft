#!/bin/bash
set -e

cmake -S . -B build -DSPARCRAFT_BUILD_APP=ON -DSPARCRAFT_BUILD_BWAPIDATA=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target SparCraft --parallel 8
