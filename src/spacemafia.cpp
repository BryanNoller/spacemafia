#include "pch-il2cpp.h"
#define WIN32_LEAN_AND_MEAN
#include "il2cpp-init.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "kiero.h"
#include <d3d11.h>
#include <stdio.h>
#include <windows.h>

using namespace app;
extern const LPCWSTR LOG_FILE = L"il2cpp-log.txt"; // unused but required by il2cpp

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

Present oPresent = NULL;
WNDPROC oWndProc;
bool showMenu = true;

bool IsInGame()
{
    return AmongUsClient__TypeInfo->static_fields->Instance->fields._.GameState == InnerNetClient_GameStates__Enum_Joined
        || AmongUsClient__TypeInfo->static_fields->Instance->fields._.GameState == InnerNetClient_GameStates__Enum_Started;
}

void Patch(BYTE* dst, BYTE* src, unsigned int size)
{
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldprotect, &oldprotect);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_KEYUP:
        switch (wParam) {
        case VK_F5:
            showMenu = !showMenu;
            break;
        }
        break;
    }
    if (showMenu && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
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

    if (showMenu) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            static bool unlockPets = false;
            static bool unlockPetsCheckbox = false;
            const unsigned char unlockPetsBytes[] = {
                0x55, // push ebp
                0x8B, 0xEC, // mov ebp,esp
                0x8B, 0x45, 0x08, // mov eax,[ebp+08]
                0x8B, 0x40, 0x18, // mov eax,[eax+18]
                0x8B, 0x40, 0x08, // mov eax,[eax+08]
                0x5D, // pop ebp
                0xC3 // ret
            };
            static unsigned char unlockPetsBytesOriginal[sizeof(unlockPetsBytes)];

            ImGui::Begin("spacemafia v0.1");
            ImGui::Text("press F5 to hide/show menu");

            ImGui::Checkbox("Unlock Pets", &unlockPetsCheckbox);

            ImGui::End();

            if (unlockPetsCheckbox) {
                if (!unlockPets) {
                    unlockPets = true;
                    memcpy(unlockPetsBytesOriginal, HatManager_GetUnlockedPets, sizeof(unlockPetsBytes));
                    Patch((BYTE*)HatManager_GetUnlockedPets, (BYTE*)unlockPetsBytes, sizeof(unlockPetsBytes));
                }
            } else {
                if (unlockPets) {
                    unlockPets = false;
                    Patch((BYTE*)HatManager_GetUnlockedPets, (BYTE*)unlockPetsBytesOriginal, sizeof(unlockPetsBytes));
                }
            }
        }

        ImGui::Render();
        context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
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
        init_il2cpp();
        CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        kiero::shutdown();
        break;
    }
    return TRUE;
}
