#include "pch-il2cpp.h"
#define WIN32_LEAN_AND_MEAN
#include <codecvt>
#include <d3d11.h>
#include <string>
#include <vector>
#include <windows.h>

#include "detours.h"
#include "il2cpp-init.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "kiero.h"

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

bool HasGameStarted()
{
    return AmongUsClient__TypeInfo->static_fields->Instance->fields._.GameState == InnerNetClient_GameStates__Enum_Started;
}

std::vector<PlayerControl*> GetPlayers()
{
    std::vector<PlayerControl*> players = std::vector<PlayerControl*>();
    List_1_PlayerControl_* playerList = PlayerControl__TypeInfo->static_fields->AllPlayerControls;

    for (int i = 0; i < List_1_PlayerControl__get_Count(playerList, NULL); i++)
        players.push_back(List_1_PlayerControl__get_Item(playerList, i, NULL));
    return players;
}

std::string GetUTF8StringFromNETString(app::String* netString)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10FFFF, std::codecvt_mode::little_endian>, char16_t> UTF16_TO_UTF8;

    if (netString == NULL)
        return NULL;

    uint16_t* buffer = new uint16_t[netString->fields.m_stringLength + 1];
    memcpy(buffer, &netString->fields.m_firstChar, netString->fields.m_stringLength * sizeof(uint16_t));
    buffer[netString->fields.m_stringLength] = L'\0';
    std::string newString = UTF16_TO_UTF8.to_bytes((const char16_t*)buffer);

    delete[] buffer;

    return newString;
}

ImVec4 ColorToImVec4(Color color)
{
    return ImVec4(color.r, color.g, color.b, color.a);
}

ImVec4 rgb(BYTE r, BYTE g, BYTE b)
{
    return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
};

ImVec4 WHITE = rgb(0xFF, 0xFF, 0xFF);
// Among Us Colors
ImVec4 Red = rgb(0xC5, 0x11, 0x11); // Thunderbird
ImVec4 Blue = rgb(0x13, 0x2E, 0xD1); // Persian Blue
ImVec4 Green = rgb(0x11, 0x7F, 0x2D); // Jewel
ImVec4 Pink = rgb(0xED, 0x54, 0xBA); // Brilliant Rose
ImVec4 Orange = rgb(0xEF, 0x7D, 0x0E); // Christine
ImVec4 Yellow = rgb(0xF6, 0xF6, 0x58); // Starship
ImVec4 Black = rgb(0x3F, 0x47, 0x4E); // Mako
ImVec4 White = rgb(0xD6, 0xE0, 0xF0); // Periwinkle Grey
ImVec4 Purple = rgb(0x6B, 0x31, 0xBC); // Purple Heart
ImVec4 Brown = rgb(0x71, 0x49, 0x1E); // Walnut
ImVec4 Cyan = rgb(0x38, 0xFE, 0xDB); // Bright Turquoise
ImVec4 Lime = rgb(0x50, 0xEF, 0x39); // Screamin' Green
ImVec4 Fortegreen = rgb(0x1D, 0x98, 0x53);
ImVec4 Tan = rgb(0x50, 0xEF, 0x39);

enum playerColors {
    RedColorId,
    BlueColorId,
    GreenColorId,
    PinkColorId,
    OrangeColorId,
    YellowColorId,
    BlackColorId,
    WhiteColorId,
    PurpleColorId,
    BrownColorId,
    CyanColorId,
    LimeColorId,
};

ImVec4 ColorFromId(unsigned int colorId)
{
    ImVec4 color = WHITE;
    switch (colorId) {
    case RedColorId:
        color = Red;
        break;
    case BlueColorId:
        color = Blue;
        break;
    case GreenColorId:
        color = Green;
        break;
    case PinkColorId:
        color = Pink;
        break;
    case OrangeColorId:
        color = Orange;
        break;
    case YellowColorId:
        color = Yellow;
        break;
    case BlackColorId:
        color = Black;
        break;
    case WhiteColorId:
        color = White;
        break;
    case PurpleColorId:
        color = Purple;
        break;
    case BrownColorId:
        color = Brown;
        break;
    case CyanColorId:
        color = Cyan;
        break;
    case LimeColorId:
        color = Lime;
        break;
    }
    return color;
}

void Patch(BYTE* dst, BYTE* src, unsigned int size)
{
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldprotect, &oldprotect);
}

HatBehaviour__Array* hkHatManager_GetUnlockedHats(HatManager* __this, MethodInfo* method)
{
    return __this->fields.AllHats->fields._items;
}

PetBehaviour__Array* hkHatManager_GetUnlockedPets(HatManager* __this, MethodInfo* method)
{
    return __this->fields.AllPets->fields._items;
}

SkinData__Array* hkHatManager_GetUnlockedSkins(HatManager* __this, MethodInfo* method)
{
    return __this->fields.AllSkins->fields._items;
}

const unsigned char _compiledGetUnlockedHats[] = {
    0x55, //             push ebp
    0x8B, 0xEC, //       mov ebp,esp

    0x8B, 0x45, 0x08, // mov eax,[ebp+08]  __this  HatManager
    0x8B, 0x40, 0x1C, // mov eax,[eax+1C]  AllHats
    0x8B, 0x40, 0x08, // mov eax,[eax+08]  _items  HatBehaviour__Array

    0x5D, //             pop ebp
    0xC3 //              ret
};
const unsigned int sizeHatManager_GetUnlockedHats = sizeof(_compiledGetUnlockedHats);

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
            static bool allHats = false, allHatsEnabled = false;
            static bool allPets = false, allPetsEnabled = false;
            static bool allSkins = false, allSkinsEnabled = false;
            static bool speedEnabled = false;
            static bool lightEnabled = false;
            static bool killCooldownEnabled = false;
            static bool killDistanceEnabled = false;
            static float speed = 1.90f;
            static float light = 10.00f;
            static float killCooldown = 0.001f;
            const int maxKillDistance = 3;
            static int killDistance = maxKillDistance - 1;

            ImGui::Begin("spacemafia v1+", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

            ImGui::Text("press F5 to hide/show menu");

            if (ImGui::CollapsingHeader("Modifications", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("All Hats", &allHatsEnabled);
                ImGui::Checkbox("All Pets", &allPetsEnabled);
                ImGui::Checkbox("All Skins", &allSkinsEnabled);

                if (ImGui::Button("No Clip") && IsInGame() && PlayerControl__TypeInfo->static_fields->LocalPlayer) {
                    auto comp = Component_get_gameObject((Component*)PlayerControl__TypeInfo->static_fields->LocalPlayer, NULL);
                    GameObject_set_layer(comp, LayerMask_NameToLayer(Marshal_PtrToStringAnsi((void*)"Ghost", NULL), NULL), NULL);
                }

                if (ImGui::Button("Reveal Impostors") && HasGameStarted()) {
                    for (auto player : GetPlayers()) {
                        auto data = PlayerControl_get_Data(player, NULL);

                        TextRenderer* nameText = (TextRenderer*)(player->fields.RemainingEmergencies);
                        if (data->fields.IsImpostor)
                            nameText->fields.Color = Palette__TypeInfo->static_fields->ImpostorRed;
                    }
                }
            }

            if (ImGui::CollapsingHeader("Game Options", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Light");
                ImGui::Checkbox("##lightEnabled", &lightEnabled);
                ImGui::SameLine();
                ImGui::SliderFloat("##light", &light, 0.0f, 10.0f);

                ImGui::Text("Speed");
                ImGui::Checkbox("##speedEnabled", &speedEnabled);
                ImGui::SameLine();
                ImGui::SliderFloat("##speed", &speed, 0.5f, 6.0f);

                ImGui::Text("Kill Cooldown");
                ImGui::Checkbox("##killCooldownEnabled", &killCooldownEnabled);
                ImGui::SameLine();
                ImGui::SliderFloat("##killCooldown", &killCooldown, 0.001f, 60.0f);

                ImGui::Text("Kill Distance");
                ImGui::Checkbox("##killDistanceEnabled", &killDistanceEnabled);
                //ImGui::SameLine();
                //ImGui::SliderInt("##killDistance", &killDistance, 0, 2);
                String__Array* killDistancesNames = GameOptionsData__TypeInfo->static_fields->KillDistanceStrings;
                for (int i = 0; i < maxKillDistance; i++) {
                    ImGui::SameLine();
                    ImGui::RadioButton(GetUTF8StringFromNETString(killDistancesNames->vector[i]).c_str(), &killDistance, i);
                }
            }

            if (ImGui::CollapsingHeader("Tasks", ImGuiTreeNodeFlags_DefaultOpen) && HasGameStarted() && PlayerControl__TypeInfo->static_fields->LocalPlayer && PlayerControl_get_Data(PlayerControl__TypeInfo->static_fields->LocalPlayer, NULL)->fields.Tasks) {
                List_1_GameData_TaskInfo_* tasks = PlayerControl_get_Data(PlayerControl__TypeInfo->static_fields->LocalPlayer, NULL)->fields.Tasks;

                for (int i = 0; i < List_1_GameData_TaskInfo__get_Count(tasks, NULL); i++) {
                    GameData_TaskInfo* task = List_1_GameData_TaskInfo__get_Item(tasks, i, NULL);

                    if (!task->fields.Complete) {
                        if (ImGui::Button(("complete##task" + std::to_string(task->fields.Id)).c_str()) && !task->fields.Complete) {
                            PlayerControl_RpcCompleteTask(PlayerControl__TypeInfo->static_fields->LocalPlayer, task->fields.Id, NULL);
                        }

                        ImGui::SameLine();

                        ImGui::TextColored(ImVec4(0.0F, 1.0F, 0.0F, 1.0F), (std::string("task #") + std::to_string(task->fields.Id)).c_str());
                    }
                }
            }

            static int selected = 0;
            if (ImGui::CollapsingHeader("Players", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (IsInGame()) {
                    auto playerList = GetPlayers();

                    for (auto player : playerList) {
                        auto data = PlayerControl_get_Data(player, NULL);
                        auto name = GetUTF8StringFromNETString(data->fields.PlayerName);

                        ImVec4 nameColor = WHITE;
                        float task_perc = 0.0f;

                        if (HasGameStarted()) {
                            if (data->fields.IsImpostor)
                                nameColor = ColorToImVec4(Palette__TypeInfo->static_fields->ImpostorRed);
                            if (data->fields.IsDead)
                                nameColor = ColorToImVec4(Palette__TypeInfo->static_fields->DisabledGrey);

                            List_1_GameData_TaskInfo_* tasks = data->fields.Tasks;
                            float compl_tasks = 0.0f;
                            float incompl_tasks = 0.0f;
                            if (List_1_GameData_TaskInfo__get_Count(tasks, NULL) > 0) {
                                for (int i = 0; i < List_1_GameData_TaskInfo__get_Count(tasks, NULL); ++i) {
                                    GameData_TaskInfo* task = List_1_GameData_TaskInfo__get_Item(tasks, i, NULL);
                                    if (task->fields.Complete)
                                        compl_tasks += 1.0f;
                                    else
                                        incompl_tasks += 1.0f;
                                }
                                task_perc = (float)((int)(compl_tasks / (compl_tasks + incompl_tasks) * 100.f + .5f));
                            }
                        }

                        int colorId = data->fields.ColorId;
                        ImGui::RadioButton(("##player" + std::to_string(colorId)).c_str(), &selected, colorId);

                        ImGui::SameLine();

                        ImGui::PushStyleColor(ImGuiCol_Text, nameColor);
                        ImGui::Text(name.c_str());
                        ImGui::PopStyleColor(1);

                        if (HasGameStarted()) {
                            ImGui::SameLine(108.0f);
                            ImVec4 playerColor = ColorFromId(data->fields.ColorId);
                            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, playerColor);
                            ImGui::ProgressBar(task_perc * 0.01f, ImVec2(ImGui::GetContentRegionAvailWidth(), 0));
                            ImGui::PopStyleColor(1);
                        }
                    }
                }
                if (ImGui::Button("teleport")) {
                    if (IsInGame() && PlayerControl__TypeInfo->static_fields->LocalPlayer) {
                        for (auto player : GetPlayers()) {
                            auto data = PlayerControl_get_Data(player, NULL);

                            if (selected == data->fields.ColorId) {
                                Transform* localTransform = Component_get_transform((Component*)PlayerControl__TypeInfo->static_fields->LocalPlayer, NULL);
                                Transform* playerTransform = Component_get_transform((Component*)player, NULL);
                                Transform_set_position(localTransform, Transform_get_position(playerTransform, NULL), NULL);
                            }
                        }
                    }
                }
                /*
                ImGui::SameLine();
                if (ImGui::Button("kill")) {
                    if (HasGameStarted() && PlayerControl__TypeInfo->static_fields->LocalPlayer) {
                        for (auto player : playerList) {
                            auto data = PlayerControl_get_Data(player, NULL);

                            if (selected == data->fields.ColorId && !data->fields.IsDead)
                                PlayerControl_RpcMurderPlayer(PlayerControl__TypeInfo->static_fields->LocalPlayer, player, NULL);
                        }
                    }
                }
                */
            }

            ImGui::End();

            if (HasGameStarted() && PlayerControl__TypeInfo->static_fields->GameOptions) {
                if (speedEnabled)
                    PlayerControl__TypeInfo->static_fields->GameOptions->fields.PlayerSpeedMod = speed;
                if (lightEnabled) {
                    PlayerControl__TypeInfo->static_fields->GameOptions->fields.ImpostorLightMod = light;
                    PlayerControl__TypeInfo->static_fields->GameOptions->fields.CrewLightMod = light;
                }
                if (killCooldownEnabled)
                    PlayerControl__TypeInfo->static_fields->GameOptions->fields.KillCooldown = killCooldown;
                if (killDistanceEnabled)
                    PlayerControl__TypeInfo->static_fields->GameOptions->fields.KillDistance = killDistance;
            }

            static unsigned char originalHatManager_GetUnlockedHats[sizeHatManager_GetUnlockedHats];
            static unsigned char originalHatManager_GetUnlockedPets[sizeHatManager_GetUnlockedHats];
            static unsigned char originalHatManager_GetUnlockedSkins[sizeHatManager_GetUnlockedHats];
            if (allHatsEnabled) {
                if (!allHats) {
                    allHats = true;
                    memcpy(originalHatManager_GetUnlockedHats, HatManager_GetUnlockedHats, sizeHatManager_GetUnlockedHats);
                    Patch((BYTE*)HatManager_GetUnlockedHats, (BYTE*)hkHatManager_GetUnlockedHats, sizeHatManager_GetUnlockedHats);
                }
            } else {
                if (allHats) {
                    allHats = false;
                    Patch((BYTE*)HatManager_GetUnlockedHats, (BYTE*)originalHatManager_GetUnlockedHats, sizeHatManager_GetUnlockedHats);
                }
            }
            if (allPetsEnabled) {
                if (!allPets) {
                    allPets = true;
                    memcpy(originalHatManager_GetUnlockedPets, HatManager_GetUnlockedPets, sizeHatManager_GetUnlockedHats);
                    Patch((BYTE*)HatManager_GetUnlockedPets, (BYTE*)hkHatManager_GetUnlockedPets, sizeHatManager_GetUnlockedHats);
                }
            } else {
                if (allPets) {
                    allPets = false;
                    Patch((BYTE*)HatManager_GetUnlockedPets, (BYTE*)originalHatManager_GetUnlockedPets, sizeHatManager_GetUnlockedHats);
                }
            }
            if (allSkinsEnabled) {
                if (!allSkins) {
                    allSkins = true;
                    memcpy(originalHatManager_GetUnlockedSkins, HatManager_GetUnlockedSkins, sizeHatManager_GetUnlockedHats);
                    Patch((BYTE*)HatManager_GetUnlockedSkins, (BYTE*)hkHatManager_GetUnlockedSkins, sizeHatManager_GetUnlockedHats);
                }
            } else {
                if (allSkins) {
                    allSkins = false;
                    Patch((BYTE*)HatManager_GetUnlockedSkins, (BYTE*)originalHatManager_GetUnlockedSkins, sizeHatManager_GetUnlockedHats);
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
    if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
        //kiero::bind(8, (void**)&oPresent, hkPresent11);
        oPresent = (Present)kiero::getMethodsTable()[8];
        if (oPresent == NULL) {
            return FALSE;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (DetourAttach(&(PVOID&)oPresent, hkPresent11) != NO_ERROR) {
            return FALSE;
        }

        if (DetourTransactionCommit() != NO_ERROR) {
            return FALSE;
        }
    }
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
