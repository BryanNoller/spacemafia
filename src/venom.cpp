#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "kiero.h" // KIERO_INCLUDE_D3D11=1 KIERO_USE_MINHOOK=1
#include <d3d11.h>
#include <stdio.h>
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

Present oPresent = NULL;
WNDPROC oWndProc;

LRESULT __stdcall WndProc(const HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, Msg, wParam, lParam);
}

HRESULT __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool init = false;
    static DXGI_SWAP_CHAIN_DESC desc;
    static ID3D11Device* device;
    static ID3D11DeviceContext* context;
    static ID3D11RenderTargetView* mainRenderTargetView;

    if (!init) {
        pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);
        device->GetImmediateContext(&context);

        pSwapChain->GetDesc(&desc);
        oWndProc = (WNDPROC)SetWindowLongPtr(desc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);

        ID3D11Texture2D* pBackBuffer;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
        pBackBuffer->Release();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(desc.OutputWindow);
        ImGui_ImplDX11_Init(device, context);

        init = true;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Relpace this block with your code.
    {
        static bool show_demo_window = false;
        static ImVec4 color = ImVec4(0.788f, 0.00f, 0.259f, 1.00f);

        ImGui::Begin("venom");

        ImGui::Text("Hello World");
        ImGui::Checkbox("show ImGui demo window", &show_demo_window);
        ImGui::ColorEdit3("color", (float*)&color);
        ImGui::Text("average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
    }

    ImGui::EndFrame();
    ImGui::Render();
    context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        kiero::bind(8, (void**)&oPresent, hkPresent11);
    return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        kiero::shutdown();
        break;
    }
    return TRUE;
}
