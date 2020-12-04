# This Windows PowerShell script uses PlatformIO to compile Grbl_ESP32
# for every machine configuration in the Machines/ directory.
# It is useful for automated testing.

# Setting PYTHONIOENCODING avoids an obscure crash related to code page mismatch
$env:PYTHONIOENCODING="utf-8"

Function BuildMachine($names) {
    $basename = $names[0]
    $env:PLATFORMIO_BUILD_FLAGS = "-DMACHINE_FILENAME=$basename"
    $displayname = $basename
    Write-Output "Building machine $displayname"
    platformio run 2>&1 | Select-String Compiling -NotMatch | Select-String error,Took
    Write-Output " "
}

# Build all the machines
foreach ($filepath in Get-ChildItem -file .\Grbl_Esp32\src\Machines\*) {
    BuildMachine($filepath.name, "")
}

Remove-Item env:PLATFORMIO_BUILD_FLAGS
