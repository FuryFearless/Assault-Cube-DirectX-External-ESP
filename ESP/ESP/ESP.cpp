#define _CRT_NON_CONFORMING_WCSTOK
#define _CRT_SECURE_NO_WARNINGS
#include "vector.h"
#include "dx_overlay.h"
#include "dx_renderer.h"
#include "W2SandGetModuleBaseAdress.h"
#include <iostream>
#include <string>
#include <codecvt>
#include <Windows.h>

HANDLE handle;

std::wstring readpChar(DWORD address) {
    try {
        if (address != 0) {
            const size_t namesize = 200;
            char x[namesize];
            ReadProcessMemory(handle, (LPCVOID)address, &x, namesize, NULL);
            std::wstring tmpname = std::wstring(&x[0], &x[namesize]);
            wchar_t* czech = wcstok(&tmpname[0], L"\0");
            if (czech != nullptr) return czech;
        }
    }
    catch (const std::exception& exc) {}
    return std::wstring(L"\0");
}


int main()
{
   
    DWORD pID;
    HWND hwnd = FindWindowA(NULL, "AssaultCube");
    GetWindowThreadProcessId(hwnd, &pID);
    handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    uintptr_t gameModule = GetModuleBaseAddress(pID,L"ac_client");

    forceinline::dx_overlay overlay(L"SDL_app",L"AssaultCube", true/**/);
    if (!overlay.is_initialized())
        return -1;

    forceinline::dx_renderer renderer = overlay.create_renderer();


    MSG m;
    ZeroMemory(&m, sizeof(m));


    while (m.message != WM_QUIT)
    {
        if (PeekMessage(&m, overlay.get_overlay_wnd(), NULL, NULL, PM_REMOVE))
        {
            TranslateMessage(&m);
            DispatchMessage(&m);
        }



        int players;
        ReadProcessMemory(handle, (LPVOID)(0x400000 + 0x10F500), &players, sizeof(players), 0);
        uintptr_t entitylist;
        ReadProcessMemory(handle, (LPVOID)(0x400000 + 0x10F4F8), &entitylist, sizeof(entitylist), 0);




        while (true)
        {
            renderer.begin_rendering();
            for (int i = 0; i < players; ++i)
            {

                uintptr_t ent;
                ReadProcessMemory(handle, (LPVOID)(entitylist + (i * 4)), &ent, sizeof(ent), 0);

                if (ent == 0)
                    continue;


                int health;
                ReadProcessMemory(handle, (LPVOID)(ent + 0xF8), &health, sizeof(health), 0);

                //if (health > 0)
                //    std::cout << health << std::endl;

                std::cout << health << std::endl;

                vec3d_f position_enemy;
                ReadProcessMemory(handle, (LPVOID)(ent + 0x34), &position_enemy, sizeof(position_enemy), 0);
                vec3d_f position_enemy_out;

                float matrix[16];
                ReadProcessMemory(handle, (LPVOID)(0x501AE8), &matrix, sizeof(matrix), 0);
                std::wstring name = readpChar(ent + 0x225);

                if (WorldToScreen(position_enemy, position_enemy_out, matrix, 1024, 768))
                {
                    //renderer.draw_text();
                    renderer.draw_outlined_rect(position_enemy_out.x, position_enemy_out.y + 15, 10, 10, D3DCOLOR_RGBA(88, 255, 71, 255));
                    renderer.draw_line(position_enemy_out.x, position_enemy_out.y + 15, 10, 10, D3DCOLOR_RGBA(0, 255, 255, 255));
                    renderer.draw_filled_rect(position_enemy_out.x, position_enemy_out.y + 15, 40, 30, D3DCOLOR_RGBA(0, 0, 255, 255));
                    renderer.draw_rect(position_enemy_out.x, position_enemy_out.y - 45, 30, 90, D3DCOLOR_RGBA(255, 0, 0, 255));
                    renderer.draw_text(name, position_enemy_out.x, position_enemy_out.y, D3DCOLOR_RGBA(255, 155, 255, 255));

                    renderer.get_fps();
                }


                
            }
            renderer.end_rendering();
            system("cls");
        }




    }


}