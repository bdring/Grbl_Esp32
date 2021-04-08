from os.path import isfile, join
import urllib.request
import os

Import("env")

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
os.chdir(FRAMEWORK_DIR)

patchpyurl = "https://raw.githubusercontent.com/techtonik/python-patch/master/patch.py"
patchurl = "https://github.com/espressif/arduino-esp32/commit/5ee6aac25.patch"

# Fetch a file from a URL
def wget(url, filename):
    handle = urllib.request.urlopen(url)
    file = open(filename,'w')
    file.write(handle.read().decode())
    file.close()

# Create an empty file
def touch(filename):
    file = open(filename, 'w');
    file.write('')
    file.close()

# patch file only if we didn't do it before
if not isfile('.patched'):
    if not isfile('patch.py'):
        wget(patchpyurl, 'patch.py')
    env.Execute("python patch.py %s" % (patchurl))
    touch('.patched')
    print('Applied serial patch to framework-arduinoespressif32')
