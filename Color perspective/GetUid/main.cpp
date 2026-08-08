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