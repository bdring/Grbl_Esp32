#!/bin/bash

# This shell script uses PlatformIO to compile Grbl_ESP32
# for every machine configuration in the Machines/ directory.
# It is useful for automated testing.

trap "echo; exit" SIGINT

# With -v, show all output.  Otherwise, show just the result
if [ "$1" = "-v" ]; then
   FILTER="cat"
else
   FILTER="grep error\|Took"
fi
BuildMachine () {
    basename=$1
    addname=$2
    BF="-DMACHINE_FILENAME=$basename"
    displayname=$basename
    if [ "$addname" != "" ]
    then
        BF="$BF -DMACHINE_FILENAME2=$addname"
        displayname="$basename + $addname"
    fi
    echo "Building machine $displayname"
    PLATFORMIO_BUILD_FLAGS=\'$BF\' platformio run 2>&1 | $FILTER
    echo
}

# First build all the base configurations with names that do not start with add_
for file in `ls ./Grbl_Esp32/Machines/* | grep -v add_\*`; do
    base=`basename $file`
    BuildMachine $base ""
done

# Then build all of the add-ons on top of a single base
base="3axis_v4.h"
for file in `ls ./Grbl_Esp32/Machines/add_*`
do
    adder=`basename $file`
    BuildMachine $base $adder
done
