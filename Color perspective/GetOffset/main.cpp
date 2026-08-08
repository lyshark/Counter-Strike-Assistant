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