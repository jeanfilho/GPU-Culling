#pragma once

#include "GraphicsAPICommon.h"

class GPUDevice
{
    GPUDevice& operator=(const GPUDevice&) = delete;
    GPUDevice(const GPUDevice&) = delete;

public:
    ~GPUDevice();
    GPUDevice() = default;
    GPUDevice(GPUDevice&& other) = default;
    GPUDevice& operator=(GPUDevice&& other) = default;

    bool Initialize();
    void Release();
    ID3D12Device* GetDevice() const { return m_device; }
    IDXGIAdapter* GetAdapter() const { return m_adapter; }

private:
    ID3D12Device* m_device = nullptr;
    IDXGIAdapter* m_adapter = nullptr;


};

