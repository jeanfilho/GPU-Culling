#include "stdafx.h"

#include "GPUCommandAllocatorPool.h"


GPUCommandAllocatorPool::~GPUCommandAllocatorPool()
{
    assertm(m_inUseAllocators.empty(), "GPUCommandAllocatorPool destroy with command allocators in flight");
    Release();
}

bool GPUCommandAllocatorPool::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, size_t initialSize)
{
    assertm(device != nullptr, "GPUCommandAllocatorPool::Initialize called with null device");

    m_device = device;
    m_type = type;
    m_availableAllocators.reserve(initialSize);
    for (size_t i = 0; i < initialSize; ++i)
    {
        ID3D12CommandAllocator* allocator = nullptr;
        if (FAILED(m_device->CreateCommandAllocator(m_type, IID_PPV_ARGS(&allocator))))
        {
            return false;
        }
        m_availableAllocators.push_back(allocator);
    }
    return true;
}

void GPUCommandAllocatorPool::Release()
{
    for (ID3D12CommandAllocator* allocator : m_availableAllocators)
    {
        if (allocator)
        {
            allocator->Release();
        }
    }
    m_availableAllocators.clear();

    m_device = nullptr;
}

ID3D12CommandAllocator* GPUCommandAllocatorPool::RequestAllocator()
{
    ID3D12CommandAllocator* allocator = nullptr;
    if (!m_availableAllocators.empty())
    {
        allocator = m_availableAllocators.back();
        m_availableAllocators.pop_back();
    }
    else
    {
        if (FAILED(m_device->CreateCommandAllocator(m_type, IID_PPV_ARGS(&allocator))))
        {
            return nullptr;
        }
    }
    return allocator;
}

void GPUCommandAllocatorPool::DiscardAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue)
{
    assertm(allocator != nullptr, "GPUCommandAllocatorPool::DiscardAllocator called with null allocator");
    m_inUseAllocators.push_back({ allocator, fenceValue });
}

void GPUCommandAllocatorPool::CleanupAllocators(uint64_t completedFenceValue)
{
    auto it = m_inUseAllocators.begin();
    while (it != m_inUseAllocators.end())
    {
        if (it->fenceValue <= completedFenceValue)
        {
            it->allocator->Reset();
            m_availableAllocators.push_back(it->allocator);
            it = m_inUseAllocators.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
