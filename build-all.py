#!/usr/bin/env python2

# Compile Grbl_ESP32 for each of the machines defined in Machines/ .
# Add-on files are built on top of a single base.
# This is useful for automated testing, to make sure you haven't broken something

# The output is filtered so that the only lines you see are a single
# success or failure line for each build, plus any preceding lines that
# contain the word "error".  If you need to see everything, for example to
# see the details of an errored build, include -v on the command line.

from __future__ import print_function
import os, subprocess, sys

env = dict(os.environ)
verbose = '-v' in sys.argv

def buildMachine(baseName, addName=None):
    displayName = baseName
    flags = '-DMACHINE_FILENAME=' + baseName
    if addName:
        displayName += ' + ' + addName
        flags += ' -DMACHINE_FILENAME2=' + addName
    print('Building machine ' + displayName)
    env['PLATFORMIO_BUILD_FLAGS'] = flags
    app = subprocess.Popen(['platformio','run'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=1, env=env)
    for line in app.stdout:
        if verbose or "Took" in line or "error" in line.lower():
            print(line, end='')
    print()

adderBase = '3axis_v4.h'
for name in os.listdir('Grbl_Esp32/Machines'):
    if name.startswith('add_'):
        buildMachine(adderBase, name)
    else:
        buildMachine(name)


