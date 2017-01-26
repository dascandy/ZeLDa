#!/bin/bash

set -e
mkdir -p build
cd build
cmake -G Ninja ..
ninja
cd ..

