#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <math.h>

// ------------------------------------------------------------
// Global Structure
// ------------------------------------------------------------

DWORD ProcessPID = 0;
DWORD ProcessHandle = 0;

DWORD engineÄ£¿é¾ä±ú = 0;
DWORD ¾ØÕó»ùµØÖ· = 0;

// ------------------------------------------------------------
// call method
// ------------------------------------------------------------

// Open process
HANDLE GetProcessHandle(DWORD pid)
{
	return OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
}

// Retrieve process PID
DWORD GetProcessIdByName(LPCTSTR name)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return NULL;
	}
	PROCESSENTRY32 pe = { sizeof(pe) };

	for (BOOL ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe))
	{
		if (lstrcmpi(pe.szExeFile, name) == 0)
		{
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
	}
	CloseHandle(hSnapshot);
	return 0;
}

// Retrieve the handle of the specified module
DWORD GetProcessModuleHandle(DWORD pid, CONST TCHAR* moduleName)
{
	MODULEENTRY32 moduleEntry;
	HANDLE handle = NULL;
	handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (!handle)
	{
		CloseHandle(handle);
		return NULL;
	}

	ZeroMemory(&moduleEntry, sizeof(MODULEENTRY32));
	moduleEntry.dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(handle, &moduleEntry))
	{
		CloseHandle(handle);
		return NULL;
	}

	do
	{
		if (lstrcmpi(moduleEntry.szModule, moduleName) == 0)
		{
			return (DWORD)moduleEntry.hModule;
		}
	} while (Module32Next(handle, &moduleEntry));
	CloseHandle(handle);
	return 0;
}

// Sixteen to decimal conversion
int Ê®Áù×ªÊ®½øÖÆ(char *str)
{
	char *s = str + 2;

	int num = strlen(s) - 1;
	int sum = 0;
	int sum1 = 0;
	for (int i = num; i >= 0; i--)
	{
		switch (s[num - i])
		{
		case '0': sum = 0; break;
		case '1': sum = 1; break;
		case '2': sum = 2; break;
		case '3': sum = 3; break;
		case '4': sum = 4; break;
		case '5': sum = 5; break;
		case '6': sum = 6; break;
		case '7': sum = 7; break;
		case '8': sum = 8; break;
		case '9': sum = 9; break;
		case 'a': sum = 10; break;
		case 'b': sum = 11; break;
		case 'c': sum = 12; break;
		case 'd': sum = 13; break;
		case 'e': sum = 14; break;
		case 'f': sum = 15; break;
		case 'A': sum = 10; break;
		case 'B': sum = 11; break;
		case 'C': sum = 12; break;
		case 'D': sum = 13; break;
		case 'E': sum = 14; break;
		case 'F': sum = 15; break;
		}

		for (int j = 1; j < i + 1; j++)
		{
			sum *= 16;
		}

		sum1 += sum;
	}

	return sum1;
}

int main(int argc, char *argv[])
{
	ProcessPID = GetProcessIdByName(L"hl2.exe");
	ProcessHandle = (DWORD)GetProcessHandle(ProcessPID);

	engineÄ£¿é¾ä±ú = GetProcessModuleHandle(ProcessPID, L"engine.dll");
	¾ØÕó»ùµØÖ· = engineÄ£¿é¾ä±ú + Ê®Áù×ªÊ®½øÖÆ("0x596EF0");

	printf("Ä£¿é¾ä±ú: %d --> ¾ØÕó»ùÖ·: 0x%x \n\n", engineÄ£¿é¾ä±ú, ¾ØÕó»ùµØÖ·);

	FLOAT ViewWorld[4][4] = { 0 };
	while (1)
	{
		// ¶ÁÈë¾ØÕó [Êú¾ØÕó]
		// ¾ØÕó ViewWorld[4][4] => 4*4=16 => 16*word = 64
		ReadProcessMemory((HANDLE)ProcessHandle, (PVOID)¾ØÕó»ùµØÖ·, ViewWorld, 64, 0);

		printf("[0][0] = %.4f | [0][1] = %.4f | [0][2] = %.4f | [0][3] = %.4f \n", ViewWorld[0][0], ViewWorld[0][1], ViewWorld[0][2], ViewWorld[0][3]);
		printf("[1][0] = %.4f | [1][1] = %.4f | [1][2] = %.4f | [1][3] = %.4f \n", ViewWorld[1][0], ViewWorld[1][1], ViewWorld[1][2], ViewWorld[1][3]);
		printf("[2][0] = %.4f | [2][1] = %.4f | [2][2] = %.4f | [2][3] = %.4f \n", ViewWorld[2][0], ViewWorld[2][1], ViewWorld[2][2], ViewWorld[2][3]);
		printf("[3][0] = %.4f | [3][1] = %.4f | [3][2] = %.4f | [3][3] = %.4f \n", ViewWorld[3][0], ViewWorld[3][1], ViewWorld[3][2], ViewWorld[3][3]);
		printf("\n\n");
		Sleep(2000);
	}

	return 0;
}