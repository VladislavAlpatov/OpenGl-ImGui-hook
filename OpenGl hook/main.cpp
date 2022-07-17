#include <Windows.h>
#include <MinHook.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "Utils/memory.h"

#include <gl/GL.h>


LPVOID oWglSwapBuffers = NULL;



__int64 __fastcall hWglSwapBuffers(HDC hdc)
{
    static bool imGuiInitialized = false;

    if (!imGuiInitialized)
    {
        imGuiInitialized = true;
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(WindowFromDC(hdc));
        ImGui_ImplOpenGL3_Init();
        ImGui::StyleColorsDark();

    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::GetBackgroundDrawList()->AddText(ImVec2(), ImColor(255, 255, 255), u8"BRO YOU GOT HACKED!!!");

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    typedef __int64(__fastcall* WglSwapBuffers_t)(HDC);
    return reinterpret_cast<WglSwapBuffers_t>(oWglSwapBuffers)(hdc);
}

DWORD WINAPI HackThread(HMODULE hModule)
{
    MH_Initialize();

    auto addr = (LPVOID)Memory::FindPattern("opengl32.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 48 8B F1 33 FF");

    MH_CreateHook(addr, hWglSwapBuffers, &oWglSwapBuffers);
    MH_EnableHook(MH_ALL_HOOKS);

    while (!GetAsyncKeyState(VK_END))
    {
        Sleep(50);
    }

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

