#pragma once
#include <d3d11.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// DX11 globals
extern ID3D11Device* g_pDevice;
extern ID3D11DeviceContext* g_pContext;
extern ID3D11RenderTargetView* g_pRenderTargetView;
extern IDXGISwapChain* g_pSwapChain;
extern HWND g_GameWindow;
extern bool g_ImGuiInitialized;
extern WNDPROC g_OriginalWndProc;

// Hook types
typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall* ResizeBuffers_t)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

extern Present_t g_OriginalPresent;
extern ResizeBuffers_t g_OriginalResizeBuffers;
extern void* g_PresentAddr;

// Functions
void* GetPresentAddress();
bool InstallDX11Hooks();
void CleanupDX11();
void InitImGui(IDXGISwapChain* swapChain);
LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT __stdcall HookedPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
HRESULT __stdcall HookedResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT format, UINT flags);
