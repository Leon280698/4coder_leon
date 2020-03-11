@echo off
cd %~dp0
call ..\bin\buildsuper_x64-win.bat 4coder_leon.cpp
copy custom_4coder.dll ..\..\custom_4coder.dll
copy custom_4coder.pdb ..\..\custom_4coder.pdb
copy vc140.pdb ..\..\vc140.pdb
