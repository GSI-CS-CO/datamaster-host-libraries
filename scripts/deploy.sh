#!/bin/bash

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <host>"
    exit 1
fi

HOST="$1"
TARGET_BIN="root@${HOST}:/usr/bin"
TARGET_LIB="root@${HOST}:/usr/lib"
TARGET_FIRMWARE="root@${HOST}:/firmware"

if [ ! -d "./out" ]; then
    echo "Error: ./out directory does not exist."
    exit 1
fi
ssh-copy-id -o StrictHostKeyChecking=no "root@${HOST}"
echo "Deploying ./out to $TARGET ..."
scp -O ./out/{dm-sched,dm-cmd} "$TARGET_BIN"
scp -O ./out/libcarpedm.* "$TARGET_LIB"

ssh -o StrictHostKeyChecking=no "root@${HOST}" "mkdir -p /firmware"

scp -O ./out/ftm.bin "$TARGET_FIRMWARE"

for i in $(seq 0 3); do
    echo "Flashing firmware to CPU $i ..."
    ssh -o StrictHostKeyChecking=no "root@${HOST}" "eb-fwload dev/wbm0 u$i 0 /firmware/ftm.bin"
done

echo "Resetting cpu..."
ssh -o StrictHostKeyChecking=no "root@${HOST}" "eb-reset dev/wbm0 cpureset 0xff"

ssh -o StrictHostKeyChecking=no "root@${HOST}" "eb-info dev/wbm0"
ssh -o StrictHostKeyChecking=no "root@${HOST}" "eb-info -w dev/wbm0"

echo "Deployment complete."