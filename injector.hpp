#pragma once
#include "common.hpp"
#include "stolen_injector_lmao.hpp"

class Injector
{
    HANDLE csgoHandle = NULL;
    DWORD csgoPID = 0;

public:
    Injector() {};

    void openCSGO()
    {
        while (true) // will break once this->csgoHandle is set
        {
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

            PROCESSENTRY32 entry{};
            entry.dwSize = sizeof(entry);

            for (bool moreProcesses = Process32First(snapshot, &entry); moreProcesses; moreProcesses = Process32Next(snapshot, &entry))
            {
                if (Utils::stringStartsWith(entry.szExeFile, L"csgo.exe"))
                {
                    this->csgoPID = entry.th32ProcessID;
                    this->csgoHandle = OpenProcess(PROCESS_ALL_ACCESS, false, csgoPID);
                    break;
                }
            }
            CloseHandle(snapshot);

            if (this->csgoPID) // found CS:GO
            {
                if (!this->csgoHandle) // but OpenProcess returned null
                {
                    MessageBox(0, L"Could not open CS:GO using OpenProcess. Try running the injector as administrator.", L"Injection Failed", MB_OK);
                    std::exit(0);
                }
                else // all is well
                {
                    break;
                }
            }

            Sleep(1000);
        }
    }

    void inject(char* dll, size_t dllSize)
    {
        const bool success = ManualMapDll(
            this->csgoHandle,
            (BYTE*)dll, // "char* is incompatible with BYTE*" man stfu
            dllSize
        );

        if (!success)
        {
            MessageBox(0, L"IDK why it would have failed, for now we're stealing an open source copy/pasted injector so I can't give more detailed error info. I'll re-write the injector as soon as I have time.", L"Injection Failed", MB_OK);
            std::exit(0);
        }
    }
};