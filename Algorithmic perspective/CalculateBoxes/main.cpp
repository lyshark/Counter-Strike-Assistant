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

typedef struct
{
	float X坐标;
	float Y坐标;
	float Z坐标;
}三维坐标型;

typedef struct
{
	float X轴;
	float Y轴;
}鼠标角度型;

DWORD ProcessPID = 0;
DWORD ProcessHandle = 0;
DWORD 辅助窗体句柄 = 0;

DWORD server模块句柄 = 0;
DWORD client模块句柄 = 0;
DWORD engine模块句柄 = 0;

DWORD 游戏窗口宽度 = 1024;
DWORD 游戏窗口高度 = 768;

#define PI 3.1415926535;

// ------------------------------------------------------------
// 调用方法
// ------------------------------------------------------------

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

DOUBLE 求平方根(DOUBLE x)
{
	return sqrt(x);
}

DOUBLE 求次方(DOUBLE x, DOUBLE y)
{
	return pow(x, y);
}

DOUBLE 求正弦(DOUBLE x)
{
	return sin(x);
}

DOUBLE 求余弦(DOUBLE x)
{
	return cos(x);
}

DOUBLE 求正切(DOUBLE x)
{
	return tan(x);
}

DOUBLE 求反正切(DOUBLE x)
{
	return atan(x);
}

DWORD 取绝对值(DOUBLE x)
{
	return abs(x);
}

DWORD 取整(DOUBLE x)
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

// ------------------------------------------------------------
// 透视主函数
// ------------------------------------------------------------

// 初始化结构体
VOID 初始化游戏数据()
{
	ProcessPID = GetProcessIdByName(L"hl2.exe");
	ProcessHandle = (DWORD)GetProcessHandle(ProcessPID);
	server模块句柄 = GetProcessModuleHandle(ProcessPID, L"server.dll");
	client模块句柄 = GetProcessModuleHandle(ProcessPID, L"client.dll");
	engine模块句柄 = GetProcessModuleHandle(ProcessPID, L"engine.dll");
}

// 得到玩家数量
DWORD 取玩家数量()
{
	DWORD 玩家数量 = ReadMemory<DWORD>((HANDLE)ProcessHandle, engine模块句柄 + 0x5D29BC);
	return 玩家数量 - 1;
}

// 判断玩家是否死亡
BOOL 玩家是否死亡(DWORD 个数)
{
	DWORD 敌人指针;
	DWORD 敌人血量;

	个数 = (个数 - 1) * 16;

	敌人指针 = ReadMemory<DWORD>((HANDLE)ProcessHandle, server模块句柄 + 个数 + 十六转十进制("0x4F2FFC"));
	敌人血量 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 敌人指针 + 十六转十进制("0xE4"));

	if (敌人血量 <= 0 || 敌人血量 == 1)
	{
		return TRUE;
	}

	return FALSE;
}

// 取自己坐标数据
三维坐标型 取自己坐标数据()
{
	三维坐标型 本人坐标;
	DWORD 本人基地址;

	// 本人 x = server.dll + 4F2FEC + 288 - 8 => 280
	// 本人 y = server.dll + 4F2FEC + 288 - 4 => 284
	// 本人 z = server.dll + 4F2FEC + 288 => 288
	本人基地址 = ReadMemory<DWORD>((HANDLE)ProcessHandle, server模块句柄 + 十六转十进制("0x4F2FEC"));

	本人坐标.X坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 十六转十进制("0x280"));
	本人坐标.Y坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 十六转十进制("0x284"));
	本人坐标.Z坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 十六转十进制("0x288"));

	return 本人坐标;
}

// 取敌人坐标数据
三维坐标型 取敌人坐标数据(DWORD 个数)
{
	三维坐标型 敌人坐标;
	DWORD 敌人指针;

	// 敌人x = server.dll + 4F2FFC + 288 - 8 => 280
	// 敌人y = server.dll + 4F2FFC + 288 - 4 => 284
	// 敌人z = server.dll + 4F2FFC + 288 =>288

	// 游戏中的人物之间的偏移值，此处是十进制的16等于十六进制10
	个数 = (个数 - 1) * 16;

	敌人指针 = ReadMemory<DWORD>((HANDLE)ProcessHandle, server模块句柄 + 个数 + 十六转十进制("0x4F2FFC"));

	敌人坐标.X坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人指针 + 十六转十进制("0x280"));
	敌人坐标.Y坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人指针 + 十六转十进制("0x284"));
	敌人坐标.Z坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人指针 + 十六转十进制("0x288"));

	return 敌人坐标;
}

// 取自己鼠标角度
鼠标角度型 取自己鼠标角度()
{
	鼠标角度型 鼠标角度;

	// 鼠标的 x 角度 = engine.dll+61D254 + 4 => 61D258
	// 鼠标的 y 角度  = engine.dll+61D254 => 61D254

	// 这里理解为鼠标角度为角度和x轴夹角,别理解为坐标
	鼠标角度.X轴 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, engine模块句柄 + 十六转十进制("0x61D258"));

	// 因为cs对于横坐标就是你转一圈从-180-0-180 我们想让他成0-360因此判断一下，其他的游戏不清楚是怎么算的
	if (鼠标角度.X轴 < 0)
	{
		// 如果鼠标X轴小于0则加上360即可得到一个正值
		鼠标角度.X轴 = 360 + 鼠标角度.X轴;
	}

	鼠标角度.Y轴 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, engine模块句柄 + 十六转十进制("0x61D254"));

	// 让其不会出现负数，将负数转化为正数
	鼠标角度.Y轴 = 鼠标角度.Y轴 * -1;

	return 鼠标角度;
}

// 开始计算方框位置
VOID 计算方框位置()
{
	三维坐标型 本人坐标数据 = { 0 };
	三维坐标型 敌人坐标数据 = { 0 };
	鼠标角度型 自己鼠标角度 = { 0 };

	// 本人XYZ坐标相对于敌人的差值
	DOUBLE 相对X坐标 = 0;
	DOUBLE 相对Y坐标 = 0;
	DOUBLE 相对Z坐标 = 0;

	// 求出敌人和自己的 二/三 维距离
	DOUBLE 二维人敌距离 = 0;
	DOUBLE 三维人敌距离 = 0;

	DOUBLE X轴角度 = 0;
	DOUBLE 横角度 = 0;
	DOUBLE 对边值 = 0;
	DOUBLE 对边坐标Y = 0;
	DOUBLE 邻边值 = 0;
	DOUBLE 比值关系 = 0;
	DOUBLE 屏幕坐标X = 0;
	DOUBLE 屏幕坐标Y = 0;

	DWORD 临时人物数量 = 0;
	DWORD 敌人基地址 = 0;
	DWORD 本人基地址 = 0;
	DWORD 本人阵营 = 0;
	DWORD 敌人阵营 = 0;
	DWORD 敌人血量 = 0;

	// 开始计算方框
	本人基地址 = ReadMemory<DWORD>((HANDLE)ProcessHandle, server模块句柄 + 十六转十进制("0x4F2FEC"));
	本人阵营 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 本人基地址 + 500);

	for (int x = 0; x < 取玩家数量(); 临时人物数量 = x, x++)
	{
		// x16 是得到每一个敌人的血量，临时任务数据就是遍历人物计数，-1 是去掉本身
		敌人基地址 = ReadMemory<DWORD>((HANDLE)ProcessHandle, server模块句柄 + 十六转十进制("0x4F2FFC") + (临时人物数量 - 1) * 16);
		敌人阵营 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 敌人基地址 + 500);
		敌人血量 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 敌人基地址 + 228);

		// 人物如果死亡，则直接停止绘制
		if (玩家是否死亡(临时人物数量))
		{
			continue;
		}

		// 如果本人是自己人则不再绘制
		if (本人阵营 == 敌人阵营)
		{
			continue;
		}

		// --------------------------------------------------------------------------------------------------
		// 距离测算
		// --------------------------------------------------------------------------------------------------

		本人坐标数据 = 取自己坐标数据();
		敌人坐标数据 = 取敌人坐标数据(临时人物数量);
		自己鼠标角度 = 取自己鼠标角度();

		// 通过自己的坐标减去敌人的坐标数据，就可以得到自己相对于敌人的三维距离
		相对X坐标 = 本人坐标数据.X坐标 - 敌人坐标数据.X坐标;
		相对Y坐标 = 本人坐标数据.Y坐标 - 敌人坐标数据.Y坐标;
		相对Z坐标 = 本人坐标数据.Z坐标 - 敌人坐标数据.Z坐标;

		// 敌人和自己的二维距离,忽略高度
		二维人敌距离 = 求平方根(求次方(相对X坐标, 2) + 求次方(相对Y坐标, 2));

		// 敌人和自己的三维距离,不忽略高度
		三维人敌距离 = 求平方根(求次方(相对X坐标, 2) + 求次方(相对Y坐标, 2) + 求次方(相对Z坐标, 2));

		// printf("我与敌人相差: %f 米 \n", 二维人敌距离);

		// --------------------------------------------------------------------------------------------------
		// 求自己鼠标角度与敌人之间的夹角度数
		// --------------------------------------------------------------------------------------------------

		// 敌人和自己建立的坐标系X轴重合
		if (相对X坐标 < 0 && 相对Y坐标 == 0)
		{
			// x角度是求自己和敌人连线与正x轴夹角
			X轴角度 = 0;
		}

		// 敌人在第一象限
		if (相对X坐标 < 0 && 相对Y坐标 < 0)
		{
			// 其中的 X180/PI 将弧度转换为角度的固定写法
			X轴角度 = 求反正切(取绝对值(相对Y坐标 / 相对X坐标)) * 180 / PI;
		}

		// 敌人在正Y轴第一象限
		if (相对X坐标 == 0 && 相对Y坐标 < 0)
		{
			X轴角度 = 90;
		}

		// 敌人在第二象限
		if (相对X坐标 > 0 && 相对Y坐标 < 0)
		{
			X轴角度 = 90 + 求反正切(取绝对值(相对X坐标 / 相对Y坐标)) * 180 / PI;
		}

		// 敌人在负x轴
		if (相对X坐标 > 0 && 相对Y坐标 == 0)
		{
			X轴角度 = 180;
		}

		// 敌人在第三象限
		if (相对X坐标 > 0 && 相对Y坐标 > 0)
		{
			X轴角度 = 180 + 求反正切(取绝对值(相对Y坐标 / 相对X坐标)) * 180 / PI;
		}

		// 敌人在y负半轴
		if (相对X坐标 == 0 && 相对Y坐标 > 0)
		{
			X轴角度 = 270;
		}

		// 敌人在第四象限
		if (相对X坐标 < 0 && 相对Y坐标 > 0)
		{
			X轴角度 = 270 + 求反正切(取绝对值(相对X坐标 / 相对Y坐标)) * 180 / PI;
		}

		// --------------------------------------------------------------------------------------------------
		// 横角度计算
		// --------------------------------------------------------------------------------------------------

		// 这个横角度要满足，以你鼠标冲着敌人为原点（假如左边为正，则右边要为负）
		横角度 = 自己鼠标角度.X轴 - X轴角度;

		// 求自己鼠标位置和敌人的夹角,其中你在原点鼠标指向第一象限敌人在第四象限一种特殊情况
		if (X轴角度 - 自己鼠标角度.X轴 > 180)
		{
			// 这个横角度就是特殊情况,求出来还是自己鼠标位置和敌人的角度
			横角度 = 360 - X轴角度 + 自己鼠标角度.X轴;
		}

		// 求自己鼠标位置和敌人的夹角,其中你在原点你鼠标指向第四象限，敌人在第一象限
		if (自己鼠标角度.X轴 - X轴角度 > 180)
		{
			// 这个横角度就是特殊情况,求出来还是自己鼠标位置和敌人的角度
			横角度 = (360 + X轴角度 - 自己鼠标角度.X轴) * -1;
		}
		// 输出调试文本 (“当前自己与敌人的夹角： ” ＋ 到文本 (横角度) ＋ “ 度”)

		// --------------------------------------------------------------------------------------------------
		// 游戏中的坐标转换为屏幕坐标算法
		// --------------------------------------------------------------------------------------------------
		对边值 = 求正弦(横角度 *  3.1415926535 / 180) * 二维人敌距离;
		邻边值 = 求余弦(横角度 * 3.1415926535 / 180) * 二维人敌距离;

		// 假设屏幕大小为 1024x768 则：1024/2=512(屏幕坐标X)    768/2=384(屏幕坐标Y)
		// 取整 (游戏窗口宽度 ÷ 2) => 获取的是游戏窗口宽度的一半,也就是1024/2 = 512
		// 取整 (游戏窗口高度 ÷ 2) => 获取的是游戏高度的一半，也就是768/2 = 384

		// 此处比值关系变量 => 是用来调节距离文字所在敌人的什么位置的
		比值关系 = 三维人敌距离 / 500;

		屏幕坐标X = 取整(游戏窗口宽度 / 2) - 15 / 比值关系 + 对边值 / 邻边值 * 取整(游戏窗口宽度 / 2);  // 锁定横轴
		对边坐标Y = 求正切(自己鼠标角度.Y轴 * 3.1415926535 / 180) * 二维人敌距离 + 相对Z坐标 + 10;      // 锁定纵轴，也就是上下之间锁定
		屏幕坐标Y = 取整(游戏窗口高度 / 2) + 对边坐标Y / 二维人敌距离 * 取整(游戏窗口宽度 / 2);

		if (取绝对值(横角度) < 45)
		{
			printf("屏幕坐标X = %f | 屏幕坐标Y = %f | 距离 = %f \n", 屏幕坐标X, 屏幕坐标Y, 二维人敌距离);
		}
	}
}

int main(int argc, char *argv[])
{
	初始化游戏数据();

	DWORD count = 取玩家数量();
	printf("玩家数量: %d \n", count);

	DWORD number = 玩家是否死亡(1);
	printf("第一个玩家是否死亡: %d \n", number);

	三维坐标型 xyz = 取自己坐标数据();
	printf("自己坐标 X = %f | Y = %f | Z = %f \n", xyz.X坐标, xyz.Y坐标, xyz.Z坐标);

	三维坐标型 dr_xyz = 取敌人坐标数据(1);
	printf("敌人坐标 X = %f | Y = %f | Z = %f \n", dr_xyz.X坐标, dr_xyz.Y坐标, dr_xyz.Z坐标);

	鼠标角度型 sb = 取自己鼠标角度();
	printf("鼠标X = %f | 鼠标Y = %f \n", sb.X轴, sb.Y轴);

	while (1)
	{
		计算方框位置();
		Sleep(1000);
	}

	getchar();
	return 0;
}