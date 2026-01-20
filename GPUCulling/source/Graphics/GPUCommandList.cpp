#include "stdafx.h"

#include "GPUCommandList.h"
#include "GPUCommandAllocatorPool.h"

GPUCommandList::~GPUCommandList()
{
    Release();
}

bool GPUCommandList::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, GPUCommandAllocatorPool* allocatorPool)
{
    if (!device || !allocatorPool)
    {
        return false;
    }

    m_device = device;
    m_type = type;
    m_allocatorPool = allocatorPool;

    // Request an allocator from the pool
    m_currentAllocator = m_allocatorPool->RequestAllocator();
    if (!m_currentAllocator)
    {
        return false;
    }

    // Create the command list
    HRESULT hr = m_device->CreateCommandList(
        0,
        m_type,
        m_currentAllocator,
        nullptr,
        IID_PPV_ARGS(&m_commandList)
    );

    if (FAILED(hr))
    {
        m_currentAllocator = nullptr;
        return false;
    }

    // Command lists are created in recording state
    m_isOpen = true;
    m_pendingBarrierCount = 0;

    return true;
}

void GPUCommandList::Release()
{
    FlushPendingBarriers();

    if (m_commandList)
    {
        m_commandList->Release();
        m_commandList = nullptr;
    }

    if (m_currentAllocator)
    {
        m_allocatorPool->DiscardAllocator(m_currentAllocator, 0);
        m_currentAllocator = nullptr;
    }

    m_device = nullptr;
    m_allocatorPool = nullptr;
    m_isOpen = false;
}

void GPUCommandList::Begin()
{
    if (m_isOpen)
    {
        return; // Already open
    }

    // Get a fresh allocator
    m_currentAllocator = m_allocatorPool->RequestAllocator();
    if (!m_currentAllocator)
    {
        return;
    }

    // Reset the allocator and command list
    m_currentAllocator->Reset();
    m_commandList->Reset(m_currentAllocator, nullptr);

    m_isOpen = true;
    m_pendingBarrierCount = 0;
}

void GPUCommandList::End()
{
    if (!m_isOpen)
    {
        return;
    }

    FlushPendingBarriers();

    HRESULT hr = m_commandList->Close();
    if (SUCCEEDED(hr))
    {
        m_isOpen = false;
    }
}

void GPUCommandList::Reset()
{
    FlushPendingBarriers();

    if (m_isOpen && m_currentAllocator)
    {
        m_currentAllocator->Reset();
        m_commandList->Reset(m_currentAllocator, nullptr);
        m_pendingBarrierCount = 0;
    }
}

void GPUCommandList::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    if (!resource || before == after || m_pendingBarrierCount >= MAX_PENDING_BARRIERS)
    {
        return;
    }

    D3D12_RESOURCE_BARRIER& barrier = m_pendingBarriers[m_pendingBarrierCount++];
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
}

void GPUCommandList::UAVBarrier(ID3D12Resource* resource)
{
    if (!resource || m_pendingBarrierCount >= MAX_PENDING_BARRIERS)
    {
        return;
    }

    D3D12_RESOURCE_BARRIER& barrier = m_pendingBarriers[m_pendingBarrierCount++];
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource = resource;
}

void GPUCommandList::AliasingBarrier(ID3D12Resource* resourceBefore, ID3D12Resource* resourceAfter)
{
    if (m_pendingBarrierCount >= MAX_PENDING_BARRIERS)
    {
        return;
    }

    D3D12_RESOURCE_BARRIER& barrier = m_pendingBarriers[m_pendingBarrierCount++];
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Aliasing.pResourceBefore = resourceBefore;
    barrier.Aliasing.pResourceAfter = resourceAfter;
}

void GPUCommandList::FlushResourceBarriers()
{
    FlushPendingBarriers();
}

void GPUCommandList::FlushPendingBarriers()
{
    if (m_pendingBarrierCount > 0 && m_commandList && m_isOpen)
    {
        m_commandList->ResourceBarrier((UINT)m_pendingBarrierCount, m_pendingBarriers);
        m_pendingBarrierCount = 0;
    }
}
