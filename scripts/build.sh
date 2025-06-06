#!/bin/bash

# shellcheck source=./.container-helpers
. "$(dirname "$0")/.container-helpers"

#if ! is_inside_container; then
    echo "BUILDING DEV TOOLS"
    exec 3>&1 4>&2
    exec > >(sed 's/^/[DEV TOOLS] /') 2>&1
    cmake -B ./build/build_dev/ -DBUILD_HOST=ON
    cmake --build build/build_dev -- -j
    exec 1>&3 2>&4
#fi

reopen_script_in_container

# shellcheck source=/dev/null
. /opt/sdk/environment-setup-core2-64-ffos-linux

exec 3>&1 4>&2
exec > >(sed 's/^/[TARGET HOST] /') 2>&1


echo "BUILDING HOST TOOLS"
cmake -B ./build/build_host/ \
    --toolchain "${OE_CMAKE_TOOLCHAIN_FILE}"
cmake --build build/build_host -- -j


exec 1>&3 2>&4

CBR_GIT1=$(git log HEAD~0 --oneline --decorate=no -n 1 2>/dev/null | cut -c1-100)
CBR_GIT2=$(git log HEAD~1 --oneline --decorate=no -n 1 2>/dev/null | cut -c1-100)
CBR_GIT3=$(git log HEAD~2 --oneline --decorate=no -n 1 2>/dev/null | cut -c1-100)
CBR_GIT4=$(git log HEAD~3 --oneline --decorate=no -n 1 2>/dev/null | cut -c1-100)
CBR_GIT5=$(git log HEAD~4 --oneline --decorate=no -n 1 2>/dev/null | cut -c1-100)

exec 3>&1 4>&2
exec > >(sed 's/^/[TARGET FIRMWARE] /') 2>&1

cmake -B ./build/build_firmware/ \
    --toolchain ./toolchains/lm32.cmake \
    -DBUILD_FIRMWARE=ON \
    -DCBR_USR="$(git log -1 --pretty=format:'%an' | iconv -f utf-8 -t ascii//translit)" \
    -DCBR_MAIL="$(git log -1 --pretty=format:'%ae')" \
    -DCBR_HOST="$BUILD_HOST" \
    -DCBR_OS="$(uname -o)" \
    -DCBR_KRNL="$(uname -r)" \
    -DUSER="$USER" \
    -DGIT_BRANCH="$(git rev-parse --abbrev-ref HEAD)" \
    -DCBR_GIT1="$CBR_GIT1" \
    -DCBR_GIT2="$CBR_GIT2" \
    -DCBR_GIT3="$CBR_GIT3" \
    -DCBR_GIT4="$CBR_GIT4" \
    -DCBR_GIT5="$CBR_GIT5"

cmake --build build/build_firmware -j

exec 1>&3 2>&4

