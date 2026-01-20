#pragma once

#include "Graphics/GPUDevice.h"
#include "Graphics/GPUSwapChain.h"
#include "Renderer.h"
#include <memory>

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
    ID3D12CommandQueue* m_commandQueue = nullptr;
    std::unique_ptr<GPUSwapChain> m_swapChain;
    std::unique_ptr<Renderer> m_renderer;

    HWND m_hwnd = nullptr;
    UINT m_width = 0;
    UINT m_height = 0;
};

