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