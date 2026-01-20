#pragma once

#include "Graphics/GPUDevice.h"
#include "Graphics/GPUCommandQueue.h"
#include "Graphics/GPUCommandList.h"
#include "Graphics/GPUDescriptorHeap.h"

class ImGuiLayer
{
private:
    static ImGuiLayer* s_Instance;

public:
    void Initialize(GPUDevice* device, GPUCommandQueue* commandQueue, int framesInFlight, UINT srvHeapSize = 64);
    void Release();
    void StartFrame();
    void EndFrame(GPUCommandList* list);

private:
    std::unique_ptr<GPUDescriptorHeap> m_srvDescHeap = nullptr;
};