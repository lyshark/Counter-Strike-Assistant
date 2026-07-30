得到函数相对偏移地址，可以注入到游戏内通过DbgView工具得到相对偏移

```CPP
#include <Windows.h>
#include <iostream>
#include <d3dx9.h>

#pragma comment(lib,"d3d9.lib") 
#pragma comment(lib,"d3dx9.lib")

void* DrawIndexedPrimitiveAddress = NULL;

// 获取指定COM组件内存地址
DWORD WINAPI GetComAddress(LPVOID dllMainThread)
{
  WaitForSingleObject(dllMainThread, INFINITE);
  CloseHandle(dllMainThread);

  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(wc);
  wc.style = CS_OWNDC;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpfnWndProc = DefWindowProc;
  wc.lpszClassName = L"LySharkWindow";

  // 注册窗口类
  if (RegisterClassEx(&wc) == 0)
  {
    return 0;
  }

  // 创建窗口
  HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, NULL, NULL, wc.hInstance, NULL);
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

  // DrawIndexedPrimitive 是IDirect3DDevice9第83个函数
  DrawIndexedPrimitiveAddress = (*(void***)device)[82];

  HANDLE handle = GetModuleHandle(TEXT("d3d9.dll"));

  char str[512] = { 0 };

  sprintf(str, "d3d9.dll => 0x%x | GetAddress = > 0x%x | offset = 0x%x \n", handle, DrawIndexedPrimitiveAddress, ((DWORD)DrawIndexedPrimitiveAddress - (DWORD)handle));
  OutputDebugStringA(str);

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
    HANDLE curThread;

    // 获取当前线程ID
    if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &curThread, SYNCHRONIZE, FALSE, 0))
    {
      return FALSE;
    }

    // DllMain中不能使用COM组件 所以要在另一个线程初始化
    CloseHandle(CreateThread(NULL, 0, GetComAddress, curThread, 0, NULL));

    break;
  }

  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}
```

有了相对偏移，接着寻找特定模型ID号，注入到游戏内，通过按键控制即可寻找上色模型序号

```CPP
#include <windows.h>
#include <iostream>
#include <d3d9.h>

#pragma comment(lib, "d3d9.lib")
DWORD jump = 0;

// 虚拟按键代码片段
WNDPROC Global_OldProc = NULL;
HWND Window;

// 人物模型
DWORD iStrideValue = 0;
DWORD NumVerticesValue = 0;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  DWORD dwCurProcessId = *((DWORD*)lParam);
  DWORD dwProcessId = 0;

  GetWindowThreadProcessId(hwnd, &dwProcessId);
  if (dwProcessId == dwCurProcessId && GetParent(hwnd) == NULL)
  {
    *((HWND *)lParam) = hwnd;
    return FALSE;
  }
  return TRUE;
}

HWND GetMainWindow()
{
  DWORD dwCurrentProcessId = GetCurrentProcessId();
  if (!EnumWindows(EnumWindowsProc, (LPARAM)&dwCurrentProcessId))
  {
    return (HWND)dwCurrentProcessId;
  }
  return NULL;
}

// 中转函数,执行被我们填充后的指令片段，并跳转到原始指令的后面继续执行
__declspec(naked) HRESULT __stdcall Transfer_DrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
  __asm{
    mov edi, edi
      push ebp
      mov ebp, esp
      mov eax, jump
      jmp eax
  }
}

// 在此函数中DIY你的项目，这个就是我们的中转函数，用于绘制透视方框等
HRESULT __stdcall MyDrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
  HRESULT Result = S_FALSE;
  IDirect3DVertexBuffer9 *pStreamData = NULL;
  UINT iOffsetInBytes, iStride;

  if (m_pDevice->GetStreamSource(0, &pStreamData, &iOffsetInBytes, &iStride) == D3D_OK)
  {
    pStreamData->Release();  // 得到模型来源
  }

  // 相等则找下一组模型
  if (iStride == iStrideValue)
  {
    // 找子模型号
    if (NumVertices == NumVerticesValue)
    {
      // 禁用Z轴
      //m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

      // 人物线条透视
      m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    }
    else
    {
      // 恢复Z轴
      //m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

      // 恢复默认线条
      // m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
      m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }
  }
  else
  {
    // 恢复Z轴
    //m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

    // 恢复默认线条
    //m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  }

  Result = Transfer_DrawIndexedPrimitive(m_pDevice, type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
  return Result;
}

// 通过硬编码方式获取到GetDrawIndexedPrimitive函数的基地址
ULONG_PTR GetDrawIndexedPrimitiveAddr()
{
  HANDLE handle = GetModuleHandle(TEXT("d3d9.dll"));      // 获得d3d9.dll模块基址
  if (handle == INVALID_HANDLE_VALUE) return NULL;
  return(ULONG_PTR)handle + 0x627B0;                      // 相加偏移
}

// 开始Hook
bool HookDrawIndexedPrimitive()
{
  ULONG_PTR address = GetDrawIndexedPrimitiveAddr();
  DWORD oldProtect = 0;
  if (VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READWRITE, &oldProtect))   // 设置内存保护方式为可读写
  {
    DWORD value = (DWORD)MyDrawIndexedPrimitive - address - 5;                 // 计算出需要跳转字节
    jump = address + 5;                                                        // 计算下一个跳转字节
    __asm
    {
      mov eax, address
        mov byte ptr[eax], 0xe9                                                 // 填充为 jmp
        add eax, 1                                                              // 指针递增
        mov ebx, value                                                          // 中转
        mov dword ptr[eax], ebx                                                 // 赋值跳转地址(远跳转)
    }
    VirtualProtect((LPVOID)address, 5, oldProtect, &oldProtect);               // 恢复内存保护方式
  }
  return true;
}

// 热键回调函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

  switch (uMsg)
  {
  case WM_KEYDOWN:
  {
    switch (wParam)
    {


    case VK_UP:
    {
      char str[512] = { 0 };

      iStrideValue = iStrideValue + 1;

      sprintf(str, "iStrideValue [上加 控制]= > %d \n", iStrideValue);
      OutputDebugStringA(str);

      break;
    }
    case VK_DOWN:
    {
      char str[512] = { 0 };

      iStrideValue = iStrideValue - 1;

      sprintf(str, "iStrideValue [下减 控制]= > %d \n", iStrideValue);
      OutputDebugStringA(str);
      break;
    }
    case VK_RIGHT:
    {
      char str[512] = { 0 };

      NumVerticesValue = NumVerticesValue + 1;

      sprintf(str, "NumVerticesValue [左加 控制]= > %d \n", NumVerticesValue);
      OutputDebugStringA(str);
      break;
    }
    case VK_LEFT:
    {
      char str[512] = { 0 };
      NumVerticesValue = NumVerticesValue - 1;

      sprintf(str, "NumVerticesValue [右减 控制]= > %d \n", NumVerticesValue);
      OutputDebugStringA(str);
      break;
    }
    break;
    }
  }
  break;
  }

  // 全局热键回调函数
  return CallWindowProc(Global_OldProc, hwnd, uMsg, wParam, lParam);
}

// 主函数判断如果是游戏标题则进行注入
bool APIENTRY DllMain(HANDLE handle, DWORD ul_reason_for_call, LPVOID lpvoid)
{
  switch (ul_reason_for_call)
  {

  case DLL_PROCESS_ATTACH:
  {
    // 获取自身进程窗口句柄
    Window = GetMainWindow();

    // 注册全局热键
    Global_OldProc = (WNDPROC)SetWindowLong(Window, GWL_WNDPROC, (LONG)WindowProc);
    HookDrawIndexedPrimitive();
    break;
  }

  case DLL_PROCESS_DETACH:
    break;


  default:
    break;
  }

  return TRUE;
}
```

当找到指定的模型ID后，就编写透视代码

```CPP
#include <windows.h>
#include <iostream>
#include <d3d9.h>

#pragma comment(lib, "d3d9.lib")
DWORD jump = 0;

IDirect3DPixelShader9* Front = NULL;
IDirect3DTexture9* Color;

// 中转函数,执行被我们填充后的指令片段，并跳转到原始指令的后面继续执行
__declspec(naked) HRESULT __stdcall Transfer_DrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
  __asm{
    mov edi, edi
      push ebp
      mov ebp, esp
      mov eax, jump
      jmp eax
  }
}

// 在此函数中DIY你的项目，这个就是我们的中转函数，用于绘制透视方框等
HRESULT __stdcall MyDrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
  HRESULT Result = S_FALSE;
  IDirect3DVertexBuffer9 *pStreamData = NULL;
  UINT iOffsetInBytes, iStride;

  if (m_pDevice->GetStreamSource(0, &pStreamData, &iOffsetInBytes, &iStride) == D3D_OK)
  {
    pStreamData->Release();  // 得到模型来源
  }

  // 相等则找下一组模型
  if (iStride == 32)
  {
    // 找子模型号
    if (NumVertices == 323 || NumVertices == 368 || NumVertices == 408 || NumVertices == 389)
    {
      m_pDevice->SetPixelShader(Front);                        // 清空着色器
      m_pDevice->SetTexture(0, Color);                         // 上色
      m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);   // 禁用z轴缓冲
      m_pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
    }
    else
    {
      // 恢复Z轴
      m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    }
  }
  else
  {
    // 恢复Z轴
    m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
  }

  Result = Transfer_DrawIndexedPrimitive(m_pDevice, type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

  m_pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL); 
  return Result;
}

// 通过硬编码方式获取到GetDrawIndexedPrimitive函数的基地址
ULONG_PTR GetDrawIndexedPrimitiveAddr()
{
  HANDLE handle = GetModuleHandle(TEXT("d3d9.dll"));      // 获得d3d9.dll模块基址
  if (handle == INVALID_HANDLE_VALUE) return NULL;
  return(ULONG_PTR)handle + 0x627B0;                      // 相加偏移
}

// 开始Hook
bool HookDrawIndexedPrimitive()
{
  ULONG_PTR address = GetDrawIndexedPrimitiveAddr();
  DWORD oldProtect = 0;
  if (VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READWRITE, &oldProtect))   // 设置内存保护方式为可读写
  {
    DWORD value = (DWORD)MyDrawIndexedPrimitive - address - 5;                 // 计算出需要跳转字节
    jump = address + 5;                                                        // 计算下一个跳转字节
    __asm
    {
      mov eax, address
        mov byte ptr[eax], 0xe9                                                 // 填充为 jmp
        add eax, 1                                                              // 指针递增
        mov ebx, value                                                          // 中转
        mov dword ptr[eax], ebx                                                 // 赋值跳转地址(远跳转)
    }
    VirtualProtect((LPVOID)address, 5, oldProtect, &oldProtect);               // 恢复内存保护方式
  }
  return true;
}

// 主函数判断如果是游戏标题则进行注入
bool APIENTRY DllMain(HANDLE handle, DWORD ul_reason_for_call, LPVOID lpvoid)
{
  switch (ul_reason_for_call)
  {

  case DLL_PROCESS_ATTACH:
  {
    HookDrawIndexedPrimitive();
    break;
  }

  case DLL_PROCESS_DETACH:
    break;

  default:
    break;
  }

  return TRUE;
}
```

在上色基础上开启线条呈现

```CPP
#include <windows.h>
#include <iostream>
#include <d3d9.h>

#pragma comment(lib, "d3d9.lib")

int* d3d_Dw5byte = new int;// DrawIndexedPrimitive入口的前5个字节

IDirect3DPixelShader9* Front = NULL;
IDirect3DTexture9* Color_TexRed; // 红色纹理
ULONG_PTR g_DwJmpTo = NULL; // DrawIndexedPrimitive调用地址
ULONG_PTR g_DwAdder = NULL; // DrawIndexedPrimitive被HOOK后地址

// 函数声明
HRESULT WINAPI hk_DrawIndexedPrimitive(LPDIRECT3DDEVICE9 Device_Interface, D3DPRIMITIVETYPE Type, INT BaseIndex, UINT MinIndex, UINT NumVertices, UINT AAAAAAAA, UINT PrimitiveCount);
HRESULT WINAPI Original_DrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);

// 获取DrawIndexedPrimitive地址
ULONG_PTR GetDrawIndexedPrimitiveAdder()
{
  HMODULE m_hModule = GetModuleHandle(L"d3d9.dll");
  return (ULONG_PTR)m_hModule + 0x627B0;
}

// 初始化HOOK
void OnHookInit()
{
  g_DwAdder = GetDrawIndexedPrimitiveAdder();
  g_DwJmpTo = g_DwAdder + 5;
  DWORD oldpro = 0;

  memcpy(d3d_Dw5byte, (VOID*)g_DwAdder, 5);//保存DrawIndexedPrimitive入口的前5个字节
  VirtualProtect((LPVOID)g_DwAdder, 5, PAGE_EXECUTE_READWRITE, &oldpro);//修改内存保护属性
  *(BYTE*)g_DwAdder = 0xe9;//0xe9在汇编中是跳转指令操作码  
  *(DWORD*)((BYTE*)g_DwAdder + 1) = (DWORD)hk_DrawIndexedPrimitive - (DWORD)g_DwAdder - 5;//目标地址-原地址-5  
  VirtualProtect((LPVOID)g_DwAdder, 5, oldpro, &oldpro);//还原内存保护属性
}

// 创建纹理
HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32)
{
  if (FAILED(pD3Ddev->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL)))
    return E_FAIL;
  
  WORD colour16 =
    ((WORD)((colour32 >> 28) & 0xF) << 12)
    | (WORD)(((colour32 >> 20) & 0xF) << 8)
    | (WORD)(((colour32 >> 12) & 0xF) << 4)
    | (WORD)(((colour32 >> 4) & 0xF) << 0);
  
  D3DLOCKED_RECT d3dlr;
  (*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
  WORD *pDst16 = (WORD*)d3dlr.pBits;
  
  for (int xy = 0; xy < 8 * 8; xy++)
    *pDst16++ = colour16;
  
  (*ppD3Dtex)->UnlockRect(0);
  return S_OK;
};

// 调用真实的DrawIndexedPrimitive函数
__declspec(naked) HRESULT WINAPI Original_DrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
  __asm
  {
    mov edi, edi
      push ebp
      mov ebp, esp
      mov eax, g_DwJmpTo
      jmp eax
  }
}

// DrawIndexedPrimitive被HOOK的函数
HRESULT WINAPI hk_DrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
  IDirect3DVertexBuffer9* pStreamData = NULL;
  UINT iOffsetInBytes;
  UINT iStride;
  if (pDevice->GetStreamSource(0, &pStreamData, &iOffsetInBytes, &iStride) == D3D_OK)
  {
    pStreamData->Release();
  }

  // 找主模型
  if (iStride == 32)
  {
    if (NumVertices == 323 || NumVertices == 368 || NumVertices == 408 || NumVertices == 389)
    {
      // 设置红色纹理
      //GenerateTexture(pDevice, &Color_TexRed, D3DCOLOR_ARGB(255, 127, 255, 0));

      pDevice->SetPixelShader(Front);                     // 清空着色器
      // pDevice->SetTexture(0, Color_TexRed);               // 上色
      pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);// 禁用z轴缓冲

      pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
      Original_DrawIndexedPrimitive(pDevice, type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
      pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

      // 线填充模式 D3D在多边形的每个边绘制一条线
      pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    }
    else
    {
      // 恢复禁用轴缓冲
      pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

      // 恢复默认线条
      pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }
  }

  return Original_DrawIndexedPrimitive(pDevice, type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

// 主函数判断如果是游戏标题则进行注入
bool APIENTRY DllMain(HANDLE handle, DWORD ul_reason_for_call, LPVOID lpvoid)
{
  switch (ul_reason_for_call)
  {

  case DLL_PROCESS_ATTACH:
  {
    OnHookInit();
    break;
  }

  case DLL_PROCESS_DETACH:
    break;

  default:
    break;
  }

  return TRUE;
}
```