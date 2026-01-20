#pragma once

#include "GraphicsAPICommon.h"

class GPUCommandAllocatorPool;
class GPUCommandQueue;

class GPUCommandList
{
    GPUCommandList(const GPUCommandList&) = delete;
    GPUCommandList& operator=(const GPUCommandList&) = delete;

public:
    GPUCommandList() = default;
    ~GPUCommandList();

    bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, GPUCommandAllocatorPool* allocatorPool);
    void Release();

    // Command list management
    void Begin();
    void End();
    void Reset();
    
    // Resource barriers
    void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
    void UAVBarrier(ID3D12Resource* resource);
    void AliasingBarrier(ID3D12Resource* resourceBefore, ID3D12Resource* resourceAfter);
    void FlushResourceBarriers();

    // Accessors
    ID3D12GraphicsCommandList* GetCommandList() { return m_commandList; }
    bool IsOpen() const { return m_isOpen; }

private:
    void FlushPendingBarriers();

    ID3D12Device* m_device = nullptr;
    ID3D12GraphicsCommandList* m_commandList = nullptr;
    ID3D12CommandAllocator* m_currentAllocator = nullptr;
    GPUCommandAllocatorPool* m_allocatorPool = nullptr;
    D3D12_COMMAND_LIST_TYPE m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    
    bool m_isOpen = false;

    static constexpr size_t MAX_PENDING_BARRIERS = 256;
    D3D12_RESOURCE_BARRIER m_pendingBarriers[MAX_PENDING_BARRIERS];
    size_t m_pendingBarrierCount = 0;
};