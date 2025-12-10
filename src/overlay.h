#pragma once
#include <windows.h>
#include <d3d11.h>

// DirectX 11 device pointers
extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_mainRenderTargetView;

// Initialize DirectX 11 device and swap chain
bool CreateDeviceD3D(HWND hWnd);

// Clean up DirectX resources
void CleanupDeviceD3D();

// Initialize ImGui with DirectX 11 and Win32
bool InitializeImGui(HWND hwnd);

// Shutdown ImGui
void ShutdownImGui();

// Begin new ImGui frame
void BeginImGuiFrame();

// End ImGui frame and present to screen
void EndImGuiFrame();
