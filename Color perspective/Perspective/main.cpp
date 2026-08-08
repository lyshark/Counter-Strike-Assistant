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