#pragma once

#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#undef CopyMemory

using Microsoft::WRL::ComPtr;

#define SWAP_CHAIN_BUFFER_COUNT 2

struct d3d12_descriptor
{
    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle;
};

struct d3d12_descriptor_heap
{
    ComPtr<ID3D12DescriptorHeap> Heap;

    u32 DescriptorsAllocated;
    u32 DescriptorsInHeap;
    u32 DescriptorSize;
};

struct d3d12_staging_buffer
{
    ComPtr<ID3D12Resource> Buffer;

    u32 LayoutCount;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT *Layouts;

    u32 FirstSubresource;
    u32 SubresourceCount;
};

struct d3d12_upload_buffer
{
    ComPtr<ID3D12Resource> Buffer;
    
    umm Size;
    umm Used;

    u8 *CPUAddress;
    D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;
};

struct d3d12_upload_buffer_region
{
    u8 *CPUAddress;
    D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;
    umm Size;
};

struct d3d12_constant_buffer_view
{
    d3d12_upload_buffer_region Data;
    d3d12_descriptor Descriptor;
};

struct d3d12_constant_buffer_transform
{
    mat4 ScreenProjection;
};

struct d3d12_constant_buffer_rectangle
{
    mat4 Model;
    vec4 Color;
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

    D3D_ROOT_SIGNATURE_VERSION RootSignatureVersion;

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

    d3d12_upload_buffer ConstantBuffer;

    d3d12_constant_buffer_view TransformCBV;

    // todo:
    u32 RectangleIndex;
    d3d12_constant_buffer_view RectanglesCBV[4];
    //

    ComPtr<ID3D12RootSignature> RectangleRootSignature;
    ComPtr<ID3D12PipelineState> RectanglePSO;
    ComPtr<ID3D12Resource> RectangleVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW RectangleVBV;
};
