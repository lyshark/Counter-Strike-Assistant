读入游戏中的竖矩阵数据

```CPP
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
```

将一个竖矩阵转换为横矩阵

```CPP
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
```

矩阵透视的核心代码

```CPP
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
```

增加D3D绘制及自动瞄准

```CPP
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <math.h>
#include <d3dx9.h>
#include <tchar.h>

#pragma comment(lib,"d3d9.lib") 
#pragma comment(lib,"d3dx9.lib")

#pragma pack(push)
#pragma pack(1)
#ifndef _WIN64
struct JmpCode
{
private:
  const BYTE jmp;
  DWORD address;

public:
  JmpCode(DWORD srcAddr, DWORD dstAddr) : jmp(0xE9)
  {
    setAddress(srcAddr, dstAddr);
  }

  void setAddress(DWORD srcAddr, DWORD dstAddr)
  {
    address = dstAddr - srcAddr - sizeof(JmpCode);
  }
};
#else
struct JmpCode
{
private:
  BYTE jmp[6];
  uintptr_t address;

public:
  JmpCode(uintptr_t srcAddr, uintptr_t dstAddr)
  {
    static const BYTE JMP[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    memcpy(jmp, JMP, sizeof(jmp));
    setAddress(srcAddr, dstAddr);
  }

  void setAddress(uintptr_t srcAddr, uintptr_t dstAddr)
  {
    address = dstAddr;
  }
};
#endif
#pragma pack(pop)

void* endSceneAddr = NULL;
BYTE endSceneOldCode[sizeof(JmpCode)];

ID3DXFont* g_font = NULL;
ID3DXLine* d3dLine = NULL;

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

// ------------------------------------------------------------------------------------------------
// 通用调用方法
// ------------------------------------------------------------------------------------------------

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

// 开始Hook
int hook(void* originalFunction, void* hookFunction, BYTE* oldCode)
{
  JmpCode code((uintptr_t)originalFunction, (uintptr_t)hookFunction);
  DWORD oldProtect, oldProtect2;

  // 设置内存保护方式为可读写
  if (VirtualProtect(originalFunction, sizeof(code), PAGE_EXECUTE_READWRITE, &oldProtect))
  {
    memcpy(oldCode, originalFunction, sizeof(code));
    memcpy(originalFunction, &code, sizeof(code));

    // 恢复内存保护方式
    if (VirtualProtect(originalFunction, sizeof(code), oldProtect, &oldProtect2))
    {
      return 1;
    }
  }
  return 0;
}

// 取消Hook
int unhook(void* originalFunction, BYTE* oldCode)
{
  DWORD oldProtect, oldProtect2;

  // 设置保护方式为可读写
  if (VirtualProtect(originalFunction, sizeof(JmpCode), PAGE_EXECUTE_READWRITE, &oldProtect))
  {
    memcpy(originalFunction, oldCode, sizeof(JmpCode));

    // 恢复内存保护方式
    if (VirtualProtect(originalFunction, sizeof(JmpCode), oldProtect, &oldProtect2))
    {
      return 1;
    }
  }
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

FLOAT 求反正切(FLOAT x)
{
  return atan(x);
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

// 写类型: 整数型 短整数型 浮点型 字节型
template<typename T1>
void WriteMemory(HANDLE processHandle, DWORD pAddr, T1 vaule)
{
  WriteProcessMemory(processHandle, (LPVOID)(pAddr), &vaule, sizeof(T1), NULL);
}

// ------------------------------------------------------------------------------------------------
// 内部绘制方法
// ------------------------------------------------------------------------------------------------

// 绘制线条
void DrawLine(float x, float y, float x2, float y2, float width, D3DCOLOR color)
{
  D3DXVECTOR2 dLine[2];

  d3dLine->SetWidth(width);

  dLine[0].x = x;
  dLine[0].y = y;

  dLine[1].x = x2;
  dLine[1].y = y2;

  d3dLine->Draw(dLine, 2, color);
}

// 绘制文字
void DrawString(const char* fmt, long x, long y, D3DCOLOR color)
{
  RECT FontPos;
  FontPos.left = x;
  FontPos.top = y;

  CHAR buf[124] = { '\0' };
  va_list va_alist;
  va_start(va_alist, fmt);
  vsprintf_s(buf, fmt, va_alist);
  va_end(va_alist);

  g_font->DrawTextA(0, buf, -1, &FontPos, DT_NOCLIP, color);
}

// 绘制方框
void DrawBox(float x, float y, float width, float height, float w, D3DCOLOR color)
{
  D3DXVECTOR2 points[5];
  points[0] = D3DXVECTOR2(x, y);
  points[1] = D3DXVECTOR2(x + width, y);
  points[2] = D3DXVECTOR2(x + width, y + height);
  points[3] = D3DXVECTOR2(x, y + height);
  points[4] = D3DXVECTOR2(x, y);
  d3dLine->SetWidth(w);
  d3dLine->Draw(points, 5, color);
}

// ------------------------------------------------------------------------------------------------
// 透视主函数
// ------------------------------------------------------------------------------------------------

// 初始化基址
VOID 初始化游戏数据()
{
  // ProcessPID = GetProcessIdByName(L"hl2.exe");
  ProcessPID = GetCurrentProcessId();
  ProcessHandle = (DWORD)GetProcessHandle(ProcessPID);

  Engine模块句柄 = GetProcessModuleHandle(ProcessPID, L"engine.dll");
  Server模块句柄 = GetProcessModuleHandle(ProcessPID, L"server.dll");
  Client模块句柄 = GetProcessModuleHandle(ProcessPID, L"client.dll");

  本人基地址 = ReadMemory<DWORD>((HANDLE)ProcessHandle, Server模块句柄 + 十六转十进制("0x4F2FEC"));
  矩阵基地址 = Engine模块句柄 + 十六转十进制("0x596EF0");

  // printf("模块句柄: %x --> 矩阵基址: 0x%x --> 本人基址: 0x%x \n", Engine模块句柄, 矩阵基地址, 本人基地址);
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

// 将弧度转角度
D2D坐标 弧度转角度(D3D坐标 个人坐标, D3D坐标 对象坐标)
{
  FLOAT 自瞄X = 0;
  FLOAT 自瞄Y = 0;
  FLOAT 自瞄Z = 0;

  D2D坐标 鼠标 = { 0 };

  自瞄X = 个人坐标.X坐标 - 对象坐标.X坐标;
  自瞄Y = 个人坐标.Y坐标 - 对象坐标.Y坐标;
  自瞄Z = 个人坐标.Z坐标 - 对象坐标.Z坐标 + 10;

  if (自瞄X >= 0 && 自瞄Y >= 0)
  {
    鼠标.X轴 = 求反正切(自瞄Y / 自瞄X) / 3.14159 * 180 - 180;
  }

  if (自瞄X <= 0 && 自瞄Y >= 0)
  {
    鼠标.X轴 = 求反正切(自瞄Y / 自瞄X) / 3.14159 * 180;
  }

  if (自瞄X <= 0 && 自瞄Y <= 0)
  {
    鼠标.X轴 = 求反正切(自瞄Y / 自瞄X) / 3.14159 * 180;
  }

  if (自瞄X >= 0 && 自瞄Y <= 0)
  {
    鼠标.X轴 = 求反正切(自瞄Y / 自瞄X) / 3.14159 * 180 + 180;
  }

  鼠标.Y轴 = 求反正切(自瞄Z / 求平方根(自瞄X * 自瞄X + 自瞄Y * 自瞄Y)) / 3.1415926 * 180;
  return 鼠标;
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

  // 获取本人坐标数据
  本人基地址 = ReadMemory<DWORD>((HANDLE)ProcessHandle, Server模块句柄 + 十六转十进制("0x4F2FEC"));
  本人阵营 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 本人基地址 + 500);

  本人坐标.X坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 648 - 8);
  本人坐标.Y坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 648 - 4);
  本人坐标.Z坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 本人基地址 + 648);

  // 循环玩家数量
  玩家个数 = ReadMemory<DWORD>((HANDLE)ProcessHandle, Server模块句柄 + 十六转十进制("0x588878"));

  for (int x = 0; x <= 玩家个数 + 1; 人物循环 = x, x++)
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

    // printf("屏幕坐标X = %f | 屏幕坐标Y = %f | 方框宽度 = %f | 方框高度 = %f | 敌人血量: %d \n", 相机X - 方框宽度 / 2, 相机Y2, 方框宽度, 方框高度, 敌人血量);

    // ----------------------------------------------------------------------------------
    // 判断自瞄对象
    // ----------------------------------------------------------------------------------
    自瞄最大距离 = 45;

    // 判断是否按下ctrl
    if (GetAsyncKeyState(VK_CONTROL) == 0)
    {
      // 如果按下自瞄，那么将本敌人基地址，放到敌人对象中
      if (自瞄最大距离 > 敌人准星距离)
      {
        自瞄最大距离 = 敌人准星距离;
        敌人对象 = 敌人基地址;
      }
    }
    else
    {
      // 如果自瞄血量小于1说明死了，那么再取出一个敌人的地址
      自瞄血量 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 敌人对象 + 228);

      // 当敌人死后，自动切换到下一个敌人身上
      if (自瞄血量 <= 1 || 自瞄血量 == 0)
      {
        if (自瞄最大距离 > 敌人准星距离)
        {
          自瞄最大距离 = 敌人准星距离;
          敌人对象 = 敌人基地址;
        }
      }
    }

    // 动态绘制锁定方框
    if (敌人基地址 == 敌人对象 && GetAsyncKeyState(VK_CONTROL) != 0)
    {
      // 绘制红色方框
      DrawBox(相机X - 方框宽度 / 2, 相机Y2, 方框宽度, 方框高度, 2, D3DCOLOR_RGBA(255, 0, 0, 255));
    }
    else
    {
      // 绘制绿色方框
      DrawBox(相机X - 方框宽度 / 2, 相机Y2, 方框宽度, 方框高度, 1, D3DCOLOR_RGBA(0, 160, 32, 240));
    }

    // ----------------------------------------------------------------------------------
    // 开始执行自瞄算法
    // ----------------------------------------------------------------------------------

    // 首先取出要自瞄的敌人的三维坐标
    自瞄坐标.X坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人对象 + 648 - 8);
    自瞄坐标.Y坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人对象 + 648 - 4);
    自瞄坐标.Z坐标 = ReadMemory<FLOAT>((HANDLE)ProcessHandle, 敌人对象 + 648);

    if (自瞄坐标.X坐标 != -1 && 自瞄坐标.Y坐标 != -1 && 自瞄坐标.Z坐标 != -1)
    {
      // 通过自瞄算法，将弧度转化为角度
      自瞄鼠标 = 弧度转角度(本人坐标, 自瞄坐标);
      自瞄血量 = ReadMemory<DWORD>((HANDLE)ProcessHandle, 敌人对象 + 228);

      if (自瞄血量 > 1)
      {
        if (GetAsyncKeyState(VK_CONTROL) != 0)
        {
          // 如果需要自瞄，将鼠标坐标调整到需要自瞄的位置
          WriteMemory<FLOAT>((HANDLE)ProcessHandle, Engine模块句柄 + 十六转十进制("0x4622CC"), 自瞄鼠标.Y轴);
          WriteMemory<FLOAT>((HANDLE)ProcessHandle, Engine模块句柄 + 十六转十进制("0x4622CC") + 4, 自瞄鼠标.X轴);
        }
      }
    }

    /*
    // 在方框上绘制文字
    方框坐标数据.左边 = (DWORD)(相机X - 方框宽度 / 2);
    方框坐标数据.顶边 = (DWORD)相机Y2;

    char szString[256] = { 0 };
    sprintf(szString, "距离: %d | 生命: %d", (DWORD)三维人敌距离, (DWORD)敌人血量);
    DrawString(szString, 取整(方框坐标数据.左边), 取整(方框坐标数据.顶边) - 20, D3DCOLOR_RGBA(255, 0, 0, 255));

    // 绘制射线
    DrawLine(视角宽度, 视角高度 * 2, 相机X, 相机Y, 2, D3DCOLOR_RGBA(0, 160, 32, 100));
    DrawLine(视角宽度, 0, 相机X, 相机Y, 2, D3DCOLOR_RGBA(0, 160, 32, 100));
    */
  }
}

// ------------------------------------------------------------------------------------------------
// D3DHook代码
// ------------------------------------------------------------------------------------------------

// 该函数是劫持后的转向函数,这里面可以增加功能
HRESULT STDMETHODCALLTYPE MyEndScene(IDirect3DDevice9* thiz)
{
  // 如果是第一次则初始化绘图库
  if (g_font == NULL)
  {
    // 初始化字体
    D3DXCreateFontA(thiz, 12, 0, FW_HEAVY, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "宋体", &g_font);
    // 线条初始化线条
    D3DXCreateLine(thiz, &d3dLine);

    unhook(endSceneAddr, endSceneOldCode);
    HRESULT hr = thiz->EndScene();
    hook(endSceneAddr, MyEndScene, endSceneOldCode);
    return hr;
  }

  // 不是第一次则直接绘图
  else
  {
    // 绘制菜单
    计算透视方框();

    // 恢复钩子
    unhook(endSceneAddr, endSceneOldCode);

    // 执行原函数
    HRESULT hr = thiz->EndScene();

    // 挂钩
    hook(endSceneAddr, MyEndScene, endSceneOldCode);
    return hr;
  }
}

// 获取虚函数地址
uintptr_t GetVtableFunAddr(void* pObj, int index)
{
  uintptr_t* pAddr = NULL;
  pAddr = reinterpret_cast<uintptr_t*>(pObj);

  // 获取虚函数表指针
  pAddr = (uintptr_t*)*pAddr;
  return pAddr[index];
}

// 初始化Hook线程
DWORD WINAPI initHookThread(LPVOID dllMainThread)
{
  WaitForSingleObject(dllMainThread, INFINITE);
  CloseHandle(dllMainThread);

  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(wc);
  wc.style = CS_OWNDC;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpfnWndProc = DefWindowProc;
  wc.lpszClassName = _T("LySharkWindow");

  // 注册窗口类
  if (RegisterClassEx(&wc) == 0)
  {
    return 0;
  }

  // 创建窗口
  HWND hwnd = CreateWindowEx(0, wc.lpszClassName, _T(""), WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, NULL, NULL, wc.hInstance, NULL);
  if (hwnd == NULL)
  {
    return 0;
  }

  // 初始化D3D
  IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
  if (d3d9 == NULL)
  {
    DestroyWindow(hwnd);
    return 0;
  }

  D3DPRESENT_PARAMETERS pp = {};
  pp.Windowed = TRUE;
  pp.SwapEffect = D3DSWAPEFFECT_COPY;

  // 创建设备
  IDirect3DDevice9* device;
  if (FAILED(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device)))
  {
    d3d9->Release();
    DestroyWindow(hwnd);
    return 0;
  }

  // 开始劫持 EndScene
  // EndScene是IDirect3DDevice9第43个函数
  // endSceneAddr = (*(void***)device)[42];
  endSceneAddr = (VOID *)GetVtableFunAddr(device, 42);

  hook(endSceneAddr, MyEndScene, endSceneOldCode);

  // 释放
  d3d9->Release();
  device->Release();
  DestroyWindow(hwnd);
  return 0;
}

// dll入口
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
  {
    // 初始化数据
    初始化游戏数据();

    HANDLE curThread;

    // 获取当前线程ID
    if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &curThread, SYNCHRONIZE, FALSE, 0))
    {
      return FALSE;
    }

    // DllMain中不能使用COM组件 所以要在另一个线程初始化
    CloseHandle(CreateThread(NULL, 0, initHookThread, curThread, 0, NULL));
    break;
  }
  case DLL_PROCESS_DETACH:
    if (endSceneAddr != NULL)
    {
      unhook(endSceneAddr, endSceneOldCode);
    }
    break;
  }
  return TRUE;
}
```