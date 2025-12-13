#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <dwmapi.h>
#include "ect/imgui/imgui.h"
#include "ect/imgui/backends/imgui_impl_win32.h"
#include "overlay.h"
#include "esp.h"
#include "spinbot.h"
#include "aimbot.h"
#include "ui_helpers.h"
#include "image_loader.h"
#include <d3d11.h>
#include <chrono>
#include "ect/json.hpp"
#include "config.h"
#include <thread>
#include <mutex>
#include <atomic>

/*
    i like to steal ascii art from the internet because i have no talent :3
     _
  |\'/-..--.
 / _ _   ,  ;
`~=`Y'~_<._./
 <`-....__.'  

*/


#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HANDLE hProcess = nullptr;
uintptr_t moduleBase = 0;
int screenWidth = 0;
int screenHeight = 0;
bool espEnabled = true;
bool teamEsp = false;

bool spinbot_enabled = false;
int spinbot_speed = 0;
bool aimbot_enabled = false;
float aimbot_fov = 90.0f;
bool aimbot_all = false;
bool fov_circle_enabled = false;
bool rapid_fire_enabled = false;
float aimbot_max_distance = 250.0f;
float fov = 115.0f;
float headoffset = 0.275f;
bool snaplines = true;
bool snaplines_all = false;



//fps
int frameCount = 0;
int fps = 0;
auto lastTime = std::chrono::high_resolution_clock::now();


ID3D11ShaderResourceView* logo_pic = nullptr;
int logo_width = 0;
int logo_height = 0;

DWORD GetProcessId(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry = { sizeof(entry) };

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (processName == entry.szExeFile) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return 0;
}

uintptr_t GetModuleBase(DWORD pid, const std::wstring& moduleName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    MODULEENTRY32W entry = { sizeof(entry) };

    if (Module32FirstW(snapshot, &entry)) {
        do {
            if (moduleName == entry.szModule) {
                CloseHandle(snapshot);
                return (uintptr_t)entry.modBaseAddr;
            }
        } while (Module32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return 0;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}


#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")


int main() {
    // Make process DPI aware to fix ImGui hitbox/input offset
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    // ImGui DPI helper (call before window creation)
    ImGui_ImplWin32_EnableDpiAwareness();


    Config::load("config.json");//load config
    fov = Config::getfov();// get fov
    fov_circle_enabled = Config::getfovcircleenabled();
    aimbot_fov = Config::getAimbotFov();
    aimbot_max_distance = Config::getAimbotmaxdist();
    snaplines = Config::getsnaplines();
    snaplines_all = Config::getsnaplinesall();


    ShowWindow(GetConsoleWindow(), SW_HIDE);


    WNDCLASSEXW wc = { 
        sizeof(wc), CS_HREDRAW | CS_VREDRAW, WndProc, 0L, 0L, 
        GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, 
        L"ImGui Overlay", nullptr 
    };
    RegisterClassExW(&wc);
    
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        wc.lpszClassName,
        L"AC Cheat",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"Failed to create window!", L"Error", MB_OK);
        return 1;
    }

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
    

    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);


    if (!CreateDeviceD3D(hwnd)) {
        MessageBoxW(nullptr, L"Failed to create DirectX device!", L"Error", MB_OK);
        CleanupDeviceD3D();
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // find game process
    DWORD pid = GetProcessId(L"ac_client.exe");
    if (!pid) {
        MessageBoxW(nullptr, L"Can't find ac_client.exe!", L"Error", MB_OK);
        CleanupDeviceD3D();
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        MessageBoxW(nullptr, L"OpenProcess failed. Run as Administrator!", L"Error", MB_OK);
        CleanupDeviceD3D();
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    moduleBase = GetModuleBase(pid, L"ac_client.exe");
    if (!moduleBase) {
        MessageBoxW(nullptr, L"Can't find module base!", L"Error", MB_OK);
        CloseHandle(hProcess);
        CleanupDeviceD3D();
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    HWND gameWindow = FindWindowW(nullptr, L"AssaultCube");

    if (!InitializeImGui(hwnd)) {
        MessageBoxW(nullptr, L"Failed to initialize ImGui!", L"Error", MB_OK);
        CleanupDeviceD3D();
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    LoadTextureFromFile("assault_cube.png", &logo_pic, &logo_width, &logo_height);

    bool running = true;
    while (running) { //main loop
        if (gameWindow && IsWindow(gameWindow)) {
            RECT rect;
            if (GetWindowRect(gameWindow, &rect)) {
                MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
            }
        }
        
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        // Process Windows messages
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }
        if (!running) break;

        static bool zKeyPrev = false;
        bool zKeyCurr = (GetAsyncKeyState('Z') & 0x8000);
        if (zKeyCurr && !zKeyPrev) {
            aimbot_enabled = !aimbot_enabled;
        }
        zKeyPrev = zKeyCurr;

        
/*
        POINT mousePoint;
        if (GetCursorPos(&mousePoint)) {
            ScreenToClient(hwnd, &mousePoint);
            ImGuiIO& io = ImGui::GetIO();
            io.MousePos = ImVec2((float)mousePoint.x, (float)mousePoint.y);
        }
*/
        BeginImGuiFrame();

        // Render ESP
        if (espEnabled) {
            RenderESP(teamEsp, hProcess, moduleBase, screenWidth, screenHeight, snaplines, snaplines_all);
        }

        // Render FOV circle
        if (fov_circle_enabled) {
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            // Convert FOV degrees to pixels
            float radius = (aimbot_fov / 90.0f) * (screenHeight / 4.0f);
            drawList->AddCircle(
                ImVec2(screenWidth / 2.0f, screenHeight / 2.0f),
                radius,
                IM_COL32(255, 255, 255, 255),
                64
            );
        }

        
        // Get local player
        uintptr_t localPlayer = 0;
        ReadProcessMemory(hProcess, (LPCVOID)(moduleBase + 0x17E0A8), &localPlayer, sizeof(localPlayer), nullptr);





        Config::setfov(fov);

        WriteProcessMemory(hProcess, (LPVOID)(moduleBase + 0x18A7CC), &fov, sizeof(fov), nullptr); //WRITE FOV
        
        

        if (aimbot_enabled && localPlayer) {
            UpdateAimbot(aimbot_all, hProcess, moduleBase, localPlayer, aimbot_enabled, aimbot_fov, aimbot_max_distance, headoffset);
        }
        
        if (spinbot_enabled && localPlayer) {
            UpdateSpinbot(spinbot_speed, hProcess, localPlayer, spinbot_enabled);
        }

        // Rapid fire - set attack delay to 0
        if (rapid_fire_enabled && localPlayer) {
            int zero = 0;
            WriteProcessMemory(hProcess, (LPVOID)(localPlayer + 0x160), &zero, sizeof(int), nullptr);
        }

        frameCount++;
        auto now = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(now - lastTime).count();
        if (elapsed >= 1.0f) {
            fps = frameCount;
            frameCount = 0;
            lastTime = now;
        }


/////////////////////////////////////////////////////////////////////////////

        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_Once);
        ImGui::Begin("NEVER-WIN", nullptr);


        //debug window
        ImGui::SetNextWindowPos(ImVec2(600, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
        ImGui::Begin("DEBUG", nullptr);

        ImGui::Text("DEBUG");
        
        ImGui::Text("OVERLAY FPS:%d", fps);




        ImGui::End(); 

    if (ImGui::BeginTabBar("CHEATS"))
    {
        if (ImGui::BeginTabItem("HOME"))
        {
            GradientPresets::Sexy("MADE BY linktr.ee/sigmacat123");

            
            ImGui::Spacing();

            if (logo_pic) {
                ImGui::Image((void*)logo_pic, ImVec2((float)logo_width, (float)logo_height));
            } else {
                ImGui::Text("failed to load image :(");
            }
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("VISUALS"))
        {
            GradientPresets::Sexy("VISUALS");
            ImGui::Checkbox("ESP", &espEnabled);
            ImGui::Checkbox("TEAM ESP", &teamEsp);

            
            ImGui::Separator();

            ImGui::SliderFloat("FOV", &fov, 30.0f, 200.0f);

            ImGui::Separator();
            
            ImGui::Checkbox("FOV Circle", &fov_circle_enabled);
            ImGui::Checkbox("snaplines", &snaplines);
            ImGui::Separator();
            ImGui::Checkbox("snaplines, show all", &snaplines_all);
            
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("RAGE"))
        {
            GradientPresets::Sexy("RAGE");
            
            ImGui::Checkbox("SPINBOT", &spinbot_enabled);
            ImGui::SliderInt("SPEED", &spinbot_speed, 0, 50);
            
            ImGui::Separator();
            
            ImGui::Checkbox("RAPID FIRE (broken/unreliable)", &rapid_fire_enabled);
            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem("AIMBOT"))
        {
            GradientPresets::Sexy("AIMBOT");
            
            ImGui::Checkbox("AIMBOT", &aimbot_enabled);
            ImGui::Checkbox("AIM AT ALL", &aimbot_all);

            ImGui::Separator();

            ImGui::Text("Toggle aimbot with Z");

            ImGui::Separator();

            ImGui::SliderFloat("HEAD OFFSET", &headoffset, -2.0f, 2.0f);
            ImGui::Text("DEFAULT: 0.275");
            ImGui::Text("+ MORE");
            ImGui::Text("- LESS");
            ImGui::Separator();

//            ImGui::SliderFloat("aim offset", &headoffset, -5.0f, 5.0f);
            ImGui::SliderFloat("MAX DISTANCE", &aimbot_max_distance, 5.0f, 750.0f);
            ImGui::SliderFloat("FOV", &aimbot_fov, 5.0f, 180.0f);
        
            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem("CONFIG"))
        {
            GradientPresets::Sexy("CONFIG");

            if (ImGui::Button("apply to config")) {
                Config::setfovcircleenabled(fov_circle_enabled);
                Config::setaimbotfov(aimbot_fov);
                Config::setaimbotdist(aimbot_max_distance);
                Config::setsnaplines(snaplines);
                Config::save("config.json");
            }
        
            ImGui::EndTabItem();
        }


        ImGui::EndTabBar(); 
    }
    

        ImGui::Spacing();

        ImGui::End();

        EndImGuiFrame();
    }


    if (logo_pic) {
        logo_pic->Release();
        logo_pic = nullptr;
    }
    ShutdownImGui();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    CloseHandle(hProcess);
    return 0;
}