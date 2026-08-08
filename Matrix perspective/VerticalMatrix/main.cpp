/**
* @Author: LYSHARK
* @Website: https://www.lyshark.com
* @Description: 本项目仅用于技术研究、技术交流、技术探索，用于启发技术学习与技术思路探索。
*
* 【重要法律声明与使用限制】
* 1. 本项目所有代码、资源仅作学习研究用途，严禁将源代码、修改后的代码片段直接或间接用于任何违反国家法律法规的场景。
* 2. 使用者一旦使用、复制、修改、分发本项目代码，即表示完全接受本全部声明条款。
* 3. 因使用者违规使用本代码所产生的一切直接、间接后果，全部由使用者本人独立承担，项目作者不承担任何法律及连带责任。
* 4. 未经作者许可，禁止将本项目用于商业项目、盈利产品、非法测试、未授权攻击等行为。
* 5. 本代码受著作权相关法律保护，未经许可不得篡改、移除本头部版权声明。
*
* 【免责提示】
* 本项目代码按现状提供，不提供任何明示或暗示担保，不对运行风险、使用后果承担责任。
*/

#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <math.h>

DWORD ProcessPID = 0;
DWORD ProcessHandle = 0;

DWORD engine模块句柄 = 0;
DWORD 矩阵基地址 = 0;

HANDLE GetProcessHandle(DWORD pid)
{
	return OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
}

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

int 十六转十进制(char *str)
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

	engine模块句柄 = GetProcessModuleHandle(ProcessPID, L"engine.dll");
	矩阵基地址 = engine模块句柄 + 十六转十进制("0x596EF0");

	printf("模块句柄: %d --> 矩阵基址: 0x%x \n\n", engine模块句柄, 矩阵基地址);

	FLOAT ViewWorld[4][4] = { 0 };
	while (1)
	{
		// 读入矩阵 [竖矩阵]
		// 矩阵 ViewWorld[4][4] => 4*4=16 => 16*word = 64
		ReadProcessMemory((HANDLE)ProcessHandle, (PVOID)矩阵基地址, ViewWorld, 64, 0);

		printf("[0][0] = %.4f | [0][1] = %.4f | [0][2] = %.4f | [0][3] = %.4f \n", ViewWorld[0][0], ViewWorld[0][1], ViewWorld[0][2], ViewWorld[0][3]);
		printf("[1][0] = %.4f | [1][1] = %.4f | [1][2] = %.4f | [1][3] = %.4f \n", ViewWorld[1][0], ViewWorld[1][1], ViewWorld[1][2], ViewWorld[1][3]);
		printf("[2][0] = %.4f | [2][1] = %.4f | [2][2] = %.4f | [2][3] = %.4f \n", ViewWorld[2][0], ViewWorld[2][1], ViewWorld[2][2], ViewWorld[2][3]);
		printf("[3][0] = %.4f | [3][1] = %.4f | [3][2] = %.4f | [3][3] = %.4f \n", ViewWorld[3][0], ViewWorld[3][1], ViewWorld[3][2], ViewWorld[3][3]);
		printf("\n\n");
		Sleep(2000);
	}

	return 0;
}