#pragma once
#include <windows.h>

#include "Engine/Application.h"

class SystemWindow
{
private:
    static inline Application s_App;
    static constexpr wchar_t CLASS_NAME[] = L"GPUCullingWindowClass";
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    static int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
};

