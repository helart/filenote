#include <windows.h>
#include <shlwapi.h>
#include <string>
#include <fstream>

#pragma comment(lib, "Shlwapi.lib")

bool IsRegistryEntryExists() {
    HKEY hKey;
    const char* keyPath = "Software\\Classes\\*\\shell\\FileNote";
    LONG res = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_READ, &hKey);
    if (res == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

void CreateRegistryEntry() {
    HKEY hKey;
    const char* keyPath = "Software\\Classes\\*\\shell\\FileNote";
    const char* cmdPath = "Software\\Classes\\*\\shell\\FileNote\\command";

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string exePathStr = exePath;
    std::string commandStr = "\"" + exePathStr + "\" \"%1\"";

    // Key FileNote
    if (RegCreateKeyExA(HKEY_CURRENT_USER, keyPath, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "MUIVerb", 0, REG_SZ,
                       (const BYTE*)"FileNote", (DWORD)strlen("FileNote") + 1);
        RegSetValueExA(hKey, "Icon", 0, REG_SZ,
                       (const BYTE*)exePathStr.c_str(), (DWORD)exePathStr.length() + 1);
        RegSetValueExA(hKey, "Position", 0, REG_SZ,
                       (const BYTE*)"Top", (DWORD)strlen("Top") + 1);
        RegCloseKey(hKey);
    }

    // Key command
    if (RegCreateKeyExA(HKEY_CURRENT_USER, cmdPath, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, NULL, 0, REG_SZ,
                       (const BYTE*)commandStr.c_str(),
                       (DWORD)commandStr.length() + 1);
        RegCloseKey(hKey);
    }

    // Create filenote_uninstall.cmd file next to exe
    std::string uninstallCmdPath = exePathStr;
    size_t lastSlash = uninstallCmdPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        uninstallCmdPath = uninstallCmdPath.substr(0, lastSlash + 1) + "filenote_uninstall.cmd";
    }

    std::ofstream ofs(uninstallCmdPath);
    ofs << "\"" << exePathStr << "\" --uninstall" << std::endl;
    ofs.close();
}

void RemoveRegistryEntry() {
    // Delete registry keys
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\*\\shell\\FileNote\\command");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\*\\shell\\FileNote");

    // Determine the path to the executable file
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // Create path to filenote_uninstall.cmd
    std::string uninstallCmdPath = exePath;
    size_t lastSlash = uninstallCmdPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        uninstallCmdPath = uninstallCmdPath.substr(0, lastSlash + 1) + "filenote_uninstall.cmd";
    }

    // Check if the file exists and delete it
    if (GetFileAttributesA(uninstallCmdPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        DeleteFileA(uninstallCmdPath.c_str());
    }

    MessageBoxA(NULL, "The FileNote item has been removed from the context menu.", "FileNote", MB_OK | MB_ICONINFORMATION);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    // If run with --uninstall argument
    if (__argc >= 2 && strcmp(__argv[1], "--uninstall") == 0) {
        RemoveRegistryEntry();
        return 0;
    }

    bool registryExists = IsRegistryEntryExists();

    // If the registry entry is missing, create it and .cmd to delete it
    if (!registryExists) {
        CreateRegistryEntry();
        MessageBoxA(NULL, "The path to the program is registered for the context menu.", "FileNote", MB_OK | MB_ICONINFORMATION);
        return 0; // exit without creating a note
    }

    // If the program is launched without an argument and the record already exists
    if (__argc < 2) {
        MessageBoxA(NULL, "There is no file for the note.", "FileNote", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Создание заметки
    char filepath[MAX_PATH];
    lstrcpyA(filepath, __argv[1]);

    char notePath[MAX_PATH];
    lstrcpyA(notePath, filepath);

    PathRemoveExtensionA(notePath);
    strcat(notePath, ".note.txt"); // stylish version of the note

    if (GetFileAttributesA(notePath) == INVALID_FILE_ATTRIBUTES) {
        std::ofstream ofs(notePath);
        ofs << "Note for file: " << PathFindFileNameA(filepath) << "\n";
    }

    ShellExecuteA(NULL, "open", notePath, NULL, NULL, SW_SHOWNORMAL);

    return 0;
}
