@echo off
REM ----------------------------
REM Build all: resource + exe
REM ----------------------------

REM --- Step 1: Compile resource ---
echo Compiling resource filenote.rc to filenote_res.o...
windres filenote.rc -o filenote_res.o
if %ERRORLEVEL% NEQ 0 (
    echo Resource compilation error!
    pause
    exit /b 1
)
echo Resource compiled successfully.

REM --- Step 2: Compile exe ---
echo Compiling filenote.exe...
g++ main.cpp filenote_res.o -o filenote.exe -std=c++11 -mwindows -lshlwapi
if %ERRORLEVEL% NEQ 0 (
    echo Compilation error!
    pause
    exit /b 1
)
echo Compilation completed successfully.

pause
