# FileNote

For Russian version, see [README_RU.md](README_RU.md)

## Description

**FileNote** is a small utility for Windows that allows you to create a text note for any file via the right-click context menu. When you right-click a file and select **FileNote**, it will generate a .txt file with the same name as the original file and open it in your default text editor.

### Features

- Add a note for any file via right-click.
- Automatically create .txt files with the same name as the target file.
- Register itself in the Windows context menu automatically.
- Supports uninstallation via **filenote.exe --uninstall**.

### Installation

1. Download or build **filenote.exe**.
2. Run the program once to register it in the context menu. A message will confirm the registration.
3. An uninstall helper **filenote_uninstall.cmd** will be created next to the program for easy removal.

### Usage

- Right-click any file → **FileNote**.
- A .txt note with the same name will be created next to the file and opened in your default text editor.

### Build

To compile from source:
```bat
REM Compile the resource (icon + version)
windres filenote.rc -o filenote_res.o

REM Compile the executable
g++ main.cpp filenote_res.o -o filenote.exe -std=c++11 -mwindows -lshlwapi
```
Or simply use the provided **build_all.cmd** to automate the build process.
