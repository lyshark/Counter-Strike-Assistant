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

// ------------------------------------------------------------
// 全局结构体
// ------------------------------------------------------------

DWORD ProcessPID = 0;
DWORD ProcessHandle = 0;

DWORD engine模块句柄 = 0;
DWORD 矩阵基地址 = 0;

// ------------------------------------------------------------
// 调用方法
// ------------------------------------------------------------

// 打开进程
HANDLE GetProcessHandle(DWORD pid)
{
	return OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
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

// 十六转十进制
int 十六转十进制(char *str)
{
	// 跳过0X
	char *s = str + 2;

	// 获取当前的长度后减1
	int num = strlen(s) - 1;
	int sum = 0;
	int sum1 = 0;
	for (int i = num; i >= 0; i--)
	{
		// 从高位获取它的值
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

		// 根据位数转化为10进制
		for (int j = 1; j < i + 1; j++)
		{
			// 高一位等于低一位的16
			sum *= 16;
		}

		// 每一位的十进制相加
		sum1 += sum;
	}

	// 返回最终结果
	return sum1;
}

// 4*4 竖矩阵转为横矩阵
VOID 竖矩阵转横矩阵(IN FLOAT InViewWorld[4][4], OUT FLOAT OutViewWorld[4][4])
{
	/*
	ViewWorld[0][0] = *(FLOAT*)(OutViewWorld);
	ViewWorld[0][1] = *(FLOAT*)(OutViewWorld+0x4);
	ViewWorld[0][2] = *(FLOAT*)(OutViewWorld+0x8);
	ViewWorld[0][3] = *(FLOAT*)(OutViewWorld+0xc);

	ViewWorld[1][0] = *(FLOAT*)(OutViewWorld+0x10);
	ViewWorld[1][1] = *(FLOAT*)(OutViewWorld + 0x14);
	ViewWorld[1][2] = *(FLOAT*)(OutViewWorld + 0x18);
	ViewWorld[1][3] = *(FLOAT*)(OutViewWorld + 0x1c);

	ViewWorld[2][0] = *(FLOAT*)(OutViewWorld + 0x20);
	ViewWorld[2][1] = *(FLOAT*)(OutViewWorld + 0x24);
	ViewWorld[2][2] = *(FLOAT*)(OutViewWorld + 0x28);
	ViewWorld[2][3] = *(FLOAT*)(OutViewWorld + 0x2c);

	ViewWorld[3][0] = *(FLOAT*)(OutViewWorld + 0x30);
	ViewWorld[3][1] = *(FLOAT*)(OutViewWorld + 0x34);
	ViewWorld[3][2] = *(FLOAT*)(OutViewWorld + 0x38);
	ViewWorld[3][3] = *(FLOAT*)(OutViewWorld + 0x3c);
	*/

	/*
	转换前 = 竖矩阵
	[0][0] | [0][1] | [0][2] | [0][3]
	[1][0] | [1][1] | [1][2] | [1][3]
	[2][0] | [2][1] | [2][2] | [2][3]
	[3][0] | [3][1] | [3][2] | [3][3]

	转换后 = 横矩阵
	[0][0] | [0][1] | [0][2] | [3][0]
	[1][0] | [1][1] | [1][2] | [3][1]
	[2][0] | [2][1] | [2][2] | [3][2]
	[0][3] | [1][3] | [2][3] | [3][3]
	*/

	// 横矩阵[0][0] = 竖矩阵[0][0]
	OutViewWorld[0][0] = InViewWorld[0][0];

	// 横矩阵[0][1] = 竖矩阵[0][1]
	OutViewWorld[0][1] = InViewWorld[0][1];

	// 横矩阵[0][2] = 竖矩阵[0][2]
	OutViewWorld[0][2] = InViewWorld[0][2];

	// 横矩阵[3][0] = 竖矩阵[0][3]
	OutViewWorld[3][0] = InViewWorld[0][3];

	// ---------------------------------------------------------
	// 横矩阵[1][0] = 竖矩阵[1][0]
	OutViewWorld[1][0] = InViewWorld[1][0];

	// 横矩阵[1][1] = 竖矩阵[1][1]
	OutViewWorld[1][1] = InViewWorld[1][1];

	// 横矩阵[1][2] = 竖矩阵[1][2]
	OutViewWorld[1][2] = InViewWorld[1][2];

	// 横矩阵[3][1] = 竖矩阵[1][3]
	OutViewWorld[3][1] = InViewWorld[1][3];

	// ---------------------------------------------------------

	// 横矩阵[2][0] = 竖矩阵[2][0]
	OutViewWorld[2][0] = InViewWorld[2][0];

	// 横矩阵[2][1] = 竖矩阵[2][1]
	OutViewWorld[2][1] = InViewWorld[2][1];

	// 横矩阵[2][2] = 竖矩阵[2][2]
	OutViewWorld[2][2] = InViewWorld[2][2];

	// 横矩阵[3][2] = 竖矩阵[2][3]
	OutViewWorld[3][2] = InViewWorld[2][3];

	// ---------------------------------------------------------

	// 横矩阵[0][3] = 竖矩阵[3][0]
	OutViewWorld[0][3] = InViewWorld[3][0];

	// 横矩阵[1][3] = 竖矩阵[3][1]
	OutViewWorld[1][3] = InViewWorld[3][1];

	// 横矩阵[2][3] = 竖矩阵[3][2]
	OutViewWorld[2][3] = InViewWorld[3][2];

	// 横矩阵[3][3] = 竖矩阵[3][3]
	OutViewWorld[3][3] = InViewWorld[3][3];
}

// 4*4 横矩阵转为竖矩阵
VOID 横矩阵转为竖矩阵(IN FLOAT InViewWorld[4][4], OUT FLOAT OutViewWorld[4][4])
{
	/*
	转换前 = 横矩阵
	[0][0] | [0][1] | [0][2] | [3][0]
	[1][0] | [1][1] | [1][2] | [3][1]
	[2][0] | [2][1] | [2][2] | [3][2]
	[0][3] | [1][3] | [2][3] | [3][3]

	转换后 = 竖矩阵
	[0][0] | [0][1] | [0][2] | [0][3]
	[1][0] | [1][1] | [1][2] | [1][3]
	[2][0] | [2][1] | [2][2] | [2][3]
	[3][0] | [3][1] | [3][2] | [3][3]
	*/
}

int main(int argc, char *argv[])
{
	ProcessPID = GetProcessIdByName(L"hl2.exe");
	ProcessHandle = (DWORD)GetProcessHandle(ProcessPID);

	engine模块句柄 = GetProcessModuleHandle(ProcessPID, L"engine.dll");
	矩阵基地址 = engine模块句柄 + 十六转十进制("0x596EF0");

	printf("模块句柄: %d --> 矩阵基址: 0x%x \n\n", engine模块句柄, 矩阵基地址);

	FLOAT ViewWorld[4][4] = { 0 };
	FLOAT HengViewWorld[4][4] = { 0 };
	FLOAT ShuViewWorld[4][4] = { 0 };

	// 读入竖矩阵
	ReadProcessMemory((HANDLE)ProcessHandle, (PVOID)矩阵基地址, ViewWorld, 64, 0);

	printf("默认竖矩阵: \n");
	printf("[0][0] = %.4f | [0][1] = %.4f | [0][2] = %.4f | [0][3] = %.4f \n", ViewWorld[0][0], ViewWorld[0][1], ViewWorld[0][2], ViewWorld[0][3]);
	printf("[1][0] = %.4f | [1][1] = %.4f | [1][2] = %.4f | [1][3] = %.4f \n", ViewWorld[1][0], ViewWorld[1][1], ViewWorld[1][2], ViewWorld[1][3]);
	printf("[2][0] = %.4f | [2][1] = %.4f | [2][2] = %.4f | [2][3] = %.4f \n", ViewWorld[2][0], ViewWorld[2][1], ViewWorld[2][2], ViewWorld[2][3]);
	printf("[3][0] = %.4f | [3][1] = %.4f | [3][2] = %.4f | [3][3] = %.4f \n", ViewWorld[3][0], ViewWorld[3][1], ViewWorld[3][2], ViewWorld[3][3]);
	printf("\n\n");

	竖矩阵转横矩阵(ViewWorld, HengViewWorld);

	printf("转为横矩阵: \n");
	printf("[0][0] = %.4f | [0][1] = %.4f | [0][2] = %.4f | [0][3] = %.4f \n", HengViewWorld[0][0], HengViewWorld[0][1], HengViewWorld[0][2], HengViewWorld[0][3]);
	printf("[1][0] = %.4f | [1][1] = %.4f | [1][2] = %.4f | [1][3] = %.4f \n", HengViewWorld[1][0], HengViewWorld[1][1], HengViewWorld[1][2], HengViewWorld[1][3]);
	printf("[2][0] = %.4f | [2][1] = %.4f | [2][2] = %.4f | [2][3] = %.4f \n", HengViewWorld[2][0], HengViewWorld[2][1], HengViewWorld[2][2], HengViewWorld[2][3]);
	printf("[3][0] = %.4f | [3][1] = %.4f | [3][2] = %.4f | [3][3] = %.4f \n", HengViewWorld[3][0], HengViewWorld[3][1], HengViewWorld[3][2], HengViewWorld[3][3]);
	printf("\n\n");

	getchar();
	return 0;
}