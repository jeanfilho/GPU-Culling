#pragma once

class GPUCommandQueue
{
    GPUCommandQueue(const GPUCommandQueue&) = delete;
    GPUCommandQueue& operator=(const GPUCommandQueue&) = delete;

public:
    GPUCommandQueue() = default;
    ~GPUCommandQueue();
    bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
    void Release();
    ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue; }
    uint64_t ExecuteCommandLists(UINT numCommandLists, ID3D12CommandList* const* commandLists);
    uint64_t Signal();
    void WaitForFenceValue(uint64_t fenceValue);
    bool IsFenceComplete(uint64_t fenceValue) const;

private:
    ID3D12CommandQueue* m_commandQueue = nullptr;
    ID3D12Fence* m_fence = nullptr;
    HANDLE m_fenceEvent = nullptr;
    uint64_t m_currentFenceValue = 0;
};