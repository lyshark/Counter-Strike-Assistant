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

// 打开进程
HANDLE GetProcessHandle(DWORD pid)
{
	return OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
}

// 获取ProcessPID
DWORD GetProcessIdByHwnd(DWORD hWnd)
{
	DWORD pid;
	GetWindowThreadProcessId((HWND)hWnd, &pid);
	return pid;
}

// 取ProcessPID
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

// 取指定模块句柄
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

// 读取类型: 整数型 短整数型 浮点型 字节型
template<typename T1>
T1 ReadMemory(HANDLE processHandle, DWORD pAddr)
{
	T1 result;
	ReadProcessMemory(processHandle, (LPCVOID)(pAddr), &result, sizeof(T1), NULL);
	return result;
}

// 读取类型: 自定义大小内存
template<typename T1>
T1* ReadMemory(HANDLE processHandle, DWORD pAddr, DWORD length)
{
	T1* result = new T1[length];
	ReadProcessMemory(processHandle, (LPCVOID)(pAddr), result, length, NULL);
	return result;
}

// 写类型: 整数型 短整数型 浮点型 字节型
template<typename T1>
void WriteMemory(HANDLE processHandle, DWORD pAddr, T1 vaule)
{
	WriteProcessMemory(processHandle, (LPVOID)(pAddr), &vaule, sizeof(T1), NULL);
}

// 写类型: 字节集
template<typename T1>
void WriteMemory(HANDLE processHandle, DWORD pAddr, T1 vaule, DWORD length)
{
	WriteProcessMemory(processHandle, (LPVOID)(pAddr), vaule, length, NULL);
}

// 分配内存
DWORD AllocMemory(HANDLE processHandle, DWORD size)
{
	return (DWORD)VirtualAllocEx(processHandle, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

// 释放内存
BOOL FreeMemory(HANDLE processHandle, DWORD addr, DWORD size)
{
	return VirtualFreeEx(processHandle, (LPVOID)addr, size, MEM_RELEASE);
}

// 特征查找
uintptr_t FindPattern(HANDLE processHandle, uintptr_t start, uintptr_t length, const unsigned char* pattern, const char* mask)
{
	size_t pos = 0;
	auto maskLength = strlen(mask) - 1;

	auto startAdress = start;
	for (auto it = startAdress; it < startAdress + length; ++it)
	{
		if (ReadMemory<unsigned char>(processHandle, (DWORD)it) == pattern[pos] || mask[pos] == '?')
		{
			if (mask[pos + 1] == '\0')
				return it - maskLength;

			pos++;
		}
		else pos = 0;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	DWORD Pid = GetProcessIdByName(L"hl2.exe");
	printf("ProcessPID = %d \n", Pid);

	HANDLE handle = GetProcessHandle(Pid);
	printf("进程句柄 = %X \n", handle);

	DWORD server = GetProcessModuleHandle(Pid, L"server.dll");
	printf("模块句柄 = %X \n", server);

	// -----------------------------------------------------------
	// 进程读内存
	// -----------------------------------------------------------

	// 测试读结构
	IMAGE_DOS_HEADER DOSHeader = ReadMemory<IMAGE_DOS_HEADER>(handle, (DWORD)server);
	printf("DOS Header = %X \n", DOSHeader);

	IMAGE_NT_HEADERS NTHeaders = ReadMemory<IMAGE_NT_HEADERS>(handle, DWORD(uintptr_t(server) + DOSHeader.e_lfanew));
	printf("NT Header = %X \n", NTHeaders);

	BYTE find_code[10] = { 0xC7, 0x86, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	const char* mask = "?? ?? ?? ?? ?? ?? ?? ?? ?? ??";

	uintptr_t param = FindPattern(handle,
		reinterpret_cast<uintptr_t>(handle)+NTHeaders.OptionalHeader.BaseOfCode,
		reinterpret_cast<uintptr_t>(handle)+NTHeaders.OptionalHeader.SizeOfCode,
		find_code,
		mask
		);

	printf("param = %d \n", param);

	// 测试读整数
	DWORD read_dword = ReadMemory<DWORD>(handle, 0x2A8E7FC4);
	printf("读内存整数型 = %d \n", read_dword);

	// 测试读浮点数
	FLOAT read_float = ReadMemory<FLOAT>(handle, 0x2A8E7FC4);
	printf("读内存浮点数 = %f \n", read_float);

	// 测试读字节
	BYTE read_byte = ReadMemory<BYTE>(handle, 0x2A8E7FC4);
	printf("读内存字节 = %02X \n", read_byte);

	// 测试读字节集
	BYTE** read_byte_ptr = ReadMemory<BYTE *>(handle, 0x2A8E7FC4, 10);

	for (int x = 0; x < 10; x++)
	{
		printf("读[%d]字节集 = %02X \n", x, read_byte_ptr[x]);
	}

	// -----------------------------------------------------------
	// 进程写内存
	// -----------------------------------------------------------
	// 写内存整数型
	WriteMemory<DWORD>(handle, 0x2A8E7FC4, (DWORD)100);

	// 写内存字节集

	DWORD addr = 0x2A8E7FC4;
	DWORD length = 10;

	BYTE code[10] = { 0xC7, 0x86, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	DWORD old;
	if (VirtualProtectEx(handle, (LPVOID)addr, length, PAGE_EXECUTE_READWRITE, &old))
	{
		BYTE* temp = (BYTE*)addr;
		for (int i = 0; i < length; i++)
		{
			WriteMemory<BYTE>(handle, DWORD(temp + i), code[i]);

		}
	}
	VirtualProtectEx(handle, (LPVOID)addr, length, old, NULL);

	getchar();
	return 0;
}