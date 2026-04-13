@echo off
echo Set SparCraft Windows Environment Variables
echo Please edit this file before running for the first time

pause

setx BWAPI_DIR %~dp0..\bin\bwapidata
setx BOOST_DIR C:\libraries\boost_1_53_0
setx EXTERNAL_LIB_DIR c:\libraries\external_lib_dir
setx SFML_DIR C:\libraries\SFML-3.0.1

pause
