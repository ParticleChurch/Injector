#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

namespace ProcessFinder {
	inline bool processLikelyCSGO(PROCESSENTRY32& entry) {
		std::string exeName(entry.szExeFile);
		std::string csgo = "csgo.exe";

		if (exeName.length() < csgo.length()) return false;
		if (exeName.substr(exeName.size() - csgo.size()) != csgo) return false;

		return true;
	}

	inline DWORD findCSGOPID() {
		HANDLE proc;
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		DWORD found = 0;

		PROCESSENTRY32 entry{};
		entry.dwSize = sizeof(entry);
		do
		{
			if (processLikelyCSGO(entry)) {
				found = entry.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &entry));
		
		CloseHandle(snapshot);

		return found;
	}
}