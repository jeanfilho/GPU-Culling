#include "stdafx.h"
#include "Graphics/GPUSwapChain.h"

GPUSwapChain::~GPUSwapChain()
{
    Release();
}

bool GPUSwapChain::Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, HWND hwnd, UINT width, UINT height)
{
    assertm(device && commandQueue && hwnd && width != 0 && height != 0, "Invalid parameters passed to swapchain initialization");

    m_device = device;
    m_commandQueue = commandQueue;
    m_width = width;
    m_height = height;

    // Create DXGI factory
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));
    if (FAILED(hr))
    {
        return false;
    }

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = BACK_BUFFER_COUNT;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = BACK_BUFFER_FORMAT;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain1* swapChain1 = nullptr;
    hr = m_dxgiFactory->CreateSwapChainForHwnd(
        m_commandQueue,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );

    if (FAILED(hr))
    {
        m_dxgiFactory->Release();
        m_dxgiFactory = nullptr;
        return false;
    }

    // Query for IDXGISwapChain3
    hr = swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain));
    swapChain1->Release();

    if (FAILED(hr))
    {
        m_dxgiFactory->Release();
        m_dxgiFactory = nullptr;
        return false;
    }

    // Create RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = BACK_BUFFER_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap));
    if (FAILED(hr))
    {
        m_swapChain->Release();
        m_swapChain = nullptr;
        m_dxgiFactory->Release();
        m_dxgiFactory = nullptr;
        return false;
    }

    // Create DSV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvDescriptorHeap));
    if (FAILED(hr))
    {
        m_rtvDescriptorHeap->Release();
        m_rtvDescriptorHeap = nullptr;
        m_swapChain->Release();
        m_swapChain = nullptr;
        m_dxgiFactory->Release();
        m_dxgiFactory = nullptr;
        return false;
    }

    // Create render target views
    CreateRenderTargetViews();

    // Create depth stencil buffer and view
    CreateDepthStencilBuffer();
    CreateDepthStencilView();

    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    m_isInitialized = true;

    return true;
}

void GPUSwapChain::Release()
{
    if (m_depthStencilBuffer)
    {
        m_depthStencilBuffer->Release();
        m_depthStencilBuffer = nullptr;
    }

    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
    {
        if (m_backBuffers[i])
        {
            m_backBuffers[i]->Release();
            m_backBuffers[i] = nullptr;
        }
    }

    if (m_rtvDescriptorHeap)
    {
        m_rtvDescriptorHeap->Release();
        m_rtvDescriptorHeap = nullptr;
    }

    if (m_dsvDescriptorHeap)
    {
        m_dsvDescriptorHeap->Release();
        m_dsvDescriptorHeap = nullptr;
    }

    if (m_swapChain)
    {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }

    if (m_dxgiFactory)
    {
        m_dxgiFactory->Release();
        m_dxgiFactory = nullptr;
    }

    m_device = nullptr;
    m_commandQueue = nullptr;
    m_currentBackBufferIndex = 0;
    m_isInitialized = false;
}

void GPUSwapChain::Present()
{
    assert(m_swapChain);

    // Present with vsync
    HRESULT hr = m_swapChain->Present(1, 0);
    assert(SUCCEEDED(hr));

    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void GPUSwapChain::Resize(UINT width, UINT height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    assert(m_swapChain);

    // Release current resources
    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
    {
        if (m_backBuffers[i])
        {
            m_backBuffers[i]->Release();
            m_backBuffers[i] = nullptr;
        }
    }

    if (m_depthStencilBuffer)
    {
        m_depthStencilBuffer->Release();
        m_depthStencilBuffer = nullptr;
    }

    m_width = width;
    m_height = height;

    // Resize swap chain
    HRESULT hr = m_swapChain->ResizeBuffers(
        BACK_BUFFER_COUNT,
        width,
        height,
        BACK_BUFFER_FORMAT,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
    );
    assert(SUCCEEDED(hr));

    // Recreate render target views
    CreateRenderTargetViews();

    // Recreate depth stencil buffer and view
    CreateDepthStencilBuffer();
    CreateDepthStencilView();

    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

ID3D12Resource* GPUSwapChain::GetBackBuffer() const
{
    return GetBackBuffer(m_currentBackBufferIndex);
}

ID3D12Resource* GPUSwapChain::GetBackBuffer(UINT index) const
{
    assert(index < BACK_BUFFER_COUNT);
    return m_backBuffers[index];
}

D3D12_CPU_DESCRIPTOR_HANDLE GPUSwapChain::GetBackBufferRTV() const
{
    return GetBackBufferRTV(m_currentBackBufferIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE GPUSwapChain::GetBackBufferRTV(UINT index) const
{
    assert(index < BACK_BUFFER_COUNT);
    return m_rtvHandles[index];
}

void GPUSwapChain::CreateRenderTargetViews()
{
    assert(m_device);
    assert(m_rtvDescriptorHeap);

    UINT rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
    {
        HRESULT hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
        assert(SUCCEEDED(hr));

        m_rtvHandles[i] = rtvHandle;
        m_device->CreateRenderTargetView(m_backBuffers[i], nullptr, rtvHandle);

        rtvHandle.ptr += rtvDescriptorSize;
    }
}

void GPUSwapChain::CreateDepthStencilBuffer()
{
    assert(m_device);

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = m_width;
    depthDesc.Height = m_height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DEPTH_STENCIL_FORMAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DEPTH_STENCIL_FORMAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    HRESULT hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&m_depthStencilBuffer)
    );
    assert(SUCCEEDED(hr));
}

void GPUSwapChain::CreateDepthStencilView()
{
    assert(m_device);
    assert(m_dsvDescriptorHeap);
    assert(m_depthStencilBuffer);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DEPTH_STENCIL_FORMAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    m_depthStencilView = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_device->CreateDepthStencilView(m_depthStencilBuffer, &dsvDesc, m_depthStencilView);
}
