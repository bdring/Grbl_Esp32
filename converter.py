# This script is imported by build-machine.py and build-all.py
# It performs a platformio build with a given machine file.
# The verbose argument controls whether the full output is
# displayed, or filtered to show only summary information.
# extraArgs can be used to perform uploading after compilation.

from __future__ import print_function
import subprocess, os
from pathlib import Path

env = dict(os.environ)

def convertMachine(baseName, verbose=True, extraArgs=None):
    cmd = ['platformio','run','-enative']
    if extraArgs:
        cmd.append(extraArgs)
    displayName = baseName
    flags = '-DEMIT_YAML -DMACHINE_FILENAME=' + baseName
    print('Converting machine ' + displayName)
    env['PLATFORMIO_BUILD_FLAGS'] = flags
    if verbose:
        app = subprocess.Popen(cmd, env=env)
    else:
        app = subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=1)
        for line in app.stdout:
            line = line.decode('utf8')
            if "Took" in line or 'Uploading' in line or ("error" in line.lower() and "Compiling" not in line):
                print(line, end='')
    app.wait()
    if app.returncode == 0:
        print()
        cmd = [ '.pio/build/native/program.exe' ]
        out_filename = "yaml/" + Path(baseName).stem + ".yaml"
        app = subprocess.Popen(cmd, env=env, stdout=open(out_filename, "w"), stderr=subprocess.STDOUT, bufsize=1)
        app.wait()
        # return app.returncode
    else:
        out_filename = "yaml/" + Path(baseName).stem + ".ERROR"
        open(out_filename, "w")
    return 0

