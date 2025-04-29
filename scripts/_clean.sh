#!/bin/bash
. /opt/sdk/environment-setup-core2-64-ffos-linux
cmake -B ./build/ \
    --toolchain ${OE_CMAKE_TOOLCHAIN_FILE} \
    #-DBOOST_ROOT="/usr/lib/boost-1_74_0/" \
    #-DBoost_NO_SYSTEM_PATHS=ON
cmake --build build --target clean
