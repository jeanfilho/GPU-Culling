#pragma once

#include "GraphicsAPICommon.h"

class GPUDescriptorHeap
{
    GPUDescriptorHeap(const GPUDescriptorHeap&) = delete;
    GPUDescriptorHeap& operator=(const GPUDescriptorHeap&) = delete;

public:
    GPUDescriptorHeap() = default;
    ~GPUDescriptorHeap();

    bool Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorCount, bool shaderVisible = false);
    void Release();

    // Descriptor allocation
    void AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle);
    void FreeDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
    
    // Heap information
    ID3D12DescriptorHeap* GetHeap() const { return m_heap; }
    UINT GetDescriptorCount() const { return m_descriptorCount; }
    UINT GetAvailableDescriptors() const { return m_descriptorCount - m_allocatedCount; }
    UINT GetAllocatedCount() const { return m_allocatedCount; }
    bool IsShaderVisible() const { return m_shaderVisible; }
    UINT GetIncrementSize() const { return m_incrementSize; }

    // Reset allocation counter (should be done after GPU work completes)
    void Reset() { m_allocatedCount = 0; }

private:
    ID3D12Device* m_device = nullptr;
    ID3D12DescriptorHeap* m_heap = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHeapStart = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHeapStart = {};
    D3D12_DESCRIPTOR_HEAP_TYPE m_type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    UINT m_descriptorCount = 0;
    UINT m_allocatedCount = 0;
    UINT m_incrementSize = 0;
    bool m_shaderVisible = false;
    std::vector<UINT> m_freeIndices;
};


