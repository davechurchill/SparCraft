@echo off
set ROOT=%~dp0
set ROOT=%ROOT:~0,-1%
cmake -G "Visual Studio 17 2022" -A x64 -S "%ROOT%" -B "%ROOT%\build" -DSFML_DIR="%SFML_DIR%\lib\cmake\SFML"
