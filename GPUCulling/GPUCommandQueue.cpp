#include "stdafx.h"
#include "GPUCommandQueue.h"

GPUCommandQueue::~GPUCommandQueue()
{
    Release();
}
bool GPUCommandQueue::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
{
    assertm(device != nullptr, "GPUCommandQueue::Initialize called with null device");
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;
    if (FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue))))
    {
        return false;
    }
    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
    {
        m_commandQueue->Release();
        m_commandQueue = nullptr;
        return false;
    }
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        m_fence->Release();
        m_fence = nullptr;
        m_commandQueue->Release();
        m_commandQueue = nullptr;
        return false;
    }
    m_currentFenceValue = 0;
    return true;
}

void GPUCommandQueue::Release()
{
    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
    if (m_fence)
    {
        m_fence->Release();
        m_fence = nullptr;
    }
    if (m_commandQueue)
    {
        m_commandQueue->Release();
        m_commandQueue = nullptr;
    }
}

uint64_t GPUCommandQueue::ExecuteCommandLists(UINT numCommandLists, ID3D12CommandList* const* commandLists)
{
    assertm(m_commandQueue != nullptr, "GPUCommandQueue::ExecuteCommandLists called on uninitialized command queue");
    m_commandQueue->ExecuteCommandLists(numCommandLists, commandLists);
    return Signal();
}

uint64_t GPUCommandQueue::Signal()
{
    assertm(m_commandQueue != nullptr && m_fence != nullptr, "GPUCommandQueue::Signal called on uninitialized command queue or fence");
    ++m_currentFenceValue;
    HRESULT hr = m_commandQueue->Signal(m_fence, m_currentFenceValue);
    assertm(SUCCEEDED(hr), "GPUCommandQueue::Signal failed to signal fence");
    return m_currentFenceValue;
}

void GPUCommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
    assertm(m_fence != nullptr && m_fenceEvent != nullptr, "GPUCommandQueue::WaitForFenceValue called on uninitialized fence or fence event");
    if (m_fence->GetCompletedValue() < fenceValue)
    {
        HRESULT hr = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
        assertm(SUCCEEDED(hr), "GPUCommandQueue::WaitForFenceValue failed to set event on fence completion");
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

bool GPUCommandQueue::IsFenceComplete(uint64_t fenceValue) const
{
    assertm(m_fence != nullptr, "GPUCommandQueue::IsFenceComplete called on uninitialized fence");
    return m_fence->GetCompletedValue() >= fenceValue;
}
