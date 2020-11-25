#include "pch-il2cpp.h"
#define WIN32_LEAN_AND_MEAN
#include <codecvt>
#include <d3d11.h>
#include <optional>
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

////////// STATE //////////

bool ShowMenu = true;

bool MarkImpostors = false;
bool NoClip = false;

bool ModifySpeed = false;
bool ModifyLight = false;
bool ModifyKillCooldown = false;
bool ModifyKillDistance = false;
float SpeedModifier = 1.90f;
float LightModifier = 10.00f;
float KillCooldownModifier = 0.001f;
int KillDistanceModifier = 2;

//char Message[256] = "";

std::optional<PlayerControl*> SelectedPlayer = std::nullopt;
std::optional<PlayerControl*> MurderTarget = std::nullopt;

////////// COLORS //////////

enum PlayerColors {
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

ImVec4 ColorToImVec4(Color color)
{
    return ImVec4(color.r, color.g, color.b, color.a);
}

ImVec4 rgb(unsigned char r, unsigned char g, unsigned char b)
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

////////// UTIL //////////

bool IsInGame()
{
    return (*AmongUsClient__TypeInfo)->static_fields->Instance->fields._.GameState == InnerNetClient_GameStates__Enum_Joined
        || (*AmongUsClient__TypeInfo)->static_fields->Instance->fields._.GameState == InnerNetClient_GameStates__Enum_Started;
}

bool HasGameStarted()
{
    return (*AmongUsClient__TypeInfo)->static_fields->Instance->fields._.GameState == InnerNetClient_GameStates__Enum_Started;
}

std::vector<PlayerControl*> GetPlayers()
{
    std::vector<PlayerControl*> players = std::vector<PlayerControl*>();
    List_1_PlayerControl_* playerList = (*PlayerControl__TypeInfo)->static_fields->AllPlayerControls;

    for (int i = 0; i < List_1_PlayerControl__get_Count(playerList, NULL); i++)
        players.push_back(List_1_PlayerControl__get_Item(playerList, i, NULL));
    return players;
}

std::string GetUTF8StringFromNETString(String* netString)
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

////////// GUI //////////

void RenderMenu(bool* p_open)
{
    ImGui::Begin("spacemafia v2", p_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::Text("press F5 to hide/show menu");

    if (ImGui::CollapsingHeader("Mods", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("No Clip", &NoClip);
        ImGui::Checkbox("Mark Impostors", &MarkImpostors);

        if (ImGui::Button("Call Meeting") && HasGameStarted())
            PlayerControl_CmdReportDeadBody((*PlayerControl__TypeInfo)->static_fields->LocalPlayer, NULL, NULL);
    }

    if (ImGui::CollapsingHeader("Game Options", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Light Modifier");
        ImGui::Checkbox("##ModifyLight", &ModifyLight);
        ImGui::SameLine();
        ImGui::SliderFloat("##LightModifier", &LightModifier, 0.0f, 10.0f);

        ImGui::Text("Speed Modifier");
        ImGui::Checkbox("##ModifySpeed", &ModifySpeed);
        ImGui::SameLine();
        ImGui::SliderFloat("##SpeedModifier", &SpeedModifier, 0.5f, 6.0f);

        ImGui::Text("Kill Cooldown Modifier");
        ImGui::Checkbox("##ModifyKillCooldown", &ModifyKillCooldown);
        ImGui::SameLine();
        ImGui::SliderFloat("##KillCooldownModifier", &KillCooldownModifier, 0.001f, 60.0f);

        ImGui::Text("Kill Distance Modifier");
        ImGui::Checkbox("##ModifyKillDistance", &ModifyKillDistance);
        //ImGui::SameLine();
        //ImGui::SliderInt("##KillDistanceModifier", &KillDistanceModifier, 0, 2);
        String__Array* killDistancesNames = (*GameOptionsData__TypeInfo)->static_fields->KillDistanceStrings;
        for (int i = 0; i < 3; i++) {
            ImGui::SameLine();
            ImGui::RadioButton(GetUTF8StringFromNETString(killDistancesNames->vector[i]).c_str(), &KillDistanceModifier, i);
        }
    }

    if (ImGui::CollapsingHeader("Tasks", ImGuiTreeNodeFlags_DefaultOpen) && HasGameStarted() && (*PlayerControl__TypeInfo)->static_fields->LocalPlayer && PlayerControl_get_Data((*PlayerControl__TypeInfo)->static_fields->LocalPlayer, NULL)->fields.Tasks) {
        List_1_GameData_TaskInfo_* tasks = PlayerControl_get_Data((*PlayerControl__TypeInfo)->static_fields->LocalPlayer, NULL)->fields.Tasks;

        for (int i = 0; i < List_1_GameData_TaskInfo__get_Count(tasks, NULL); i++) {
            GameData_TaskInfo* task = List_1_GameData_TaskInfo__get_Item(tasks, i, NULL);

            if (ImGui::Button(("Complete##Task" + std::to_string(task->fields.Id)).c_str()) && !task->fields.Complete) {
                PlayerControl_RpcCompleteTask((*PlayerControl__TypeInfo)->static_fields->LocalPlayer, task->fields.Id, NULL);
            }

            ImGui::SameLine();

            ImGui::TextColored(task->fields.Complete
                    ? ImVec4(0.0F, 1.0F, 0.0F, 1.0F)
                    : ColorToImVec4((*Palette__TypeInfo)->static_fields->DisabledGrey),
                (std::string("Task #") + std::to_string(task->fields.Id)).c_str());
        }
    }

    if (ImGui::CollapsingHeader("Players", ImGuiTreeNodeFlags_DefaultOpen) && IsInGame() && (*PlayerControl__TypeInfo)->static_fields->LocalPlayer) {
        for (auto player : GetPlayers()) {
            auto data = PlayerControl_get_Data(player, NULL);
            auto name = GetUTF8StringFromNETString(data->fields.PlayerName);

            ImVec4 nameColor = WHITE;

            if (HasGameStarted()) {
                if (data->fields.IsImpostor && MarkImpostors)
                    nameColor = ColorToImVec4((*Palette__TypeInfo)->static_fields->ImpostorRed);
                if (data->fields.IsDead)
                    nameColor = ColorToImVec4((*Palette__TypeInfo)->static_fields->DisabledGrey);
            }

            ImGui::PushStyleColor(ImGuiCol_Text, nameColor);
            if (ImGui::RadioButton(name.c_str(), (SelectedPlayer.has_value() && player == SelectedPlayer.value()) ? true : false)) {
                if (SelectedPlayer.has_value() && player == SelectedPlayer.value())
                    SelectedPlayer = std::nullopt;
                else
                    SelectedPlayer = player;
            }
            ImGui::PopStyleColor(1);

            if (HasGameStarted()) {
                List_1_GameData_TaskInfo_* tasks = data->fields.Tasks;
                float compl_tasks = 0.0f;
                float incompl_tasks = 0.0f;
                float task_perc = 0.0f;
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
                ImGui::SameLine(108.0f);
                ImVec4 playerColor = ColorFromId(data->fields.ColorId);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, playerColor);
                ImGui::ProgressBar(task_perc * 0.01f, ImVec2(ImGui::GetContentRegionAvailWidth(), 0));
                ImGui::PopStyleColor(1);
            }
        }
        if (ImGui::Button("Teleport")) {
            if (SelectedPlayer.has_value()) {
                Transform* localTransform = Component_get_transform((Component*)(*PlayerControl__TypeInfo)->static_fields->LocalPlayer, NULL);
                Transform* playerTransform = Component_get_transform((Component*)SelectedPlayer.value(), NULL);
                Transform_set_position(localTransform, Transform_get_position(playerTransform, NULL), NULL);
            }
        }
        if (HasGameStarted()) {
            ImGui::SameLine();
            if (ImGui::Button("Murder"))
                MurderTarget = SelectedPlayer;
        }

        /*
		static char Message[256] = "";
		ImGui::Text("Message");
		ImGui::SameLine();
		if (ImGui::InputText("##Message", Message, IM_ARRAYSIZE(Message), ImGuiInputTextFlags_EnterReturnsTrue)) {
			if (SelectedPlayer.has_value()) {
				PlayerControl_RpcSendChat(SelectedPlayer.value(), Marshal_PtrToStringAnsi(Message, NULL), NULL);
				Message[0] = '\0';
			}
		}
		*/
    }

    ImGui::End();
}

////////// HOOKS //////////

typedef HRESULT(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent = NULL;
WNDPROC oWndProc = NULL;
void (*oKeyboardJoystick_Update)(KeyboardJoystick*, MethodInfo*);
void (*oMeetingHud_Update)(MeetingHud*, MethodInfo*);
void (*oPlayerControl_FixedUpdate)(PlayerControl*, MethodInfo*);
HatBehaviour__Array* (*oHatManager_GetUnlockedHats)(HatManager*, MethodInfo*);
PetBehaviour__Array* (*oHatManager_GetUnlockedPets)(HatManager*, MethodInfo*);
SkinData__Array* (*oHatManager_GetUnlockedSkins)(HatManager*, MethodInfo*);

void hkKeyboardJoystick_Update(KeyboardJoystick* __this, MethodInfo* method)
{
    if ((*AmongUsClient__TypeInfo)->static_fields->Instance->fields._.GameState == InnerNetClient_GameStates__Enum_Ended) {
        SelectedPlayer = std::nullopt;
        MurderTarget = std::nullopt;
    }
    if (IsInGame()) {
        if (NoClip) {
            auto comp = Component_get_gameObject((Component*)(*PlayerControl__TypeInfo)->static_fields->LocalPlayer, NULL);
            GameObject_set_layer(comp, LayerMask_NameToLayer(Marshal_PtrToStringAnsi((void*)"Ghost", NULL), NULL), NULL);
        }
    }
    if (HasGameStarted()) {
        if (MurderTarget.has_value()) {
            auto player = (*PlayerControl__TypeInfo)->static_fields->LocalPlayer;
            auto data = PlayerControl_get_Data(player, NULL);
            if (data->fields.IsImpostor && !data->fields.IsDead) {
                PlayerControl_RpcMurderPlayer(player, MurderTarget.value(), NULL);
            }
            MurderTarget = std::nullopt;
        }
        if (ModifySpeed)
            (*PlayerControl__TypeInfo)->static_fields->GameOptions->fields.PlayerSpeedMod = SpeedModifier;
        if (ModifyLight) {
            (*PlayerControl__TypeInfo)->static_fields->GameOptions->fields.ImpostorLightMod = LightModifier;
            (*PlayerControl__TypeInfo)->static_fields->GameOptions->fields.CrewLightMod = LightModifier;
        }
        if (ModifyKillCooldown)
            (*PlayerControl__TypeInfo)->static_fields->GameOptions->fields.KillCooldown = KillCooldownModifier;
        if (ModifyKillDistance)
            (*PlayerControl__TypeInfo)->static_fields->GameOptions->fields.KillDistance = KillDistanceModifier;
    }

    oKeyboardJoystick_Update(__this, method);
}

void hkMeetingHud_Update(MeetingHud* __this, MethodInfo* method)
{
    oMeetingHud_Update(__this, method);
}

void hkPlayerControl_FixedUpdate(PlayerControl* __this, MethodInfo* method)
{
    if (HasGameStarted()) {
        if (MarkImpostors) {
            auto data = PlayerControl_get_Data(__this, NULL);
            if (data->fields.IsImpostor)
                __this->fields.nameText->fields.Color = (*Palette__TypeInfo)->static_fields->ImpostorRed;
        }
    }

    oPlayerControl_FixedUpdate(__this, method);
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

LRESULT __stdcall WndProc(const HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_KEYUP:
        switch (wParam) {
        case VK_F5:
        case VK_INSERT:
        case VK_DELETE:
            ShowMenu = !ShowMenu;
            break;
        }
        break;
    }
    if (ShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
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

    if (ShowMenu)
        RenderMenu(&ShowMenu);

    ImGui::Render();
    context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return oPresent(pSwapChain, SyncInterval, Flags);
}

////////// MAIN //////////

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    oKeyboardJoystick_Update = KeyboardJoystick_Update;
    oMeetingHud_Update = MeetingHud_Update;
    oPlayerControl_FixedUpdate = PlayerControl_FixedUpdate;
    oHatManager_GetUnlockedHats = HatManager_GetUnlockedHats;
    oHatManager_GetUnlockedPets = HatManager_GetUnlockedPets;
    oHatManager_GetUnlockedSkins = HatManager_GetUnlockedSkins;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)oKeyboardJoystick_Update, hkKeyboardJoystick_Update);
    //DetourAttach(&(PVOID&)oMeetingHud_Update, hkMeetingHud_Update);
    DetourAttach(&(PVOID&)oPlayerControl_FixedUpdate, hkPlayerControl_FixedUpdate);
    DetourAttach(&(PVOID&)oHatManager_GetUnlockedHats, hkHatManager_GetUnlockedHats);
    DetourAttach(&(PVOID&)oHatManager_GetUnlockedPets, hkHatManager_GetUnlockedPets);
    DetourAttach(&(PVOID&)oHatManager_GetUnlockedSkins, hkHatManager_GetUnlockedSkins);

    if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
        //kiero::bind(8, (void**)&oPresent, hkPresent);
        oPresent = (Present)kiero::getMethodsTable()[8];
        if (oPresent)
            DetourAttach(&(PVOID&)oPresent, hkPresent);
    }

    DetourTransactionCommit();
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
        break;
    }
    return TRUE;
}
