#include <Windows.h>
#include <MinHook.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "Utils/memory.h"

#include <gl/GL.h>



LPVOID oWglSwapBuffers = NULL;
HDC    appHDC = 0;
LONG_PTR oWndProc;
bool imGuiInitialized = false;

__int64 __fastcall hWglSwapBuffers(HDC hdc)
{

    if (!imGuiInitialized)
    {
        appHDC = hdc;


        ImGui::CreateContext();
        ImGui_ImplWin32_Init(WindowFromDC(hdc));
        ImGui_ImplOpenGL3_Init();
        ImGui::StyleColorsDark();

        ImGui::GetStyle().AntiAliasedLinesUseTex = false;
        auto io = ImGui::GetIO();

        ImFontConfig cfg;
        static ImWchar ranges[] = { 0x1, 0xFFFD, 0 };
        auto& style = ImGui::GetStyle();
        auto& theme = style.Colors;

        style.FrameBorderSize = 1;
        style.AntiAliasedLinesUseTex = false;
        style.AntiAliasedLines = false;
        style.AntiAliasedFill = false;
        style.ScrollbarRounding = 0.f;
        style.WindowMinSize = ImVec2(10, 10);

        theme[ImGuiCol_WindowBg] = ImColor(24, 31, 35, 255);
        theme[ImGuiCol_Button] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_Tab] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_SeparatorActive] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_Border] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        theme[ImGuiCol_ButtonActive] = ImVec4(1.f, 0.57f, 0.57f, 1.f);
        theme[ImGuiCol_ButtonHovered] = ImVec4(1.f, 0.4f, 0.4f, 1.f);
        theme[ImGuiCol_CheckMark] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_TextSelectedBg] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_FrameBg] = ImVec4(0.31f, 0.31f, 0.31f, 0);
        theme[ImGuiCol_FrameBgActive] = ImVec4(1.f, 0.57f, 0.57f, 0);
        theme[ImGuiCol_FrameBgHovered] = ImVec4(0.41f, 0.41f, 0.41f, 0);
        theme[ImGuiCol_PopupBg] = ImColor(24, 31, 35, 255);
        theme[ImGuiCol_ScrollbarBg] = ImVec4(1.f, 0.372f, 0.372f, 0.f);
        theme[ImGuiCol_ScrollbarGrab] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_SliderGrab] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_SliderGrabActive] = ImVec4(1.f, 0.372f, 0.372f, 1.f);
        theme[ImGuiCol_TabHovered] = ImVec4(1.f, 0.57f, 0.57f, 1.f);
        theme[ImGuiCol_TabActive] = ImVec4(1.f, 0.372f, 0.372f, 1.f);

        imGuiInitialized = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::GetBackgroundDrawList()->AddText({}, ImColor(255, 255, 255), "Hello!");
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    typedef __int64(__fastcall* WglSwapBuffers_t)(HDC);
    return reinterpret_cast<WglSwapBuffers_t>(oWglSwapBuffers)(hdc);
}

LRESULT WINAPI WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);


    typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
    return CallWindowProc((WNDPROC)oWndProc, hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI HackThread(HMODULE hModule)
{
    MH_Initialize();

    auto wglSwapBuffersAddr = (LPVOID)Memory::FindPattern("opengl32.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 48 8B F1 33 FF");
    // x86 8B FF 55 8B EC 83 E4 F8 83 EC 14 53 56
    // x64 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 48 8B F1 33 FF
    MH_CreateHook(wglSwapBuffersAddr, hWglSwapBuffers, &oWglSwapBuffers);
    MH_EnableHook(MH_ALL_HOOKS);

    while (!imGuiInitialized)
        Sleep(50);

    auto window = WindowFromDC(appHDC);
    oWndProc = SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

    while (!GetAsyncKeyState(VK_END))
    {
        Sleep(50);
    }
    SetWindowLongPtr(WindowFromDC(appHDC), GWLP_WNDPROC, oWndProc);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    ImGui::DestroyContext();
    

    FreeLibraryAndExitThread(hModule, 0);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr);

    return TRUE;
}

