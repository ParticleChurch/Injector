#pragma once

#include <filesystem>
#include <string>

#include <Windows.h>

namespace OS {
    namespace filesystem = std::filesystem;

    inline std::string system(std::string cmd, int* returnCode = nullptr)
    {
        // we will spawn a new cmd.exe to ensure that no window appears
        cmd = "cmd.exe /c " + cmd;

        // declare vars
        SECURITY_ATTRIBUTES securityAttributes;
        PROCESS_INFORMATION processInfo;
        STARTUPINFOA startupInfo;
        HANDLE hPipeRead = INVALID_HANDLE_VALUE, hPipeWrite = INVALID_HANDLE_VALUE;

        // initialize vars
        securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        securityAttributes.bInheritHandle = true;
        securityAttributes.lpSecurityDescriptor = nullptr;

        ZeroMemory(&processInfo, sizeof(processInfo));
        ZeroMemory(&startupInfo, sizeof(startupInfo));

        if (!CreatePipe(&hPipeRead, &hPipeWrite, &securityAttributes, 0))
        {
            if (returnCode) *returnCode = -1;
            return "";
        }

        startupInfo.cb = sizeof(startupInfo);
        startupInfo.hStdInput = 0;
        startupInfo.hStdOutput = hPipeWrite;
        startupInfo.hStdError = hPipeWrite;
        startupInfo.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        startupInfo.wShowWindow = SW_HIDE;

        // make a copy because CreateProcess might edit the data
        // and add null terminator to mimic c_str
        bool success = CreateProcessA(
            nullptr,
            cmd.data(),
            nullptr,
            nullptr,
            true,
            CREATE_NEW_CONSOLE,
            nullptr,
            nullptr,
            &startupInfo,
            &processInfo
        );

        if (!success) {
            CloseHandle(hPipeRead);
            CloseHandle(hPipeWrite);

            if (returnCode) *returnCode = GetLastError();
            return "Error: CreateProcessA falsy return. Error Code: " + std::to_string(GetLastError());
        }

        std::string output = "";
        char buffer[1024];
        bool processExited = false;
        while (!processExited)
        {
            processExited = WaitForSingleObject(processInfo.hProcess, 10) == WAIT_OBJECT_0;

            while (true)
            {
                size_t bytesRead = 0;
                size_t bytesAvailable = 0;

                if (!PeekNamedPipe(hPipeRead, nullptr, 0, nullptr, (DWORD*)&bytesAvailable, nullptr) || bytesAvailable == 0)
                    break;

                if (!ReadFile(hPipeRead, buffer, min((size_t)1024, bytesAvailable), (DWORD*)&bytesRead, nullptr) || bytesRead == 0)
                    break;

                output += std::string(buffer, bytesRead);
            }
        }

        if (returnCode && !GetExitCodeProcess(processInfo.hProcess, (DWORD*)&returnCode)) {
            *returnCode = GetLastError();
            return "Error: GetExitCodeProcess falsy return. Error Code: " + std::to_string(*returnCode);
        }

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(hPipeRead);
        CloseHandle(hPipeWrite);

        return output;
    }
}
