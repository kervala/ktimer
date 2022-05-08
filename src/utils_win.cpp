/*
 *  kTimer is a timers manager
 *  Copyright (C) 2021  Cedric OCHS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common.h"
#include "utils.h"

#ifdef Q_OS_WIN

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>
#include <ShellAPI.h>
#include <sdkddkver.h>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

// Windows 2000 = GetModuleFileName()
// Windows XP x32 = GetProcessImageFileName()
// Windows XP x64 = GetProcessImageFileName()

//typedef BOOL (WINAPI *QueryFullProcessImageNamePtr)(HANDLE hProcess, DWORD dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);
//typedef DWORD (WINAPI *GetProcessImageFileNamePtr)(HANDLE hProcess, LPWSTR lpImageFileName, DWORD nSize);

//static QueryFullProcessImageNamePtr pQueryFullProcessImageName = NULL;
//static GetProcessImageFileNamePtr pGetProcessImageFileName = NULL;

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

bool IsOS64bits()
{
	bool res;

#ifdef _WIN644
	res = true;
#else
	res = false;

	// IsWow64Process is not available on all supported versions of Windows.
	// Use GetModuleHandle to get a handle to the DLL that contains the function
	// and GetProcAddress to get a pointer to the function if available.
	HMODULE module = GetModuleHandleA("kernel32");

	if (module)
	{
		LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(module, "IsWow64Process");

		if (fnIsWow64Process)
		{
			BOOL bIsWow64 = FALSE;

			if (fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
			{
				res = bIsWow64 == TRUE;
			}
		}
	}
#endif
	return res;
}

#endif
