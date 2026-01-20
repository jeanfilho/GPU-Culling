#pragma once

#include "GraphicsAPICommon.h"

class GPUCommandAllocatorPool
{
    GPUCommandAllocatorPool(const GPUCommandAllocatorPool&) = delete;
    GPUCommandAllocatorPool& operator=(const GPUCommandAllocatorPool&) = delete;

public:
    GPUCommandAllocatorPool() = default;
    ~GPUCommandAllocatorPool();

    bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, size_t initialSize = 8);
    void Release();
    ID3D12CommandAllocator* RequestAllocator();
    void DiscardAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue);
    void CleanupAllocators(uint64_t completedFenceValue);

private:
    struct AllocatorEntry
    {
        ID3D12CommandAllocator* allocator = nullptr;
        uint64_t fenceValue = 0;
    };
    ID3D12Device* m_device = nullptr;
    D3D12_COMMAND_LIST_TYPE m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    std::vector<ID3D12CommandAllocator*> m_availableAllocators;
    std::vector<AllocatorEntry> m_inUseAllocators;
};