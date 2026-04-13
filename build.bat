@echo off
setlocal

cmake -S . -B build -DSPARCRAFT_BUILD_APP=ON -DSPARCRAFT_BUILD_BWAPIDATA=ON
if errorlevel 1 exit /b %errorlevel%

cmake --build build --config Release --target SparCraft --parallel 8
if errorlevel 1 exit /b %errorlevel%

endlocal
