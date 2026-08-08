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

DWORD 游戏窗口宽度 = 1024;
DWORD 游戏窗口高度 = 768;

DWORD Engine模块句柄 = 0;
DWORD Client模块句柄 = 0;
DWORD Server模块句柄 = 0;

DWORD 矩阵基地址 = 0;
DWORD 敌人对象 = 0;
DWORD 本人基地址 = 0;

typedef struct
{
	FLOAT X轴;
	FLOAT Y轴;
}D2D坐标;

typedef struct
{
	FLOAT X坐标;
	FLOAT Y坐标;
	FLOAT Z坐标;
}D3D坐标;

typedef struct
{
	DWORD 左边;
	DWORD 顶边;
	DWORD 宽度;
	DWORD 高度;
}方框数据;

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

FLOAT 求平方根(FLOAT x)
{
	return sqrt(x);
}

FLOAT 取整(FLOAT x)
{
	return floor(x);
}

// 读取类型: 整数型 短整数型 浮点型 字节型
template<typename T1>
T1 ReadMemory(HANDLE processHandle, DWORD pAddr)
{
	T1 result;
	ReadProcessMemory(processHandle, (LPCVOID)(pAddr), &result, sizeof(T1), NULL);
	return result;
}

// ------------------------------------------------------------
// 透视代码
// ------------------------------------------------------------

// 初始化基址
VOID 初始化部分()
{
	ProcessPID = GetProcessIdByName(L"hl2.exe");
	ProcessHandle = (DWORD)GetProcessHandle(ProcessPID);

	Engine模块句柄 = GetProcessModuleHandle(ProcessPID, L"engine.dll");
	Server模块句柄 = GetProcessModuleHandle(ProcessPID, L"server.dll");
	Client模块句柄 = GetProcessModuleHandle(ProcessPID, L"client.dll");

	本人基地址 = ReadMemory<DWORD>((HANDLE)ProcessHandle, Server模块句柄 + 十六转十进制("0x4F2FEC"));
	矩阵基地址 = Engine模块句柄 + 十六转十进制("0x596EF0");


	printf("模块句柄: %x --> 矩阵基址: 0x%x --> 本人基址: 0x%x \n", Engine模块句柄, 矩阵基地址, 本人基地址);
}

// 获取准星距离
FLOAT 取准星距离(FLOAT 准星X, FLOAT 准星Y, FLOAT 二维对象X, FLOAT 二维对象Y)
{
	FLOAT 距离X = 0;
	FLOAT 距离Y = 0;
	FLOAT 距离 = 0;

	距离X = 准星X - 二维对象X;
	距离Y = 准星Y - 二维对象Y;

	距离 = 求平方根(距离X * 距离X + 距离Y * 距离Y);

	return 距离;
}

// 获取敌人距离
FLOAT 取敌人距离(D3D坐标 个人坐标, D3D坐标 对象坐标)
{
	FLOAT 距离X = 0;
	FLOAT 距离Y = 0;
	FLOAT 距离 = 0;

	距离X = 个人坐标.X坐标 - 对象坐标.X坐标;
	距离Y = 个人坐标.Y坐标 - 对象坐标.Y坐标;

	距离 = 求平方根(距离X * 距离X + 距离Y * 距离Y);

	return 距离;
}

// 计算透视方框位置
VOID 计算透视方框()
{
	FLOAT ViewWorld[4][4] = { 0 };

	FLOAT 相机X = 0;
	FLOAT 相机Y = 0;
	FLOAT 相机Y2 = 0;
	FLOAT 相机Z = 0;

	DWORD 视角宽度 = 0;
	DWORD 视角高度 = 0;

	D3D坐标 本人坐标 = { 0 };
	D3D坐标 敌人坐标 = { 0 };

	DWORD 本人阵营 = 0;
	DWORD 敌人阵营 = 0;

	DWORD 本人基地址 = 0;
	DWORD 敌人基地址 = 0;

	FLOAT 缩放比例 = 0;
	FLOAT 方框高度 = 0;
	FLOAT 方框宽度 = 0;

	方框数据 方框坐标数据 = { 0 };

	DWORD 人物循环 = 0;
	DWORD 玩家个数 = 0;
	DWORD 敌人血量 = 0;

	FLOAT 敌人准星距离 = 0;
	FLOAT 三维人敌距离 = 0;

	FLOAT 自瞄最大距离 = 0;

	D3D坐标 自瞄坐标 = { 0 };
	D2D坐标 自瞄鼠标 = { 0 };
	DWORD 自瞄血量 = { 0 };

	// 读取出竖矩阵数据
	// 矩阵 ViewWorld[4][4] => 4*4=16 => 16*word = 64
	ReadProcessMemory((HANDLE)ProcessHandle, (PVOID)矩阵基地址, ViewWorld, 64, 0);

	// 视角也就是屏幕的一半
	视角宽度 = 游戏窗口宽度 / 2;
	视角高度 = 游戏窗口高度 / 2;

	自瞄最大距离 = 10000;

	// 获取本人坐标数据
	本人基地址 = ReadMemory<DWORD>((HANDLE)ProcessHandle, Server模块句柄 + 十六转十进制("0x4F2FEC"));
	本人阵营 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 本人基地址 + 500);

	本人坐标.X坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 648 - 8);
	本人坐标.Y坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 648 - 4);
	本人坐标.Z坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 648);

	// 循环玩家数量
	玩家个数 = ReadMemory<DWORD>((HANDLE)ProcessHandle, Server模块句柄 + 十六转十进制("0x588878"));

	for (int x = 0; x <= 玩家个数; 人物循环 = x, x++)
	{
		// 得到敌人阵营,判断是否是自己人,如果是自己人则跳过绘制
		敌人基地址 = ReadMemory<DWORD>((HANDLE)ProcessHandle, Server模块句柄 + 十六转十进制("0x4F2FFC") + (人物循环 - 1) * 16);
		敌人阵营 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 敌人基地址 + 500);
		if (本人阵营 == 敌人阵营)
		{
			continue;
		}

		// 判断敌人是否死亡,如果死亡则不需要绘制了
		敌人血量 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 敌人基地址 + 228);
		if (敌人血量 == 0 || 敌人血量 <= 1)
		{
			continue;
		}

		// 获取敌人坐标数据
		敌人坐标.X坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人基地址 + 648 - 8);
		敌人坐标.Y坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人基地址 + 648 - 4);
		敌人坐标.Z坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人基地址 + 648);

		// 竖矩阵算法，取出来的是本人坐标数据
		相机X = ViewWorld[2][0] * 敌人坐标.X坐标 + ViewWorld[2][1] * 敌人坐标.Y坐标 + ViewWorld[2][2] * 敌人坐标.Z坐标 + ViewWorld[2][3];
		缩放比例 = 1 / 相机X;
		if (相机X < 0)
		{
			continue;
		}

		// 计算竖矩阵敌人相机位置
		相机X = 视角宽度 + (ViewWorld[0][0] * 敌人坐标.X坐标 + ViewWorld[0][1] * 敌人坐标.Y坐标 + ViewWorld[0][2] * 敌人坐标.Z坐标 + ViewWorld[0][3]) * 缩放比例 * 视角宽度;
		相机Y = 视角高度 - (ViewWorld[1][0] * 敌人坐标.X坐标 + ViewWorld[1][1] * 敌人坐标.Y坐标 + ViewWorld[1][2] * (敌人坐标.Z坐标 - 8) + ViewWorld[1][3]) * 缩放比例 * 视角高度;
		相机Y2 = 视角高度 - (ViewWorld[1][0] * 敌人坐标.X坐标 + ViewWorld[1][1] * 敌人坐标.Y坐标 + ViewWorld[1][2] * (敌人坐标.Z坐标 + 78) + ViewWorld[1][3]) * 缩放比例 * 视角高度;

		// 得到方框参数
		方框高度 = 相机Y - 相机Y2;
		方框宽度 = 方框高度 * 0.526515151552;

		敌人准星距离 = 取准星距离(视角宽度, 视角高度, 相机X, 相机Y);
		三维人敌距离 = 取整(取敌人距离(本人坐标, 敌人坐标) / 10);

		printf("屏幕坐标X = %f | 屏幕坐标Y = %f | 方框宽度 = %f | 方框高度 = %f | 敌人血量: %d \n", 相机X - 方框宽度 / 2, 相机Y2, 方框宽度, 方框高度, 敌人血量);
	}
}

int main(int argc, char *argv[])
{
	初始化部分();

	while (1)
	{
		计算透视方框();
		Sleep(1000);
	}
	return 0;
}