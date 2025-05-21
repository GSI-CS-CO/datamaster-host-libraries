#!/bin/bash

DEVICE=$1
FIRMWARE=$2
NUM_CPUS=$3
CLEAR_DIAGNOSTICS=$4

if [ -z "$DEVICE" ] || [ -z "$FIRMWARE" ]; then
    echo "Usage: $0 <device> <firmware> [<num_cpus>] [-d]"
    echo "Example: $0 /dev/ttyUSB0 firmware.bin 4"
    exit 1
fi

if [ ! -f "$FIRMWARE" ]; then
    echo "Firmware file $FIRMWARE not found!"
    exit 1
fi

# firmware must have extension .bin
if [[ "$FIRMWARE" != *.bin ]]; then
    echo "Firmware file must have .bin extension!"
    exit 1
fi

# when device starts with / remove it
if [[ "$DEVICE" == /* ]]; then
    DEVICE=${DEVICE:1}
fi

# default for NUM_CPUS is obtained with eb-ls command
if [ -z "$NUM_CPUS" ]; then
    if $(command -v eb-ls >/dev/null 2>&1); then
        NUM_CPUS=$(eb-ls $DEVICE | grep -oP 'LM32-RAM-User' -c)
    else
        echo "eb-ls command not found, using default NUM_CPUS=4"
        NUM_CPUS=4
    fi
fi

echo -n "Halting device ($DEVICE) cpus..."
eb-reset $DEVICE cpuhalt 0xff
echo "done."

cpu_index=0
while [ $cpu_index -lt $NUM_CPUS ]; do
    echo -n "Loading firmware ($FIRMWARE) to CPU $cpu_index..."
    eb-fwload $DEVICE u$cpu_index 0 $FIRMWARE
    if [ $? -ne 0 ]; then
        echo "error. Aborting."
        exit 1
    fi
    echo "done."
    cpu_index=$((cpu_index + 1))
done

echo -n "Resetting device ($DEVICE) cpus..."
eb-reset $DEVICE cpureset 0xff
if [ $? -ne 0 ]; then
    echo "error. Aborting."
    exit 1
fi
echo "done."
echo "Firmware loaded successfully to all CPUs."

eb-info -w $DEVICE