#pragma once

#include "GraphicsAPICommon.h"

class GPUSwapChain
{
    GPUSwapChain(const GPUSwapChain&) = delete;
    GPUSwapChain& operator=(const GPUSwapChain&) = delete;

public:
    GPUSwapChain() = default;
    ~GPUSwapChain();

    bool Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, HWND hwnd, UINT width, UINT height);
    void Release();

    // Presentation
    void Present();
    void Resize(UINT width, UINT height);

    // Back buffer access
    ID3D12Resource* GetBackBuffer() const;
    ID3D12Resource* GetBackBuffer(UINT index) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRTV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRTV(UINT index) const;

    // Depth stencil
    ID3D12Resource* GetDepthStencilBuffer() const { return m_depthStencilBuffer; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return m_depthStencilView; }

    // Properties
    UINT GetBackBufferCount() const { return BACK_BUFFER_COUNT; }
    UINT GetCurrentBackBufferIndex() const { return m_currentBackBufferIndex; }
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }

    // Descriptor heap
    ID3D12DescriptorHeap* GetRTVDescriptorHeap() const { return m_rtvDescriptorHeap; }
    ID3D12DescriptorHeap* GetDSVDescriptorHeap() const { return m_dsvDescriptorHeap; }

private:
    void CreateDepthStencilBuffer();
    void CreateRenderTargetViews();
    void CreateDepthStencilView();

    static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
    static constexpr DXGI_FORMAT DEPTH_STENCIL_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;
    static constexpr UINT BACK_BUFFER_COUNT = 3;

    ID3D12Device* m_device = nullptr;
    ID3D12CommandQueue* m_commandQueue = nullptr;
    IDXGISwapChain3* m_swapChain = nullptr;
    IDXGIFactory4* m_dxgiFactory = nullptr;

    // Render target resources
    ID3D12Resource* m_backBuffers[BACK_BUFFER_COUNT] = {};
    ID3D12DescriptorHeap* m_rtvDescriptorHeap = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[BACK_BUFFER_COUNT] = {};
    UINT m_currentBackBufferIndex = 0;

    // Depth stencil resources
    ID3D12Resource* m_depthStencilBuffer = nullptr;
    ID3D12DescriptorHeap* m_dsvDescriptorHeap = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_depthStencilView = {};

    UINT m_width = 0;
    UINT m_height = 0;
    bool m_isInitialized = false;
};
