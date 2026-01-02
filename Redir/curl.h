#pragma once

//#include "util.h"
#include "settings.h"        
#include <regex>
#include <string>
#include <Windows.h>
#include <Psapi.h>
#include <iostream>

#define HYBRID 0
#define CURLOPT_SSL_VERIFYPEER 64
#define CURLOPT_URL 10002
#define CURLE_BAD_FUNCTION_ARGUMENT 43

std::regex ohost("https:\\/\\/(.*)\\.ol\\.epicgames.com");
std::string nhost(PROXY_HOST);  

std::string routes[] = {
    "/content/api/pages/fortnite-game/",
    "/fortnite/api/game/v2/profile/",
    "/fortnite/api/cloudstorage/system",
    "/fortnite/api/v2/versioncheck/",
    "/content/api/page/",
    "/affiliate/api/public/affiliates/slug",
    "/socialban/api/public/v1"
};

int (*oCurlEasySetopt)(void* handle, int option, ...);

int hkCurlEasySetopt(void* handle, int option, va_list parameter) {
    if (!handle) return CURLE_BAD_FUNCTION_ARGUMENT;

    if (option == CURLOPT_SSL_VERIFYPEER) {
        return oCurlEasySetopt(handle, option, 0);
    }

    if (option == CURLOPT_URL) {
        const char* url = va_arg(parameter, const char*);
        std::string ourl(url ? url : "");

        for (const auto& route : routes) {
            if (ourl.find(route) != std::string::npos) {
                ourl = std::regex_replace(ourl, ohost, nhost);
                break;
            }
        }

        return oCurlEasySetopt(handle, option, ourl.c_str());
    }

    return oCurlEasySetopt(handle, option, parameter);
}

uintptr_t sigscan(const char* pattern) {
    MODULEINFO modInfo;
    HMODULE hModule = GetModuleHandle(NULL);
    GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO));

    uintptr_t base = (uintptr_t)modInfo.lpBaseOfDll;
    uintptr_t size = (uintptr_t)modInfo.SizeOfImage;

    size_t pattern_len = strlen(pattern);
    for (uintptr_t i = base; i < base + size - pattern_len; i++) {
        if (memcmp((void*)i, pattern, pattern_len) == 0) {
            return i;
        }
    }

    return 0;
}

void hookCurl() {
    const char* pattern = "\x89\x54\x24\x10\x4C\x89\x44\x24\x18\x4C\x89\x4C\x24\x20\x48\x83\xEC\x28\x48\x85\xC9\x75\x08\x8D\x41\x2B\x48\x83\xC4\x28\xC3\x4C";
    uintptr_t aCurlEasySetopt = sigscan(pattern);

    if (aCurlEasySetopt == 0) {
        MessageBox(NULL, L"Failed to find curl_easy_setopt! (Pattern may be outdated for current season)", L"Error", MB_OK);
        return;
    }

    DWORD oldProtect;
    if (!VirtualProtect((LPVOID)aCurlEasySetopt, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        std::cerr << "Failed to change memory protection!" << std::endl;
        return;
    }
     
    *(uintptr_t*)aCurlEasySetopt = (uintptr_t)hkCurlEasySetopt;

    VirtualProtect((LPVOID)aCurlEasySetopt, sizeof(void*), oldProtect, &oldProtect);

    std::wcout << L"curl_easy_setopt hooked at address: 0x" << std::hex << aCurlEasySetopt << std::endl;
}