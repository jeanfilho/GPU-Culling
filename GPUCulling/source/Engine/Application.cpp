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
    assert(m_gpuDevice.Initialize());

    ID3D12Device* device = m_gpuDevice.GetDevice();
    assert(device);

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    
    HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    assert(SUCCEEDED(hr) && m_commandQueue);

    // Create swap chain
    m_swapChain = std::make_unique<GPUSwapChain>();
    if (!m_swapChain->Initialize(device, m_commandQueue, hwnd, m_width, m_height))
    {
        m_swapChain.reset();
        m_commandQueue->Release();
        m_commandQueue = nullptr;
        m_gpuDevice.Release();
        return;
    }

    // Create renderer
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(device, m_commandQueue))
    {
        m_renderer.reset();
        m_swapChain.reset();
        m_commandQueue->Release();
        m_commandQueue = nullptr;
        m_gpuDevice.Release();
        return;
    }

    // Set initial viewport
    m_renderer->SetViewport((float)m_width, (float)m_height);
}

void Application::Shutdown()
{
    m_renderer.reset();
    m_swapChain.reset();

    if (m_commandQueue)
    {
        m_commandQueue->Release();
        m_commandQueue = nullptr;
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
