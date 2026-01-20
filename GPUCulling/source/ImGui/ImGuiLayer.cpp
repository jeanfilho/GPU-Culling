#include "stdafx.h"
#include "ImGuiLayer.h"

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

ImGuiLayer* ImGuiLayer::s_Instance = nullptr;

void ImGuiLayer::Initialize(GPUDevice* device, GPUCommandQueue* commandQueue, int framesInFlight, UINT srvHeapSize)
{
    assertm(s_Instance == nullptr, "ImGuiLayer::Initialize called more than once!");
    s_Instance = this;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = device->GetDevice();
    init_info.CommandQueue = commandQueue->GetCommandQueue();
    init_info.NumFramesInFlight = framesInFlight;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // Or your render target format.

    // Create ImGui SRV Descriptor Heap
    m_srvDescHeap = std::make_unique<GPUDescriptorHeap>();
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = srvHeapSize;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        auto descHeap = m_srvDescHeap->GetHeap();
        assert(device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap)) != S_OK);
    }

    // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
    // The example_win32_directx12/main.cpp application include a simple free-list based allocator.
    init_info.SrvDescriptorHeap = m_srvDescHeap->GetHeap();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle) { return s_Instance->m_srvDescHeap->AllocateDescriptor(outCpuHandle, outGpuHandle); };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { return s_Instance->m_srvDescHeap->FreeDescriptor(cpuHandle, gpuHandle); };
}

void ImGuiLayer::Release()
{
    s_Instance = nullptr;
    m_srvDescHeap.reset();
}

void ImGuiLayer::StartFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //TODO remove test
    ImGui::ShowDemoWindow(); // Show demo window! :)
}

void ImGuiLayer::EndFrame(GPUCommandList* commandList)
{
    // Rendering    
    ImGui::Render();
    ID3D12DescriptorHeap* ppHeaps[] = { m_srvDescHeap->GetHeap() };
    commandList->GetCommandList()->SetDescriptorHeaps(1, ppHeaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList->GetCommandList());
}
