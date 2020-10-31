#pragma once

#include "PipelineState.h"
#include "DescriptorHeap.h"
#include "RootSignature.h"


#include "pbe/Core/EngineVar.h"

class ColorBuffer;
class DepthBuffer;
class GraphicsPSO;
class CommandContext;
class CommandListManager;
class CommandSignature;
class ContextManager;

namespace Graphics
{
#ifndef RELEASE
    extern const GUID WKPDID_D3DDebugObjectName;
#endif

    using namespace Microsoft::WRL;

    void Initialize(HWND g_hWnd, pbe::uint width, pbe::uint height);
    void Resize(uint32_t width, uint32_t height);
    void Terminate(void);
    void Shutdown(void);
    void Present(void);

    // Returns the number of elapsed frames since application start
    uint64_t GetFrameCount(void);

    // The amount of time elapsed during the last completed frame.  The CPU and/or
    // GPU may be idle during parts of the frame.  The frame time measures the time
    // between calls to present each frame.
    float GetFrameTime(void);

    // The total number of frames per second
    float GetFrameRate(void);

	pbe::Ref<ColorBuffer> GetCurrentBB();

    extern ID3D12Device* g_Device;
    extern CommandListManager g_CommandManager;
    extern ContextManager g_ContextManager;

    extern D3D_FEATURE_LEVEL g_D3DFeatureLevel;
    extern bool g_bTypedUAVLoadSupport_R11G11B10_FLOAT;
    extern bool g_bEnableHDROutput;

    extern DescriptorAllocator g_DescriptorAllocator[];
    inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor( D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1 )
    {
        return g_DescriptorAllocator[Type].Allocate(Count);
    }
}
