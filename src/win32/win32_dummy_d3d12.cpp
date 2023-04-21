#include "dummy.h"
#include "win32_dummy.h"
#include "win32_dummy_d3d12.h"

dummy_internal d3d12_descriptor_heap
Direct3D12CreateDescriptorHeap(d3d12_state *State, D3D12_DESCRIPTOR_HEAP_DESC HeapDesc)
{
    d3d12_descriptor_heap Result;

    State->Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Result.Heap));

    //Result.DescriptorsAllocated = 0;
    //Result.DescriptorsInHeap = HeapDesc.NumDescriptors;
    Result.DescriptorSize = State->Device->GetDescriptorHandleIncrementSize(HeapDesc.Type);

    return Result;
}

inline void
Direct3D12ExecuteCommandList(d3d12_state *State)
{
    State->CommandList->Close();

    ID3D12CommandList *CommandLists[] = { State->CommandList.Get() };
    State->CommandQueue->ExecuteCommandLists(1, CommandLists);
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

    Direct3D12ExecuteCommandList(State);
    Direct3D12FlushCommandQueue(State);
}

dummy_internal void
ShutdownDirect3D12(d3d12_state *State)
{
    Direct3D12FlushCommandQueue(State);
    CloseHandle(State->FenceCompletionEvent);
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

    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
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
        }

        BaseAddress += Entry->Size;
    }


    vec4 ClearColor = vec4(1.f, 1.f, 1.f, 1.f);
    State->CommandList->OMSetRenderTargets(1, &CurrentBackBufferView, false, &DepthStencilView);

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
Direct3D12PresentFrame(d3d12_state *State)
{
    State->SwapChain->Present(0, 0);

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
