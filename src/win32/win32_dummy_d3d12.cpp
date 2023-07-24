#include "dummy.h"
#include "win32_dummy.h"
#include "win32_dummy_d3d12.h"

dummy_internal ComPtr<ID3DBlob>
Direct3D12CompileShaderFromFile(d3d12_state *State, const wchar *FileName, const char *Entry, const char *Target)
{
    u32 Flags = D3DCOMPILE_ENABLE_STRICTNESS;

#if DEBUG
    Flags |= D3DCOMPILE_DEBUG;
    Flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> Shader;
    ComPtr<ID3DBlob> Errors;

    D3DCompileFromFile(FileName, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, Entry, Target, Flags, 0, &Shader, &Errors);

    if (Errors)
    {
        char *ErrorMessages = (char *) Errors->GetBufferPointer();
        Assert(!ErrorMessages);
    }

    return Shader;
}

dummy_internal ComPtr<ID3D12RootSignature>
Direct3D12CreateRootSignature(d3d12_state *State, D3D12_VERSIONED_ROOT_SIGNATURE_DESC SignatureDesc)
{
    D3D12_ROOT_SIGNATURE_FLAGS StandardFlags =
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    switch (SignatureDesc.Version)
    {
        case D3D_ROOT_SIGNATURE_VERSION_1_0:
        {
            SignatureDesc.Desc_1_0.Flags |= StandardFlags;
            break;
        }
        case D3D_ROOT_SIGNATURE_VERSION_1_1:
        {
            SignatureDesc.Desc_1_1.Flags |= StandardFlags;
            break;
        }
        case D3D_ROOT_SIGNATURE_VERSION_1_2:
        {
            SignatureDesc.Desc_1_2.Flags |= StandardFlags;
            break;
        }
    }

    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3DBlob> SignatureBlob;
    ComPtr<ID3DBlob> Errors;

    D3DX12SerializeVersionedRootSignature(&SignatureDesc, State->RootSignatureVersion, &SignatureBlob, &Errors);

    if (Errors)
    {
        char *ErrorMessages = (char *)Errors->GetBufferPointer();
        Assert(!ErrorMessages);
    }

    State->Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature));

    return RootSignature;
}

dummy_internal d3d12_descriptor_heap
Direct3D12CreateDescriptorHeap(d3d12_state *State, D3D12_DESCRIPTOR_HEAP_DESC HeapDesc)
{
    d3d12_descriptor_heap Result;

    State->Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Result.Heap));

    Result.DescriptorsAllocated = 0;
    Result.DescriptorsInHeap = HeapDesc.NumDescriptors;
    Result.DescriptorSize = State->Device->GetDescriptorHandleIncrementSize(HeapDesc.Type);

    return Result;
}

dummy_internal d3d12_staging_buffer
Direct3D12CreateStagingBuffer(d3d12_state *State, ComPtr<ID3D12Resource> Resource, u32 FirstSubresource, u32 SubresourceCount, D3D12_SUBRESOURCE_DATA *Data)
{
    d3d12_staging_buffer Result;

    Result.FirstSubresource = FirstSubresource;
    Result.SubresourceCount = SubresourceCount;
    Result.LayoutCount = SubresourceCount;
    Result.Layouts = PushArray(State->Arena, Result.LayoutCount, D3D12_PLACED_SUBRESOURCE_FOOTPRINT);

    D3D12_RESOURCE_DESC ResourceDesc = Resource->GetDesc();

    scoped_memory ScopedMemory(State->Arena);

    u64 TotalBytes;
    u32 *RowCount = PushArray(ScopedMemory.Arena, SubresourceCount, u32);
    u64 *RowSizeInBytes = PushArray(ScopedMemory.Arena, SubresourceCount, u64);

    State->Device->GetCopyableFootprints(&ResourceDesc, FirstSubresource, SubresourceCount, 0, Result.Layouts, RowCount, RowSizeInBytes, &TotalBytes);

    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC ResourceDesc_ = CD3DX12_RESOURCE_DESC::Buffer(TotalBytes);

    State->Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDesc_,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        0,
        IID_PPV_ARGS(&Result.Buffer)
    );

    if (Data)
    {
        void *BufferMemory;
        CD3DX12_RANGE ReadRange{ 0, 0 };
        Result.Buffer->Map(0, &ReadRange, &BufferMemory);

        for (u32 SubresourceIndex = 0; SubresourceIndex < SubresourceCount; ++SubresourceIndex)
        {
            u8 *SubresourceMemory = (u8 *) BufferMemory + Result.Layouts[SubresourceIndex].Offset;

            if (ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                // Generic buffer: copy everything in one go
                CopyMemory((void*) Data->pData, SubresourceMemory, TotalBytes);
            }
            else
            {
                // Texture: copy data line by line
                Assert(!"Not implemented");
            }
        }

        Result.Buffer->Unmap(0, 0);
    }

    return Result;
}

dummy_internal d3d12_upload_buffer
Direct3D12CreateUploadBuffer(d3d12_state *State, u32 Size)
{
    d3d12_upload_buffer Result;

    Result.Used = 0;
    Result.Size = Size;

    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(Size);
    State->Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        0,
        IID_PPV_ARGS(&Result.Buffer)
    );

    CD3DX12_RANGE ReadRange{ 0, 0 };
    Result.Buffer->Map(0, &ReadRange, (void **) &Result.CPUAddress);
    Result.GPUAddress = Result.Buffer->GetGPUVirtualAddress();

    return Result;
}

// Constant buffers must be a multiple of the minimum hardware allocation size (usually 256 bytes)
inline umm
Direct3D12CalculateConstantBufferByteSize(umm Size)
{
    umm Result = (Size + 255) & ~255;
    return Result;
}

dummy_internal d3d12_upload_buffer_region
Direct3D12AllocateFromUploadBuffer(d3d12_state *State, d3d12_upload_buffer *Buffer, umm Size)
{
    umm Address = (umm) Buffer->CPUAddress + Buffer->Used;

    Assert(Buffer->Used + Size < Buffer->Size);

    d3d12_upload_buffer_region Result;

    Result.CPUAddress = (u8 *)Address;
    Result.GPUAddress = Buffer->GPUAddress + Buffer->Used;
    Result.Size = Size;

    Buffer->Used += Size;

    return Result;
}

inline d3d12_descriptor
Direct3D12AllocateFromDescriptorHeap(d3d12_descriptor_heap *DescriptorHeap)
{
    u32 Index = DescriptorHeap->DescriptorsAllocated++;

    Assert(Index < DescriptorHeap->DescriptorsInHeap);

    d3d12_descriptor Result;

    Result.CPUHandle = D3D12_CPU_DESCRIPTOR_HANDLE{ DescriptorHeap->Heap->GetCPUDescriptorHandleForHeapStart().ptr + Index * DescriptorHeap->DescriptorSize };
    Result.GPUHandle = D3D12_GPU_DESCRIPTOR_HANDLE{ DescriptorHeap->Heap->GetGPUDescriptorHandleForHeapStart().ptr + Index * DescriptorHeap->DescriptorSize };

    return Result;
}

dummy_internal d3d12_constant_buffer_view
Direct3D12CreateConstantBufferView(d3d12_state *State, void *Data, umm Size)
{
    d3d12_constant_buffer_view Result;

    umm AlignedSize = Direct3D12CalculateConstantBufferByteSize(Size);

    Result.Data = Direct3D12AllocateFromUploadBuffer(State, &State->ConstantBuffer, AlignedSize);
    Result.Descriptor = Direct3D12AllocateFromDescriptorHeap(&State->DescriptorHeapCBV_SRV_UAV);

    if (Data)
    {
        CopyMemory(Data, Result.Data.CPUAddress, Size);
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC Desc = {};
    Desc.BufferLocation = Result.Data.GPUAddress;
    Desc.SizeInBytes = (u32) Result.Data.Size;

    State->Device->CreateConstantBufferView(&Desc, Result.Descriptor.CPUHandle);

    return Result;
}

dummy_internal d3d12_framebuffer
Direct3D12CreateFramebuffer(d3d12_state *State, u32 Width, u32 Height, u32 Samples, DXGI_FORMAT ColorFormat, DXGI_FORMAT DepthStencilFormat)
{
    Assert(ColorFormat != DXGI_FORMAT_UNKNOWN);
    Assert(DepthStencilFormat != DXGI_FORMAT_UNKNOWN);

    d3d12_framebuffer Result = {};

    Result.Width = Width;
    Result.Height = Height;
    Result.Samples = Samples;

    // Color
    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    Desc.Width = Width;
    Desc.Height = Height;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.SampleDesc.Count = Samples;
    Desc.Format = ColorFormat;
    Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    float ClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_CLEAR_VALUE ColorClearValue = CD3DX12_CLEAR_VALUE(ColorFormat, ClearColor);

    State->Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &Desc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &ColorClearValue,
        IID_PPV_ARGS(&Result.ColorTexture)
    );
    
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = Desc.Format;
    rtvDesc.ViewDimension = (Samples > 1) ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;

    Result.rtv = Direct3D12AllocateFromDescriptorHeap(&State->DescriptorHeapRTV);
    State->Device->CreateRenderTargetView(Result.ColorTexture.Get(), &rtvDesc, Result.rtv.CPUHandle);

    // Depth/Stencil
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = Desc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    Result.srv = Direct3D12AllocateFromDescriptorHeap(&State->DescriptorHeapCBV_SRV_UAV);
    State->Device->CreateShaderResourceView(Result.ColorTexture.Get(), &srvDesc, Result.srv.CPUHandle);

    Desc.Format = DepthStencilFormat;
    Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

    CD3DX12_CLEAR_VALUE DepthStencilClearValue = CD3DX12_CLEAR_VALUE(DepthStencilFormat, 1.f, 0);

    State->Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &Desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &DepthStencilClearValue,
        IID_PPV_ARGS(&Result.DepthStencilTexture)
    );

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = Desc.Format;
    dsvDesc.ViewDimension = (Samples > 1) ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;

    Result.dsv = Direct3D12AllocateFromDescriptorHeap(&State->DescriptorHeapDSV);
    State->Device->CreateDepthStencilView(Result.DepthStencilTexture.Get(), &dsvDesc, Result.dsv.CPUHandle);

    return Result;
}

inline ID3D12Resource *
Direct3D12GetCurrentBackBuffer(d3d12_state *State)
{
    ID3D12Resource *Result = State->SwapChainBuffers[State->CurrentBackBufferIndex].Get();
    return Result;
}

inline ID3D12CommandAllocator *
Direct3D12GetCurrentCommmandAllocator(d3d12_state *State)
{
    ID3D12CommandAllocator *Result = State->CommandAllocators[State->CurrentBackBufferIndex].Get();
    return Result;
}

inline D3D12_CPU_DESCRIPTOR_HANDLE
Direct3D12GetCurrentBackBufferView(d3d12_state *State)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE Result = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        State->DescriptorHeapRTV.Heap->GetCPUDescriptorHandleForHeapStart(),
        State->CurrentBackBufferIndex,
        State->DescriptorHeapRTV.DescriptorSize
    );

    return Result;
}

inline void
Direct3D12ExecuteCommandList(d3d12_state *State, bool32 Reset = false)
{
    State->CommandList->Close();

    ID3D12CommandList *CommandLists[] = { State->CommandList.Get() };
    State->CommandQueue->ExecuteCommandLists(1, CommandLists);

    if (Reset)
    {
        ID3D12CommandAllocator *CommandAllocator = Direct3D12GetCurrentCommmandAllocator(State);
        State->CommandList->Reset(CommandAllocator, 0);
    }
}

inline void
Direct3D12FlushCommandQueue(d3d12_state *State)
{
    u64 *FenceValue = &State->FenceValues[State->CurrentBackBufferIndex];
    *FenceValue = *FenceValue + 1;

    State->CommandQueue->Signal(State->Fence.Get(), *FenceValue);
    State->Fence->SetEventOnCompletion(*FenceValue, State->FenceCompletionEvent);
    WaitForSingleObject(State->FenceCompletionEvent, INFINITE);
}

dummy_internal void
Direct3D12InitRectangle(d3d12_state *State)
{
    // Create root signature & pipeline configuration for rendering rectangle
    ComPtr<ID3DBlob> VertexShader = Direct3D12CompileShaderFromFile(State, L"shaders\\hlsl\\simple.hlsl", "VS", "vs_5_0");
    ComPtr<ID3DBlob> PixelShader = Direct3D12CompileShaderFromFile(State, L"shaders\\hlsl\\simple.hlsl", "PS", "ps_5_0");

    D3D12_INPUT_ELEMENT_DESC VertexLayout[] =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };

    const CD3DX12_DESCRIPTOR_RANGE1 DescriptorRanges[] =
    {
        {
            D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC
        },
        {
            D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC
        },
    };

    CD3DX12_ROOT_PARAMETER1 RootParameters[2];
    RootParameters[0].InitAsDescriptorTable(1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_VERTEX);
    RootParameters[1].InitAsDescriptorTable(1, &DescriptorRanges[1], D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC SignatureDesc(ArrayCount(RootParameters), RootParameters, 0, 0, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    State->RectangleRootSignature = Direct3D12CreateRootSignature(State, SignatureDesc);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PSO = {};
    PSO.pRootSignature = State->RectangleRootSignature.Get();
    PSO.InputLayout = { VertexLayout, ArrayCount(VertexLayout) };
    PSO.VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get());
    PSO.PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get());
    PSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    PSO.RasterizerState.FrontCounterClockwise = true;
    PSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    PSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    PSO.NumRenderTargets = 1;
    PSO.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    PSO.SampleDesc.Count = 1;
    PSO.SampleMask = U32_MAX;

    State->Device->CreateGraphicsPipelineState(&PSO, IID_PPV_ARGS(&State->RectanglePSO));

    // Create vertex buffer for rendering rectangle
    f32 RectangleVertices[] =
    {
        -1.f, -1.f, 0.f,
        1.f, -1.f, 0.f,
        -1.f, 1.f, 0.f,
        1.f, 1.f, 0.f
    };

    CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(RectangleVertices));

    State->Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        0,
        IID_PPV_ARGS(&State->RectangleVertexBuffer)
    );

    State->RectangleVBV.BufferLocation = State->RectangleVertexBuffer->GetGPUVirtualAddress();
    State->RectangleVBV.SizeInBytes = sizeof(RectangleVertices);
    State->RectangleVBV.StrideInBytes = sizeof(vec3);

    D3D12_SUBRESOURCE_DATA RectangleData = { RectangleVertices };
    d3d12_staging_buffer RectangleStagingBuffer = Direct3D12CreateStagingBuffer(State, State->RectangleVertexBuffer, 0, 1, &RectangleData);

    State->CommandList->CopyResource(State->RectangleVertexBuffer.Get(), RectangleStagingBuffer.Buffer.Get());

    CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(State->RectangleVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    State->CommandList->ResourceBarrier(1, &Barrier);

    Direct3D12ExecuteCommandList(State, true);
    Direct3D12FlushCommandQueue(State);
}

dummy_internal void
Win32InitDirect3D12(d3d12_state *State, win32_platform_state *PlatformState)
{
#if DEBUG
    {
        ComPtr<ID3D12Debug> DebugController;
        D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController));
        DebugController->EnableDebugLayer();
    }
#endif

    CreateDXGIFactory1(IID_PPV_ARGS(&State->DXGIFactory));

    ComPtr<IDXGIAdapter1> Adapter;
    State->DXGIFactory->EnumAdapters1(0, &Adapter);
    Adapter->GetDesc1(&State->AdapterDesc);

    D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&State->Device));

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MultisampleQualityLevels = {};
    MultisampleQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    MultisampleQualityLevels.SampleCount = PlatformState->Samples;

    State->Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MultisampleQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));

    Assert(MultisampleQualityLevels.NumQualityLevels > 0);

    D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
    CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    State->Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&State->CommandQueue));

    DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
    SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> SwapChain;
    State->DXGIFactory->CreateSwapChainForHwnd(State->CommandQueue.Get(), PlatformState->WindowHandle, &SwapChainDesc, 0, 0, &SwapChain);
    SwapChain.As(&State->SwapChain);

    State->CurrentBackBufferIndex = State->SwapChain->GetCurrentBackBufferIndex();

    D3D12_FEATURE_DATA_ROOT_SIGNATURE RootSignatureFeature = { D3D_ROOT_SIGNATURE_VERSION_1_1 };
    State->Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &RootSignatureFeature, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE));
    State->RootSignatureVersion = RootSignatureFeature.HighestVersion;

    State->DescriptorHeapRTV = Direct3D12CreateDescriptorHeap(State, { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
    State->DescriptorHeapDSV = Direct3D12CreateDescriptorHeap(State, { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
    State->DescriptorHeapCBV_SRV_UAV = Direct3D12CreateDescriptorHeap(State, { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE });

    for (u32 BufferIndex = 0; BufferIndex < SWAP_CHAIN_BUFFER_COUNT; ++BufferIndex)
    {
        State->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(State->CommandAllocators[BufferIndex].GetAddressOf()));

        State->SwapChain->GetBuffer(BufferIndex, IID_PPV_ARGS(&State->SwapChainBuffers[BufferIndex]));

        D3D12_CPU_DESCRIPTOR_HANDLE HeapHandleRTV(State->DescriptorHeapRTV.Heap->GetCPUDescriptorHandleForHeapStart().ptr + BufferIndex * State->DescriptorHeapRTV.DescriptorSize);
        State->Device->CreateRenderTargetView(State->SwapChainBuffers[BufferIndex].Get(), 0, HeapHandleRTV);
    }

    State->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, State->CommandAllocators[0].Get(), 0, IID_PPV_ARGS(State->CommandList.GetAddressOf()));

    D3D12_RESOURCE_DESC DepthStencilDesc = {};
    DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    DepthStencilDesc.Width = PlatformState->WindowWidth;
    DepthStencilDesc.Height = PlatformState->WindowHeight;
    DepthStencilDesc.DepthOrArraySize = 1;
    DepthStencilDesc.MipLevels = 1;
    DepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilDesc.SampleDesc.Count = 1;
    DepthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE OptClear = {};
    OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    OptClear.DepthStencil.Depth = 1.f;
    OptClear.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES DepthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);

    State->Device->CreateCommittedResource(
        &DepthStencilHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &DepthStencilDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &OptClear,
        IID_PPV_ARGS(State->DepthStencilTexture.GetAddressOf())
    );

    State->Device->CreateDepthStencilView(State->DepthStencilTexture.Get(), 0, State->DescriptorHeapDSV.Heap->GetCPUDescriptorHandleForHeapStart());

    State->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&State->Fence));
    State->FenceCompletionEvent = CreateEvent(0, false, false, 0);

    for (u32 FenceValueIndex = 0; FenceValueIndex < SWAP_CHAIN_BUFFER_COUNT; ++FenceValueIndex)
    {
        State->FenceValues[FenceValueIndex] = 0;
    }

    State->ConstantBuffer = Direct3D12CreateUploadBuffer(State, Kilobytes(64));

    State->TransformCBV = Direct3D12CreateConstantBufferView(State, 0, sizeof(d3d12_constant_buffer_transform));

    for (u32 RectangleIndex = 0; RectangleIndex < ArrayCount(State->RectanglesCBV); ++RectangleIndex)
    {
        State->RectanglesCBV[RectangleIndex] = Direct3D12CreateConstantBufferView(State, 0, sizeof(d3d12_constant_buffer_rectangle));
    }

    //State->Framebuffer = Direct3D12CreateFramebuffer(State, PlatformState->WindowWidth, PlatformState->WindowHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);

    Direct3D12ExecuteCommandList(State, true);
    Direct3D12FlushCommandQueue(State);

    Direct3D12InitRectangle(State);

    Direct3D12ExecuteCommandList(State);
    Direct3D12FlushCommandQueue(State);
}

dummy_internal void
ShutdownDirect3D12(d3d12_state *State)
{
    Direct3D12FlushCommandQueue(State);
    CloseHandle(State->FenceCompletionEvent);
}

inline D3D12_CPU_DESCRIPTOR_HANDLE
Direct3D12GetDepthStencilView(d3d12_state *State)
{
    D3D12_CPU_DESCRIPTOR_HANDLE Result = State->DescriptorHeapDSV.Heap->GetCPUDescriptorHandleForHeapStart();
    return Result;
}

dummy_internal void
Direct3D12ProcessRenderCommands(d3d12_state *State, render_commands *Commands)
{
    PROFILE(State->Profiler, "Direct3D12ProcessRenderCommands");

    State->RectangleIndex = 0;

    ID3D12Resource *CurrentBackBuffer = Direct3D12GetCurrentBackBuffer(State);
    ID3D12CommandAllocator *CommandAllocator = Direct3D12GetCurrentCommmandAllocator(State);

    CommandAllocator->Reset();
    State->CommandList->Reset(CommandAllocator, 0);

    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView = Direct3D12GetCurrentBackBufferView(State);
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = Direct3D12GetDepthStencilView(State);

    {
        CD3DX12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        State->CommandList->ResourceBarrier(1, &ResourceBarrier);
    }

    State->CommandList->OMSetRenderTargets(1, &CurrentBackBufferView, false, &DepthStencilView);

    ID3D12DescriptorHeap *DescriptorHeaps[] = {
        State->DescriptorHeapCBV_SRV_UAV.Heap.Get()
    };
    State->CommandList->SetDescriptorHeaps(1, DescriptorHeaps);

    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
            case RenderCommand_SetScreenProjection:
            {
                render_command_set_screen_projection *Command = (render_command_set_screen_projection *)Entry;

                mat4 Projection = OrthographicProjection(Command->Left, Command->Right, Command->Bottom, Command->Top, Command->Near, Command->Far);

                d3d12_constant_buffer_transform *Transform = (d3d12_constant_buffer_transform *) State->TransformCBV.Data.CPUAddress;

                Transform->ScreenProjection = Projection;

                break;
            }
            case RenderCommand_SetViewport:
            {
                render_command_set_viewport *Command = (render_command_set_viewport *) Entry;

                CD3DX12_VIEWPORT Viewport = {};
                Viewport.TopLeftX = (f32) Command->x;
                Viewport.TopLeftY = (f32) Command->y;
                Viewport.Width = (f32) Command->Width;
                Viewport.Height = (f32) Command->Height;

                State->CommandList->RSSetViewports(1, &Viewport);

                CD3DX12_RECT ScissorRect = {};
                ScissorRect.left = Command->x;
                ScissorRect.top = Command->y;
                ScissorRect.right = Command->Width;
                ScissorRect.bottom = Command->Height;

                State->CommandList->RSSetScissorRects(1, &ScissorRect);

                break;
            }
            case RenderCommand_Clear:
            {
                render_command_clear *Command = (render_command_clear *) Entry;

                State->CommandList->ClearRenderTargetView(CurrentBackBufferView, Command->Color.Elements, 0, 0);
                State->CommandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, 0);

                break;
            }
            case RenderCommand_DrawRectangle:
            {
                render_command_draw_rectangle *Command = (render_command_draw_rectangle *) Entry;

                mat4 Model = Transform(Command->Transform);

                d3d12_constant_buffer_view *TransformCBV = &State->TransformCBV;
                d3d12_constant_buffer_view *RectangleCBV = &State->RectanglesCBV[State->RectangleIndex++];

                d3d12_constant_buffer_rectangle *Rectangle = (d3d12_constant_buffer_rectangle *) RectangleCBV->Data.CPUAddress;

                Assert(State->RectangleIndex <= ArrayCount(State->RectanglesCBV));

                Rectangle->Model = Model;
                Rectangle->Color = Command->Color;

                State->CommandList->SetGraphicsRootSignature(State->RectangleRootSignature.Get());
                State->CommandList->SetGraphicsRootDescriptorTable(0, TransformCBV->Descriptor.GPUHandle);
                State->CommandList->SetGraphicsRootDescriptorTable(1, RectangleCBV->Descriptor.GPUHandle);
                State->CommandList->IASetVertexBuffers(0, 1, &State->RectangleVBV);
                State->CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                State->CommandList->SetPipelineState(State->RectanglePSO.Get());
                State->CommandList->DrawInstanced(4, 1, 0, 0);

                break;
            }
        }

        BaseAddress += Entry->Size;
    }

#if EDITOR
    // todo: write to texture
#else
    {
        CD3DX12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        State->CommandList->ResourceBarrier(1, &ResourceBarrier);
    }

    Direct3D12ExecuteCommandList(State);
#endif
}

inline void
Direct3D12PresentFrame(d3d12_state *State, bool32 VSync)
{
    State->SwapChain->Present(VSync, 0);

    u64 PrevFrameFenceValue = State->FenceValues[State->CurrentBackBufferIndex];
    State->CurrentBackBufferIndex = State->SwapChain->GetCurrentBackBufferIndex();
    u64 *CurrentFrameFenceValue = &State->FenceValues[State->CurrentBackBufferIndex];

    State->CommandQueue->Signal(State->Fence.Get(), PrevFrameFenceValue);

    if (State->Fence->GetCompletedValue() < *CurrentFrameFenceValue)
    {
        State->Fence->SetEventOnCompletion(*CurrentFrameFenceValue, State->FenceCompletionEvent);
        WaitForSingleObject(State->FenceCompletionEvent, INFINITE);
    }

    *CurrentFrameFenceValue = PrevFrameFenceValue + 1;
}
