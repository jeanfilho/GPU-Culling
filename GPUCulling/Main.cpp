#include "stdafx.h"

#include "SystemWindow.h"

int main()
{
    SystemWindow window;
    return window.WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOWDEFAULT);
}
