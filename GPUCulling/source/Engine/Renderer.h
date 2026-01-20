#pragma once

#include "Graphics/GPUDevice.h"
#include "Graphics/GPUCommandQueue.h"
#include "Graphics/GPUCommandList.h"
#include "Graphics/GPUCommandAllocatorPool.h"
#include "Graphics/GPUDescriptorHeap.h"
#include <DirectXMath.h>
#include <memory>
#include <array>

using namespace DirectX;

static constexpr UINT FRAME_COUNT = 3;
static constexpr float CLEAR_COLOR[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

class Renderer
{
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

public:
    Renderer() = default;
    ~Renderer();

    bool Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue);
    void Release();

    // Rendering interface
    void BeginFrame();
    void ClearRenderTarget();
    void Render();
    void EndFrame();

    // Render pass setup
    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);
    void SetViewport(float width, float height);
    void SetClearColor(float r, float g, float b, float a);

    // Accessors
    GPUCommandList* GetCurrentCommandList() const { return m_commandLists[m_currentFrameIndex].get(); }
    GPUDescriptorHeap* GetDescriptorHeap(UINT frameIndex) const { return m_descriptorHeaps[frameIndex].get(); }
    UINT GetCurrentFrameIndex() const { return m_currentFrameIndex; }

private:
    void InitializeComputeResources();
    void RenderClustering();
    void RenderClusterDebugOutlines();
    void RenderDebugVisualization();
    void WaitForFrameCompletion(UINT frameIndex);

    // Core GPU resources (triple buffered)
    ID3D12Device* m_device = nullptr;
    ID3D12CommandQueue* m_commandQueue = nullptr;
    ID3D12Fence* m_fence = nullptr;
    HANDLE m_fenceEvent = nullptr;

    std::array<std::unique_ptr<GPUCommandList>, FRAME_COUNT> m_commandLists;
    std::array<std::unique_ptr<GPUCommandAllocatorPool>, FRAME_COUNT> m_commandAllocatorPools;
    std::array<std::unique_ptr<GPUDescriptorHeap>, FRAME_COUNT> m_descriptorHeaps;

    // Frame synchronization
    uint64_t m_fenceValues[FRAME_COUNT] = {};
    uint64_t m_currentFenceValue = 0;
    UINT m_currentFrameIndex = 0;

    // Rendering state
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentRTV = {};
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentDSV = {};
    D3D12_VIEWPORT m_currentViewport = {};
    D3D12_RECT m_currentScissorRect = {};
    float m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    bool m_isInitialized = false;
};
