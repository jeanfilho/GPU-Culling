#include "stdafx.h"

#include "GPUDevice.h"

#include "dxgi1_6.h"
#include "GPUCommandAllocatorPool.h"

GPUDevice::~GPUDevice()
{
    Release();
}

bool GPUDevice::Initialize()
{
    // Create DXGI Factory
    IDXGIFactory7* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    {
        return false;
    }
    // Enumerate adapters
    IDXGIAdapter1* adapter = nullptr;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        // Check for a suitable adapter (e.g., dedicated GPU)
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            adapter->Release();
            continue;
        }
        // Try to create the D3D12 device
        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_device))))
        {
            m_adapter = adapter;
            break;
        }
        adapter->Release();
    }
    factory->Release();
    return m_device != nullptr;
}

void GPUDevice::Release()
{
    if (m_device)
    {
        m_device->Release();
        m_device = nullptr;
    }
    if (m_adapter)
    {
        m_adapter->Release();
        m_adapter = nullptr;
    }
}
