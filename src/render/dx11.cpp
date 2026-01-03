#include "dx11.h"
#include "menu.h"
#include "../features/settings.h"
#include "../features/esp.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <MinHook.h>
#include <cstdio>

ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pContext = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
HWND g_GameWindow = nullptr;
bool g_ImGuiInitialized = false;
WNDPROC g_OriginalWndProc = nullptr;

Present_t g_OriginalPresent = nullptr;
ResizeBuffers_t g_OriginalResizeBuffers = nullptr;
void* g_PresentAddr = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_ImGuiInitialized) {
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    }
    return CallWindowProc(g_OriginalWndProc, hWnd, msg, wParam, lParam);
}

void InitImGui(IDXGISwapChain* swapChain) {
    if (g_ImGuiInitialized) return;
    
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    g_GameWindow = desc.OutputWindow;
    
    swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice);
    if (!g_pDevice) return;
    
    g_pDevice->GetImmediateContext(&g_pContext);
    if (!g_pContext) return;
    
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (backBuffer) {
        g_pDevice->CreateRenderTargetView(backBuffer, nullptr, &g_pRenderTargetView);
        backBuffer->Release();
    }
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    
    ImGui_ImplWin32_Init(g_GameWindow);
    ImGui_ImplDX11_Init(g_pDevice, g_pContext);
    
    g_OriginalWndProc = (WNDPROC)SetWindowLongPtr(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);
    
    g_ImGuiInitialized = true;
    printf("[GFR Mod] ImGui initialized\n");
}

HRESULT __stdcall HookedPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
    if (!g_ImGuiInitialized) {
        InitImGui(swapChain);
    }
    
    static bool homeWasPressed = false;
    bool homePressed = (GetAsyncKeyState(VK_HOME) & 0x8000) != 0;
    if (homePressed && !homeWasPressed) {
        g_MenuVisible = !g_MenuVisible;
    }
    homeWasPressed = homePressed;
    
    bool altPressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    g_ESPEnabled = altPressed;
    
    if (g_MenuVisible || g_ESPEnabled) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        if (g_MenuVisible) {
            RenderMenu();
        }
        
        if (g_ESPEnabled) {
            RenderESPOverlay();
        }
        
        ImGui::Render();
        g_pContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
    
    return g_OriginalPresent(swapChain, syncInterval, flags);
}

HRESULT __stdcall HookedResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT format, UINT flags) {
    if (g_pRenderTargetView) {
        g_pRenderTargetView->Release();
        g_pRenderTargetView = nullptr;
    }
    
    HRESULT hr = g_OriginalResizeBuffers(swapChain, bufferCount, width, height, format, flags);
    
    if (SUCCEEDED(hr) && g_pDevice) {
        ID3D11Texture2D* backBuffer = nullptr;
        swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (backBuffer) {
            g_pDevice->CreateRenderTargetView(backBuffer, nullptr, &g_pRenderTargetView);
            backBuffer->Release();
        }
    }
    
    return hr;
}

void* GetPresentAddress() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "DummyDX11Window";
    RegisterClassEx(&wc);
    
    HWND dummyWindow = CreateWindowEx(0, wc.lpszClassName, "", WS_OVERLAPPEDWINDOW,
        0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
    
    if (!dummyWindow) {
        printf("[GFR Mod] Failed to create dummy window\n");
        return nullptr;
    }
    
    D3D_FEATURE_LEVEL featureLevel;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 2;
    sd.BufferDesc.Height = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = dummyWindow;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &sd,
        &swapChain, &device, &featureLevel, &context);
    
    if (FAILED(hr)) {
        printf("[GFR Mod] Failed to create dummy D3D11 device: 0x%08X\n", hr);
        DestroyWindow(dummyWindow);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return nullptr;
    }
    
    void** vtable = *(void***)swapChain;
    void* presentAddr = vtable[8];
    
    swapChain->Release();
    device->Release();
    context->Release();
    DestroyWindow(dummyWindow);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    
    return presentAddr;
}

bool InstallDX11Hooks() {
    g_PresentAddr = GetPresentAddress();
    if (!g_PresentAddr) {
        printf("[GFR Mod] Failed to get Present address\n");
        return false;
    }
    
    if (MH_CreateHook(g_PresentAddr, (void*)HookedPresent, (void**)&g_OriginalPresent) != MH_OK) {
        printf("[GFR Mod] Failed to create Present hook\n");
        return false;
    }
    
    if (MH_EnableHook(g_PresentAddr) != MH_OK) {
        printf("[GFR Mod] Failed to enable Present hook\n");
        return false;
    }
    
    printf("[GFR Mod] DX11 hooks installed\n");
    return true;
}

void CleanupDX11() {
    if (g_ImGuiInitialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_ImGuiInitialized = false;
    }
    
    if (g_OriginalWndProc && g_GameWindow) {
        SetWindowLongPtr(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)g_OriginalWndProc);
    }
    
    if (g_pRenderTargetView) {
        g_pRenderTargetView->Release();
        g_pRenderTargetView = nullptr;
    }
    if (g_pContext) {
        g_pContext->Release();
        g_pContext = nullptr;
    }
    if (g_pDevice) {
        g_pDevice->Release();
        g_pDevice = nullptr;
    }
}
