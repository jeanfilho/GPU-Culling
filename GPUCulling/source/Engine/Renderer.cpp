#include "stdafx.h"
#include "Engine/Renderer.h"

Renderer::~Renderer()
{
    Release();
}

bool Renderer::Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue)
{
    if (!device || !commandQueue)
    {
        return false;
    }

    m_device = device;
    m_commandQueue = commandQueue;

    // Create fence for frame synchronization
    HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr))
    {
        return false;
    }

    // Create fence event for waiting
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent)
    {
        m_fence->Release();
        m_fence = nullptr;
        return false;
    }

    m_currentFenceValue = 1;

    // Initialize triple buffered resources
    for (UINT i = 0; i < FRAME_COUNT; ++i)
    {
        // Create command allocator pool for this frame
        m_commandAllocatorPools[i] = std::make_unique<GPUCommandAllocatorPool>();
        if (!m_commandAllocatorPools[i]->Initialize(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, 4))
        {
            return false;
        }

        // Create command list for this frame
        m_commandLists[i] = std::make_unique<GPUCommandList>();
        if (!m_commandLists[i]->Initialize(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocatorPools[i].get()))
        {
            return false;
        }

        // Create descriptor heap for this frame
        m_descriptorHeaps[i] = std::make_unique<GPUDescriptorHeap>();
        if (!m_descriptorHeaps[i]->Initialize(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, true))
        {
            return false;
        }

        m_fenceValues[i] = 0;
    }

    // Initialize compute resources for clustering
    InitializeComputeResources();

    m_isInitialized = true;
    return true;
}

void Renderer::Release()
{
    // Wait for all frames to complete before releasing
    for (UINT i = 0; i < FRAME_COUNT; ++i)
    {
        WaitForFrameCompletion(i);
    }

    // Release triple buffered resources (unique_ptr handles cleanup automatically)
    for (UINT i = 0; i < FRAME_COUNT; ++i)
    {
        m_commandLists[i].reset();
        m_commandAllocatorPools[i].reset();
        m_descriptorHeaps[i].reset();
    }

    // Release synchronization objects
    if (m_fence)
    {
        m_fence->Release();
        m_fence = nullptr;
    }

    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }

    m_device = nullptr;
    m_commandQueue = nullptr;
    m_isInitialized = false;
}

void Renderer::BeginFrame()
{
    assert(m_isInitialized);

    // Wait for the current frame to complete if necessary
    WaitForFrameCompletion(m_currentFrameIndex);

    // Get the current frame's resources
    GPUCommandList* commandList = GetCurrentCommandList();
    assert(commandList);

    // Start a new command list for this frame
    commandList->Begin();

    // Set default viewport and scissor rect
    commandList->GetCommandList()->RSSetViewports(1, &m_currentViewport);
    commandList->GetCommandList()->RSSetScissorRects(1, &m_currentScissorRect);

    // Set the descriptor heap for this frame
    ID3D12DescriptorHeap* heaps[] = { m_descriptorHeaps[m_currentFrameIndex]->GetHeap() };
    if (heaps[0])
    {
        commandList->GetCommandList()->SetDescriptorHeaps(1, heaps);
    }

    // Clear render target and depth stencil
    ClearRenderTarget();
}

void Renderer::ClearRenderTarget()
{
    GPUCommandList* commandList = GetCurrentCommandList();
    assert(commandList);
    assert(commandList->GetCommandList());

    // Clear render target view
    commandList->GetCommandList()->ClearRenderTargetView(m_currentRTV, m_clearColor, 0, nullptr);

    // Clear depth stencil view
    commandList->GetCommandList()->ClearDepthStencilView(
        m_currentDSV,
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f,
        0,
        0,
        nullptr
    );
}

void Renderer::Render()
{
    // Render the clustered Forward+ outline in stages
    RenderClustering();
    RenderClusterDebugOutlines();
    RenderDebugVisualization();
}

void Renderer::EndFrame()
{
    assert(m_isInitialized);

    GPUCommandList* commandList = GetCurrentCommandList();
    assert(commandList);

    // Close the command list
    commandList->End();

    // Execute the command list
    ID3D12CommandList* commandLists[] = { commandList->GetCommandList() };
    m_commandQueue->ExecuteCommandLists(1, commandLists);

    // Signal fence for this frame
    HRESULT hr = m_commandQueue->Signal(m_fence, m_currentFenceValue);
    assert(SUCCEEDED(hr));

    m_fenceValues[m_currentFrameIndex] = m_currentFenceValue;
    m_currentFenceValue++;

    // Advance to next frame
    m_currentFrameIndex = (m_currentFrameIndex + 1) % FRAME_COUNT;
}

void Renderer::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
{
    m_currentRTV = rtvHandle;
    m_currentDSV = dsvHandle;

    GPUCommandList* commandList = GetCurrentCommandList();
    assert(commandList);
    assert(commandList->GetCommandList());

    commandList->GetCommandList()->OMSetRenderTargets(1, &m_currentRTV, FALSE, &m_currentDSV);
}

void Renderer::SetViewport(float width, float height)
{
    m_currentViewport.TopLeftX = 0.0f;
    m_currentViewport.TopLeftY = 0.0f;
    m_currentViewport.Width = width;
    m_currentViewport.Height = height;
    m_currentViewport.MinDepth = 0.0f;
    m_currentViewport.MaxDepth = 1.0f;

    m_currentScissorRect.left = 0;
    m_currentScissorRect.top = 0;
    m_currentScissorRect.right = static_cast<LONG>(width);
    m_currentScissorRect.bottom = static_cast<LONG>(height);
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
    m_clearColor[0] = r;
    m_clearColor[1] = g;
    m_clearColor[2] = b;
    m_clearColor[3] = a;
}

void Renderer::WaitForFrameCompletion(UINT frameIndex)
{
    assert(m_fence);
    assert(m_fenceEvent);

    uint64_t fenceValue = m_fenceValues[frameIndex];
    if (m_fence->GetCompletedValue() < fenceValue)
    {
        HRESULT hr = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
        assert(SUCCEEDED(hr));

        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void Renderer::InitializeComputeResources()
{
    // TODO: Initialize cluster compute shaders and resources
    // This would include:
    // - Cluster grid buffer
    // - Light culling compute shader
    // - Debug visualization resources
}

void Renderer::RenderClustering()
{
    assert(m_commandLists[m_currentFrameIndex]);

    // TODO: Execute light clustering compute shader
    // This would:
    // 1. Set up compute shader
    // 2. Set compute buffers
    // 3. Dispatch compute work
    // 4. Insert barriers between compute and graphics
}

void Renderer::RenderClusterDebugOutlines()
{
    assert(m_commandLists[m_currentFrameIndex]);

    // TODO: Render cluster boundaries/outlines
    // This would:
    // 1. Set up debug visualization graphics pipeline
    // 2. Draw cluster grid lines/boxes
    // 3. Draw per-cluster statistics if needed
}

void Renderer::RenderDebugVisualization()
{
    assert(m_commandLists[m_currentFrameIndex]);

    // TODO: Render additional debug information
    // This could include:
    // - Heat maps showing light counts per cluster
    // - Cluster density visualization
    // - Performance metrics overlay
}
