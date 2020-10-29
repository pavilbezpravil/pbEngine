#include "pch.h"
#include "GraphicsCore.h"
#include "GpuTimeManager.h"
#include "ColorBuffer.h"
#include "SamplerManager.h"
#include "DescriptorHeap.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "RootSignature.h"
#include "CommandSignature.h"
#include <algorithm>

// This macro determines whether to detect if there is an HDR display and enable HDR10 output.
// Currently, with HDR display enabled, the pixel magnfication functionality is broken.
#define CONDITIONALLY_ENABLE_HDR_OUTPUT 1

// Uncomment this to enable experimental support for the new shader compiler, DXC.exe
//#define DXIL

#include <dxgi1_6.h>

#include <winreg.h>

#include "pbe/Core/SystemTime.h"

#define SWAP_CHAIN_BUFFER_COUNT 3

DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != nullptr) { x->Release(); x = nullptr; }
#endif

namespace
{
    float s_FrameTime = 0.0f;
    uint64_t s_FrameIndex = 0;
    int64_t s_FrameStartTick = 0;

    BoolVar s_LimitTo30Hz("Timing/Limit To 30Hz", false);
    BoolVar s_DropRandomFrames("Timing/Drop Random Frames", false);
}

namespace Graphics
{
    void PreparePresentLDR();
    void PreparePresentHDR();

#ifndef RELEASE
    const GUID WKPDID_D3DDebugObjectName = { 0x429b8c22,0x9188,0x4b0c, { 0x87,0x42,0xac,0xb0,0xbf,0x85,0xc2,0x00 }};
#endif

    BoolVar s_EnableVSync("Timing/VSync", true);

    bool g_bTypedUAVLoadSupport_R11G11B10_FLOAT = false;
    bool g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT = false;
    bool g_bEnableHDROutput = false;

    ID3D12Device* g_Device = nullptr;

    CommandListManager g_CommandManager;
    ContextManager g_ContextManager;

    D3D_FEATURE_LEVEL g_D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    ColorBuffer g_DisplayPlane[SWAP_CHAIN_BUFFER_COUNT];
    UINT g_CurrentBuffer = 0;

    IDXGISwapChain1* s_SwapChain1 = nullptr;

    DescriptorAllocator g_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
    {
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
    };

    enum { kBilinear, kBicubic, kSharpening, kFilterCount };
    const char* FilterLabels[] = { "Bilinear", "Bicubic", "Sharpening" };
    EnumVar UpsampleFilter("Graphics/Display/Upsample Filter", kFilterCount - 1, kFilterCount, FilterLabels);
    NumVar BicubicUpsampleWeight("Graphics/Display/Bicubic Filter Weight", -0.75f, -1.0f, -0.25f, 0.25f);
    NumVar SharpeningSpread("Graphics/Display/Sharpness Sample Spread", 1.0f, 0.7f, 2.0f, 0.1f);
    NumVar SharpeningRotation("Graphics/Display/Sharpness Sample Rotation", 45.0f, 0.0f, 90.0f, 15.0f);
    NumVar SharpeningStrength("Graphics/Display/Sharpness Strength", 0.10f, 0.0f, 1.0f, 0.01f);

    enum DebugZoomLevel { kDebugZoomOff, kDebugZoom2x, kDebugZoom4x, kDebugZoom8x, kDebugZoom16x, kDebugZoomCount };
    const char* DebugZoomLabels[] = { "Off", "2x Zoom", "4x Zoom", "8x Zoom", "16x Zoom" };
    EnumVar DebugZoom("Graphics/Display/Magnify Pixels", kDebugZoomOff, kDebugZoomCount, DebugZoomLabels);
}

void Graphics::Resize(uint32_t width, uint32_t height)
{
    ASSERT(s_SwapChain1 != nullptr);

    // Check for invalid window dimensions
    if (width == 0 || height == 0)
        return;

    g_CommandManager.IdleGPU();


    for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
        g_DisplayPlane[i].Destroy();

    ASSERT_SUCCEEDED(s_SwapChain1->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, width, height, SwapChainFormat, 0));

    for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
    {
        ComPtr<ID3D12Resource> DisplayPlane;
        ASSERT_SUCCEEDED(s_SwapChain1->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
        g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
    }

    g_CurrentBuffer = 0;

    g_CommandManager.IdleGPU();
}

static HRESULT EnableExperimentalShaderModels() { return S_OK; }

// Initialize the DirectX resources required to run.
void Graphics::Initialize(HWND g_hWnd, pbe::uint width, pbe::uint height)
{
    ASSERT(s_SwapChain1 == nullptr, "Graphics has already been initialized");

    Microsoft::WRL::ComPtr<ID3D12Device> pDevice;

#if HZ_DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(MY_IID_PPV_ARGS(&debugInterface))))
        debugInterface->EnableDebugLayer();
    else
        HZ_CORE_WARN("Unable to enable D3D12 debug validation layer\n");
#endif

    EnableExperimentalShaderModels();

    // Obtain the DXGI factory
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    ASSERT_SUCCEEDED(CreateDXGIFactory2(0, MY_IID_PPV_ARGS(&dxgiFactory)));

    // Create the D3D graphics device
    Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;

     SIZE_T MaxSize = 0;

     for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(Idx, &pAdapter); ++Idx)
     {
         DXGI_ADAPTER_DESC1 desc;
         pAdapter->GetDesc1(&desc);
         if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
             continue;

         if (desc.DedicatedVideoMemory > MaxSize && SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&pDevice))))
         {
             pAdapter->GetDesc1(&desc);
				 HZ_CORE_INFO("D3D12-capable hardware found:  {0} {1} MB", MakeStr(desc.Description), desc.DedicatedVideoMemory >> 20);
             MaxSize = desc.DedicatedVideoMemory;
         }
     }

     if (MaxSize > 0)
         g_Device = pDevice.Detach();

    if (g_Device == nullptr) {
			HZ_CORE_WARN("Failed to find a hardware adapter.  Falling back to WARP");
        ASSERT_SUCCEEDED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter)));
        ASSERT_SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&pDevice)));
        g_Device = pDevice.Detach();
    }
#ifndef HZ_RELEASE
    else
    {
        bool DeveloperModeEnabled = false;

        // Look in the Windows Registry to determine if Developer Mode is enabled
        HKEY hKey;
        LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS)
        {
            DWORD keyValue, keySize = sizeof(DWORD);
            result = RegQueryValueEx(hKey, L"AllowDevelopmentWithoutDevLicense", 0, NULL, (byte*)&keyValue, &keySize);
            if (result == ERROR_SUCCESS && keyValue == 1)
                DeveloperModeEnabled = true;
            RegCloseKey(hKey);
        }

        WARN_ONCE_IF_NOT(DeveloperModeEnabled, "Enable Developer Mode on Windows 10 to get consistent profiling results");

        // Prevent the GPU from overclocking or underclocking to get consistent timings
        if (DeveloperModeEnabled)
            g_Device->SetStablePowerState(TRUE);
    }
#endif    

#if HZ_DEBUG
    ID3D12InfoQueue* pInfoQueue = nullptr;
    if (SUCCEEDED(g_Device->QueryInterface(MY_IID_PPV_ARGS(&pInfoQueue))))
    {
        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] = 
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] =
        {
            // This occurs when there are uninitialized descriptors in a descriptor table, even when a
            // shader does not access the missing descriptors.  I find this is common when switching
            // shader permutations and not wanting to change much code to reorder resources.
            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

            // Triggered when a shader does not export all color components of a render target, such as
            // when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
            D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

            // This occurs when a descriptor table is unbound even when a shader does not access the missing
            // descriptors.  This is common with a root signature shared between disparate shaders that
            // don't all need the same types of resources.
            D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

            // RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS
            (D3D12_MESSAGE_ID)1008,
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        pInfoQueue->PushStorageFilter(&NewFilter);
        pInfoQueue->Release();
    }
#endif

    // We like to do read-modify-write operations on UAVs during post processing.  To support that, we
    // need to either have the hardware do typed UAV loads of R11G11B10_FLOAT or we need to manually
    // decode an R32_UINT representation of the same buffer.  This code determines if we get the hardware
    // load support.
    D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData = {};
    if (SUCCEEDED(g_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData))))
    {
        if (FeatureData.TypedUAVLoadAdditionalFormats)
        {
            D3D12_FEATURE_DATA_FORMAT_SUPPORT Support =
            {
                DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
            };

            if (SUCCEEDED(g_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
                (Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
            {
                g_bTypedUAVLoadSupport_R11G11B10_FLOAT = true;
            }

            Support.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

            if (SUCCEEDED(g_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
                (Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
            {
                g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT = true;
            }
        }
    }

    g_CommandManager.Create(g_Device);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = SwapChainFormat;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChainForHwnd(g_CommandManager.GetCommandQueue(), g_hWnd, &swapChainDesc, nullptr, nullptr, &s_SwapChain1));

#if CONDITIONALLY_ENABLE_HDR_OUTPUT && defined(NTDDI_WIN10_RS2) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
    {
        IDXGISwapChain4* swapChain = (IDXGISwapChain4*)s_SwapChain1;
        ComPtr<IDXGIOutput> output;
        ComPtr<IDXGIOutput6> output6;
        DXGI_OUTPUT_DESC1 outputDesc;
        UINT colorSpaceSupport;

        // Query support for ST.2084 on the display and set the color space accordingly
        if (SUCCEEDED(swapChain->GetContainingOutput(&output)) &&
            SUCCEEDED(output.As(&output6)) &&
            SUCCEEDED(output6->GetDesc1(&outputDesc)) &&
            outputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 &&
            SUCCEEDED(swapChain->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &colorSpaceSupport)) &&
            (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) &&
            SUCCEEDED(swapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)))
        {
            g_bEnableHDROutput = true;
        }
    }
#endif

    for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
    {
        ComPtr<ID3D12Resource> DisplayPlane;
        ASSERT_SUCCEEDED(s_SwapChain1->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
        g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
    }

    // Common state was moved to GraphicsCommon.*
	 InitializeCommonState();

    GpuTimeManager::Initialize(4096);
}

void Graphics::Terminate( void )
{
    g_CommandManager.IdleGPU();
    s_SwapChain1->SetFullscreenState(FALSE, nullptr);
}

void Graphics::Shutdown( void )
{
    g_CommandManager.IdleGPU();

    CommandContext::DestroyAllContexts();
    g_CommandManager.Shutdown();
    GpuTimeManager::Shutdown();
    s_SwapChain1->Release();
    PSO::DestroyAll();
    RootSignature::DestroyAll();
    DescriptorAllocator::DestroyAll();

    DestroyCommonState();

    for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
        g_DisplayPlane[i].Destroy();

#ifdef HZ_DEBUG
    ID3D12DebugDevice* debugInterface;
    if (SUCCEEDED(g_Device->QueryInterface(&debugInterface)))
    {
        debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        debugInterface->Release();
    }
#endif

    SAFE_RELEASE(g_Device);
}

void Graphics::PreparePresentHDR(void)
{
	HZ_CORE_ASSERT(false);
    GraphicsContext& Context = GraphicsContext::Begin(L"Present");

    Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET);
	 Context.ClearColor(g_DisplayPlane[g_CurrentBuffer]);

    Context.Finish();
}

void Graphics::PreparePresentLDR(void)
{
	GraphicsContext& Context = GraphicsContext::Begin(L"Present");

	Context.TransitionResource(GetCurrentBB(), D3D12_RESOURCE_STATE_PRESENT);

	// dx12
	std::this_thread::sleep_for(std::chrono::milliseconds(16));

	Context.Finish();
}

void Graphics::Present(void)
{
    if (g_bEnableHDROutput)
        PreparePresentHDR();
    else
        PreparePresentLDR();

    g_CurrentBuffer = (g_CurrentBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

    UINT PresentInterval = s_EnableVSync ? std::min(4, (int)std::round(s_FrameTime * 60.0f)) : 0;

    s_SwapChain1->Present(PresentInterval, 0);

    int64_t CurrentTick = SystemTime::GetCurrentTick();
	 // static int n = 0;
	 // ++n;
	 // HZ_CORE_INFO("{}", n);

    if (s_EnableVSync)
    {
        // With VSync enabled, the time step between frames becomes a multiple of 16.666 ms.  We need
        // to add logic to vary between 1 and 2 (or 3 fields).  This delta time also determines how
        // long the previous frame should be displayed (i.e. the present interval.)
        s_FrameTime = (s_LimitTo30Hz ? 2.0f : 1.0f) / 60.0f;
        if (s_DropRandomFrames)
        {
            if (std::rand() % 50 == 0)
                s_FrameTime += (1.0f / 60.0f);
        }
    }
    else
    {
        // When running free, keep the most recent total frame time as the time step for
        // the next frame simulation.  This is not super-accurate, but assuming a frame
        // time varies smoothly, it should be close enough.
        s_FrameTime = (float)SystemTime::TimeBetweenTicks(s_FrameStartTick, CurrentTick);
    }

    s_FrameStartTick = CurrentTick;

    ++s_FrameIndex;
}

uint64_t Graphics::GetFrameCount(void)
{
    return s_FrameIndex;
}

float Graphics::GetFrameTime(void)
{
    return s_FrameTime;
}

float Graphics::GetFrameRate(void)
{
    return s_FrameTime == 0.0f ? 0.0f : 1.0f / s_FrameTime;
}

ColorBuffer& Graphics::GetCurrentBB() {
	return g_DisplayPlane[g_CurrentBuffer];
}
