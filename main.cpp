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

    // Ключ FileNote
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

    // Ключ command
    if (RegCreateKeyExA(HKEY_CURRENT_USER, cmdPath, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, NULL, 0, REG_SZ,
                       (const BYTE*)commandStr.c_str(),
                       (DWORD)commandStr.length() + 1);
        RegCloseKey(hKey);
    }

    // Создаём файл filenote_uninstall.cmd рядом с exe
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
    // Удаляем ключи реестра
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\*\\shell\\FileNote\\command");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\*\\shell\\FileNote");

    // Определяем путь к исполняемому файлу
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // Создаём путь к filenote_uninstall.cmd
    std::string uninstallCmdPath = exePath;
    size_t lastSlash = uninstallCmdPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        uninstallCmdPath = uninstallCmdPath.substr(0, lastSlash + 1) + "filenote_uninstall.cmd";
    }

    // Проверяем, существует ли файл, и удаляем
    if (GetFileAttributesA(uninstallCmdPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        DeleteFileA(uninstallCmdPath.c_str());
    }

    MessageBoxA(NULL, "Пункт FileNote удалён из контекстного меню.", "FileNote", MB_OK | MB_ICONINFORMATION);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    // Если запущено с аргументом --uninstall
    if (__argc >= 2 && strcmp(__argv[1], "--uninstall") == 0) {
        RemoveRegistryEntry();
        return 0;
    }

    bool registryExists = IsRegistryEntryExists();

    // Если запись в реестре отсутствует, создаём её и .cmd для удаления
    if (!registryExists) {
        CreateRegistryEntry();
        MessageBoxA(NULL, "Путь к программе зарегистрирован для контекстного меню.", "FileNote", MB_OK | MB_ICONINFORMATION);
        return 0; // выходим, не создавая заметку
    }

    // Если программа запущена без аргумента и запись уже есть
    if (__argc < 2) {
        MessageBoxA(NULL, "Нет файла для заметки.", "FileNote", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Создание заметки
    char filepath[MAX_PATH];
    lstrcpyA(filepath, __argv[1]);

    char notePath[MAX_PATH];
    lstrcpyA(notePath, filepath);

    PathRemoveExtensionA(notePath);
    strcat(notePath, ".note.txt"); // стильный вариант заметки

    if (GetFileAttributesA(notePath) == INVALID_FILE_ATTRIBUTES) {
        std::ofstream ofs(notePath);
        ofs << "Заметка для файла: " << PathFindFileNameA(filepath) << "\n";
    }

    ShellExecuteA(NULL, "open", notePath, NULL, NULL, SW_SHOWNORMAL);

    return 0;
}
