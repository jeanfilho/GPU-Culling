#pragma once

#include "Graphics/GPUDevice.h"
#include "Graphics/GPUSwapChain.h"
#include "Renderer.h"
#include <windows.h>

class Application
{
public:
    void Startup(HWND hwnd);
    void Shutdown();

    void Present();
    void Simulate();
    void Resize(UINT width, UINT height);

private:
    GPUDevice m_gpuDevice;
    GPUSwapChain* m_swapChain = nullptr;
    Renderer* m_renderer = nullptr;

    HWND m_hwnd = nullptr;
    UINT m_width = 0;
    UINT m_height = 0;
};

