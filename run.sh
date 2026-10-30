#!/usr/bin/env bash

set -e

mkdir -p build
cd build

echo ">>> Configuring CMake..."
cmake ..

echo ">>> Compiling project..."
cmake --build .

echo ">>> Launching..."
./chess
