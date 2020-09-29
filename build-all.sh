#!/bin/bash

# This shell script uses PlatformIO to compile Grbl_ESP32
# for every machine configuration in the Machines/ directory.
# It is useful for automated testing.

trap "echo; exit 255" SIGINT

# With -v, show all output.  Otherwise, show just the result
if [ "$1" = "-v" ]; then
   FILTER="cat"
   FILTER2="cat"
else
   FILTER="grep -v Compiling"
   FILTER2="grep error\|Took"
fi
set -o pipefail
NUM_ERRORS=0

BuildMachine () {
    basename=$1
    BF="\"-DMACHINE_FILENAME=$basename\""
    displayname=$basename
    echo "Building machine $displayname"
    PLATFORMIO_BUILD_FLAGS=$BF platformio run 2>&1 | $FILTER | $FILTER2
    local re=$?
    # check result
    if [ $re -ne 0 ]; then
        echo "Failed to build machine $displayname"
        echo
        NUM_ERRORS=$(( NUM_ERRORS + 1 ))
        return $re
	fi
    echo
}

# Build all the machines
for file in `ls ./Grbl_Esp32/src/Machines/*`; do
    base=`basename $file`
    BuildMachine $base ""
done

exit $NUM_ERRORS
