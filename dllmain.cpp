// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include "mem.h"
#include "proc.h"

DWORD WINAPI HackThread(HMODULE hModule)
{
    // Create console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "TestText\n";


    // Get module base
    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");

    bool bHealth = false, bAmmo = false, bRecoil = false;

   

    // Hack loop
    while (true)
    {
        // Get key input
        if (GetAsyncKeyState(VK_END) & 1)
        {
            break;
        }

        if (GetAsyncKeyState(VK_NUMPAD1) & 1)
        {
            bHealth = !bHealth;
        }

        if (GetAsyncKeyState(VK_NUMPAD2) & 1)
        {
            bAmmo = !bAmmo;
        }

        if (GetAsyncKeyState(VK_NUMPAD3) & 1)
        {
            bRecoil = !bRecoil;
            if (bRecoil)
            {
                // nop
                mem::Nop((BYTE*)moduleBase + 0x63786, 10);
            }
            else
            {
                // revert to original
                //write back original instructions
                mem::Patch((BYTE*)moduleBase + 0x63786, (BYTE*)"\x50\x8d\x4c\x24\x1c\x51\x8b\xce\xff\xd2", 10);
            }
        }
    }

    // continous write/freeze
    uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10f4f4);
    if (localPlayerPtr)
    {
        if (bHealth)
        {
            *(int*)(*localPlayerPtr + 0xf8) = 1337;
        }

        if (bAmmo)
        {
            uintptr_t ammoAddr = mem::FindDMAAddy(moduleBase + 0x10f4f4, { 0x374, 0x14, 0x0 });
            int* ammo = (int*)ammoAddr;
            *ammo = 1337;

            // or just
            *(int*)mem::FindDMAAddy(moduleBase + 0x10f4f4, { 0x374, 0x14, 0x0 }) = 1337;
        }
        Sleep(5);
    }
    // Clean and Eject
    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
}

/*Function Explanation:
 *hModule: handle to the dll. Its value is the base address of the dll
 *ul_reason_for_call: this can be process attach, thread attatch, process detach, thread detach. */
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    // this is called when load library is called or the process is starting up
    case DLL_PROCESS_ATTACH:
    {
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    }
    // current process is creating a new thread?
    case DLL_THREAD_ATTACH:
    // thread memory cleaning??
    case DLL_THREAD_DETACH:
    // this reason is for when the dll failed to load, failed to attach to process, process has been terminated, or free library was called.
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

