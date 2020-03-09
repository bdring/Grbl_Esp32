#!/usr/bin/env python2

# Compile Grbl_ESP32 for the machine listed on the command line, as in
#  ./build-machine.py 3xis_v4.h

# Add-ons can be built by listing both the base name and the adder, as in
#   ./build-machine.py 3axis_v4.h add_esc_spindle.h

#  -q suppresses most messages
#  -u uploads the firmware to the target machine

from __future__ import print_function
import os, subprocess, sys

cmd=['platformio','run']
verbose = '-v' in sys.argv or '-q' not in sys.argv
if '-v' in sys.argv:
    sys.argv.remove('-v')
if '-q' in sys.argv:
    sys.argv.remove('-q')
if '-u' in sys.argv:
    sys.argv.remove('-u')
    cmd.append('--target=upload')

env = dict(os.environ)

def buildMachine(baseName, addName=None):
    displayName = baseName
    flags = '-DMACHINE_FILENAME=' + baseName
    if addName:
        displayName += ' + ' + addName
        flags += ' -DMACHINE_FILENAME2=' + addName
    print('Building machine ' + displayName)
    env['PLATFORMIO_BUILD_FLAGS'] = flags
    if verbose:
        subprocess.Popen(cmd, env=env).wait()
    else:
        app = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=1, env=env)
        for line in app.stdout:
            if "Took" in line or 'Uploading' in line or "error" in line.lower():
                print(line, end='')

if len(sys.argv) == 2:
    buildMachine(sys.argv[1], None)
elif len(sys.argv) == 3:
    buildMachine(sys.argv[1], sys.argv[2])
else:
    print("Usage: ./build-machine.py [-q] [-u] machine_name.h [add_name.h]")
    print(' Build for the given machine and optional add-on regardless of machine.h')
    print('  -q suppresses most messages')
    print('  -u uploads to the target after compilation')