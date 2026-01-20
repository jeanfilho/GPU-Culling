#include "stdafx.h"
#include "Application.h"

void Application::Startup(HWND hwnd)
{
    m_hwnd = hwnd;

    // Get window size
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    m_width = clientRect.right - clientRect.left;
    m_height = clientRect.bottom - clientRect.top;

    // Initialize GPU device
    if (!m_gpuDevice.Initialize())
    {
        return;
    }

    ID3D12Device* device = m_gpuDevice.GetDevice();
    if (!device)
    {
        return;
    }

    // Create swap chain
    m_swapChain = new GPUSwapChain();
    if (!m_swapChain->Initialize(device, nullptr, hwnd, m_width, m_height))
    {
        delete m_swapChain;
        m_swapChain = nullptr;
        m_gpuDevice.Release();
        return;
    }

    // Need command queue from GPUDevice - for now create a temporary one
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    
    ID3D12CommandQueue* commandQueue = nullptr;
    HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    if (FAILED(hr) || !commandQueue)
    {
        m_swapChain->Release();
        delete m_swapChain;
        m_swapChain = nullptr;
        m_gpuDevice.Release();
        return;
    }

    // Create renderer
    m_renderer = new Renderer();
    if (!m_renderer->Initialize(device, commandQueue))
    {
        m_renderer->Release();
        delete m_renderer;
        m_renderer = nullptr;
        commandQueue->Release();
        m_swapChain->Release();
        delete m_swapChain;
        m_swapChain = nullptr;
        m_gpuDevice.Release();
        return;
    }

    // Set initial viewport
    m_renderer->SetViewport((float)m_width, (float)m_height);
}

void Application::Shutdown()
{
    if (m_renderer)
    {
        m_renderer->Release();
        delete m_renderer;
        m_renderer = nullptr;
    }

    if (m_swapChain)
    {
        m_swapChain->Release();
        delete m_swapChain;
        m_swapChain = nullptr;
    }

    m_gpuDevice.Release();
}

void Application::Present()
{
    if (!m_renderer || !m_swapChain)
    {
        return;
    }

    // Set current back buffer as render target
    m_renderer->SetRenderTarget(
        m_swapChain->GetBackBufferRTV(),
        m_swapChain->GetDepthStencilView()
    );

    // Begin frame
    m_renderer->BeginFrame();

    // Render
    m_renderer->Render();

    // End frame
    m_renderer->EndFrame();

    // Present to screen
    m_swapChain->Present();
}

void Application::Simulate()
{
}

void Application::Resize(UINT width, UINT height)
{
    if (!m_swapChain || width == 0 || height == 0)
    {
        return;
    }

    m_width = width;
    m_height = height;

    m_swapChain->Resize(width, height);

    if (m_renderer)
    {
        m_renderer->SetViewport((float)width, (float)height);
    }
}
