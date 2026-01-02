
#include <windows.h>
#include <string>
#include "pch.h"
#include <iostream> 
// used to make std:cout working

BOOL APIENTRY DllMain(HMODULE module, DWORD call_reason, LPVOID reserved) {
    if (call_reason == DLL_PROCESS_ATTACH) {
        FILE* pFile;
        freopen_s(&pFile, "CONOUT$", "w", stdout);
        freopen_s(&pFile, "CONOUT$", "w", stderr);
        freopen_s(&pFile, "CONIN$", "r", stdin);
        std::cout << "Redir" << std::endl;
    }
    return TRUE;
}