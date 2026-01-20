#include "stdafx.h"
#include "GPUDescriptorHeap.h"

GPUDescriptorHeap::~GPUDescriptorHeap()
{
    Release();
}

bool GPUDescriptorHeap::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorCount, bool shaderVisible)
{
    if (!device || descriptorCount == 0)
    {
        return false;
    }

    m_device = device;
    m_type = type;
    m_descriptorCount = descriptorCount;
    m_shaderVisible = shaderVisible;

    // Get the descriptor increment size for this heap type
    m_incrementSize = m_device->GetDescriptorHandleIncrementSize(m_type);

    // Create the descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = m_type;
    heapDesc.NumDescriptors = m_descriptorCount;
    heapDesc.Flags = m_shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NodeMask = 0;

    HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap));
    if (FAILED(hr))
    {
        return false;
    }

    // Get the starting handles for both CPU and GPU access
    m_cpuHeapStart = m_heap->GetCPUDescriptorHandleForHeapStart();
    
    if (m_shaderVisible)
    {
        m_gpuHeapStart = m_heap->GetGPUDescriptorHandleForHeapStart();
    }

    m_allocatedCount = 0;

    return true;
}

void GPUDescriptorHeap::Release()
{
    if (m_heap)
    {
        m_heap->Release();
        m_heap = nullptr;
    }

    m_device = nullptr;
    m_descriptorCount = 0;
    m_allocatedCount = 0;
    m_incrementSize = 0;
    m_cpuHeapStart = {};
    m_gpuHeapStart = {};
    m_shaderVisible = false;
}

void GPUDescriptorHeap::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
{
    assert(m_heap && outCpuHandle && outGpuHandle);

    D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
    if (m_allocatedCount >= m_descriptorCount)
    {
        assertm(false, "Not enough descriptors available in heap to allocate.");
        return;
    }

    UINT idx = m_freeIndices.back();
    m_freeIndices.pop_back();

    outCpuHandle->ptr = m_cpuHeapStart.ptr + (static_cast<SIZE_T>(idx) * m_incrementSize);
    outGpuHandle->ptr = m_gpuHeapStart.ptr + (static_cast<SIZE_T>(idx) * m_incrementSize);
    m_allocatedCount++;
}

void GPUDescriptorHeap::FreeDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
{
    assert(m_heap);
    SIZE_T cpuOffset = cpuHandle.ptr - m_cpuHeapStart.ptr;
    SIZE_T gpuOffset = gpuHandle.ptr - m_gpuHeapStart.ptr;

    assert(cpuOffset == gpuOffset);
    UINT index = static_cast<UINT>(cpuOffset / m_incrementSize);
    m_freeIndices.push_back(index);
    m_allocatedCount--;
}
