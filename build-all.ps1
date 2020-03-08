# This Windows PowerShell script uses PlatformIO to compile Grbl_ESP32
# for every machine configuration in the Machines/ directory.
# It is useful for automated testing.

# Setting PYTHONIOENCODING avoids an obscure crash related to code page mismatch
$env:PYTHONIOENCODING="utf-8"

Function BuildMachine($names) {
    $basename = $names[0]
    $addname = $names[1]
    $env:PLATFORMIO_BUILD_FLAGS = "-DMACHINE_FILENAME=$basename"
    $displayname = $basename
    if ($addname -ne "") {
        $env:PLATFORMIO_BUILD_FLAGS += " -DMACHINE_FILENAME2=$addname"
        $displayname += " + $addname"
    }
    Write-Output "Building machine $displayname"
    platformio run 2>&1 | Select-String error,Took
    Write-Output " "
}

# First build all the base configurations with names that do not start with add_
foreach ($filepath in Get-ChildItem -file .\Grbl_Esp32\Machines\* -Exclude add_*) {
    BuildMachine($filepath.name, "")
}

# Then build all of the add-ons on top of a single base
$base="3axis_v4.h"
foreach ($filepath in Get-ChildItem -file .\Grbl_Esp32\Machines\add_*) {
    BuildMachine($base, $filepath.name)
}
Remove-Item env:PLATFORMIO_BUILD_FLAGS
