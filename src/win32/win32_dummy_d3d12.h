#pragma once

#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#define SWAP_CHAIN_BUFFER_COUNT 2

struct d3d12_descriptor_heap
{
    ComPtr<ID3D12DescriptorHeap> Heap;

    //u32 DescriptorsAllocated;
    //u32 DescriptorsInHeap;
    u32 DescriptorSize;
};

struct d3d12_state
{
    stream *Stream;
    memory_arena *Arena;
    platform_api *Platform;
    platform_profiler *Profiler;

    DXGI_ADAPTER_DESC1 AdapterDesc;
    
    ComPtr<IDXGIFactory4> DXGIFactory;
    ComPtr<ID3D12Device> Device;

    ComPtr<ID3D12Fence> Fence;
    HANDLE FenceCompletionEvent;
    u64 FenceValues[SWAP_CHAIN_BUFFER_COUNT];

    ComPtr<ID3D12CommandQueue> CommandQueue;
    ComPtr<ID3D12CommandAllocator> CommandAllocators[SWAP_CHAIN_BUFFER_COUNT];
    ComPtr<ID3D12GraphicsCommandList> CommandList;

    u32 CurrentBackBufferIndex;
    ComPtr<IDXGISwapChain3> SwapChain;
    ComPtr<ID3D12Resource> SwapChainBuffers[SWAP_CHAIN_BUFFER_COUNT];

    d3d12_descriptor_heap DescriptorHeapRTV;
    d3d12_descriptor_heap DescriptorHeapDSV;
    d3d12_descriptor_heap DescriptorHeapCBV_SRV_UAV;

    ComPtr<ID3D12Resource> DepthStencilTexture;
};
