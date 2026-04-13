@echo off
setlocal

pushd "%~dp0"

if not exist "SparCraft.exe" (
    echo Error: SparCraft.exe was not found in %~dp0
    popd
    exit /b 1
)

if not exist "sample_experiment\simple_gui.txt" (
    echo Error: sample_experiment\simple_gui.txt was not found in %~dp0
    popd
    exit /b 1
)

"%~dp0SparCraft.exe" "%~dp0sample_experiment\simple_gui.txt"
set "ERR=%ERRORLEVEL%"

popd
exit /b %ERR%
