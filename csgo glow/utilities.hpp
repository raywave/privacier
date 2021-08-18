#pragma once
#include <cstddef>
#include <windows.h>
#include <tlhelp32.h>
#include "lazy_importer.hpp"

namespace utilities {
	__forceinline auto getPid(const char* processName) -> DWORD
	{
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = LI_FN(CreateToolhelp32Snapshot).get()(TH32CS_SNAPPROCESS, NULL);

        if (LI_FN(Process32First).get()(snapshot, &entry) == TRUE)
        {
            while (LI_FN(Process32Next).get()(snapshot, &entry) == TRUE)
            {
                if (_stricmp(entry.szExeFile, processName) == 0)
                {
                    LI_FN(CloseHandle).get()(snapshot);

                    return entry.th32ProcessID;
                }
            }
        }

        LI_FN(CloseHandle).get()(snapshot);

        return 0;
	}
}