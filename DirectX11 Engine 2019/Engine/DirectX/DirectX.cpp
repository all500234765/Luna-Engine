#include "DirectX.h"
#include <iostream>
#include <string>

#include "Engine/Extensions/Default.h"

#if USE_ANSEL
#include "Vendor/Ansel/AnselSDK.h"
#endif

_DirectX::_DirectX() {
}

int _DirectX::Create(const DirectXConfig& config) {
    // Save config
    cfg = config;

    // Temp variables
    D3D_FEATURE_LEVEL level;
    HRESULT res;
    D3D_FEATURE_LEVEL pFeatureLevels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };
    
    D3D_DRIVER_TYPE DXDriverType[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE
    };

    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    if( config.UseHDR ) {
        // WTF
        // In the Ansel's documentation it says:
        // Ansel currently supports the following backbuffer formats:
        //  DXGI_FORMAT_R8G8B8A8_UNORM
        //  DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
        //  DXGI_FORMAT_B8G8R8A8_UNORM
        //  DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
        //  DXGI_FORMAT_R10G10B10A2_UNORM
        // Additionally, multisampling is supported for all the above formats. If your game is using a format that is not on the list 
        // Ansel will produce images with zero for every pixel - i.e. black.
        // 
        // THO! You still can use DXGI_FORMAT_R16G16B16A16_FLOAT !
        // I hope that they just forgot to update their guide.

        //if( config.Ansel ) format = DXGI_FORMAT_R10G10B10A2_UNORM;
        //else               
        format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    }

    // Create swapchain
    SecureZeroMemory(&scd, sizeof(scd));
    scd.Width = config.Width;
    scd.Height = config.Height;
    scd.Format = format;
    
    scd.BufferCount = config.BufferCount;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.Flags = 0;
    scd.SwapEffect = (config.BufferCount >= 2) ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_SEQUENTIAL;

    UINT DeviceFlags = 0;

#ifdef _DEBUG
    DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    if( config.MSAA ) {
        scd.SampleDesc.Count   = config.MSAA_Samples;
        scd.SampleDesc.Quality = config.MSAA_Quality;
    } else {
        scd.SampleDesc.Count   = 1;
        scd.SampleDesc.Quality = 0;
    }

    pSCFDesc.RefreshRate.Numerator   = config.RefreshRate;
    pSCFDesc.RefreshRate.Denominator = 1;
    pSCFDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
    pSCFDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    pSCFDesc.Windowed                = config.Windowed;

    // Create device
    for( int i = 0; i < 3; i++ ) {
        res = D3D11CreateDevice(NULL, DXDriverType[i], NULL, DeviceFlags, pFeatureLevels, ARRAYSIZE(pFeatureLevels),
                                D3D11_SDK_VERSION, &gDevice, &level, &gContext);

        if( SUCCEEDED(res) ) { break; }
    }

    //res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, DeviceFlags, pFeatureLevels, 
    //                                    1, D3D11_SDK_VERSION, &scd, &gSwapchain, &gDevice, &level, &gContext);
    if( FAILED(res) ) { return 1; }

    std::cout << "DirectX 11 Device created with ";

    switch( level ) {
        case D3D_FEATURE_LEVEL_12_1: std::cout << "12.1"; break;
        case D3D_FEATURE_LEVEL_12_0: std::cout << "12.0"; break;
        case D3D_FEATURE_LEVEL_11_1: std::cout << "11.1"; break;
        case D3D_FEATURE_LEVEL_11_0: std::cout << "11.0"; break;
    }

    std::cout << " version" << std::endl;

    // Get factory // Prob just IDXGIDevice
    IDXGIDevice2 *pDXGIDevice = nullptr;
    res = gDevice->QueryInterface(__uuidof(IDXGIDevice2), (void **)&pDXGIDevice);
    if( FAILED(res) ) { return 10; } // Failed to retrieve DXGI Device

    IDXGIAdapter *pDXGIAdapter = nullptr;
    res = pDXGIDevice->GetAdapter(&pDXGIAdapter);
    if( FAILED(res) ) { return 11; } // Failed to retrieve DXGI Adapter
    
    IDXGIFactory4 *pIDXGIFactory = nullptr;
    res = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory4), (void **)&pIDXGIFactory);
    if( FAILED(res) ) { return 12; } // Failed to retrieve DXGI Factory

    // Get DXGI Adapter 3
    IDXGIAdapter1 *tmpDxgiAdapter = nullptr;

    DXGI_ADAPTER_DESC1 desc;
    UINT adapterIndex = 0;

    while( pIDXGIFactory->EnumAdapters1(adapterIndex, &tmpDxgiAdapter) != DXGI_ERROR_NOT_FOUND ) {
        tmpDxgiAdapter->GetDesc1(&desc);

        if( !gAdapter3 && desc.Flags == 0 ) {
            tmpDxgiAdapter->QueryInterface(IID_PPV_ARGS(&gAdapter3));
        }

        tmpDxgiAdapter->Release();
        ++adapterIndex;
    }

    // Enumerate adapters
    UINT OutputIndex = 0;
    if( gAdapter3 ) {
        while( gAdapter3->EnumOutputs(OutputIndex, &gOutput) != DXGI_ERROR_NOT_FOUND ) {
            if( gOutput ) break;
            ++OutputIndex;
        }
    } else {
        while( pDXGIAdapter->EnumOutputs(OutputIndex, &gOutput) != DXGI_ERROR_NOT_FOUND ) {
            if( gOutput ) break;
            ++OutputIndex;
        }
    }

    // Create swapchain
    res = pIDXGIFactory->CreateSwapChainForHwnd(gDevice, config.m_hwnd, &scd, &pSCFDesc, gOutput, &gSwapchain);
    if( FAILED(res) ) { return 13; } // Failed to create swapchain

    gSwapchain4 = static_cast<IDXGISwapChain4*>(gSwapchain);

    // Check for HDR API Support
    gUseHDR = false;
    res = gOutput->QueryInterface(__uuidof(IDXGIOutput6), (void**)&gOutput6);
    if( SUCCEEDED(res) ) {
        gUseHDR = true;

        gOutput6->GetDesc1(&pDescHDR);

        DXGI_HDR_METADATA_HDR10 pHDRMeta;
        for( int i = 0; i < 2; i++ ) {
            pHDRMeta.RedPrimary[i]   = UINT(50000.f * pDescHDR.RedPrimary[i]  );
            pHDRMeta.GreenPrimary[i] = UINT(50000.f * pDescHDR.GreenPrimary[i]);
            pHDRMeta.BluePrimary[i]  = UINT(50000.f * pDescHDR.BluePrimary[i] );
            pHDRMeta.WhitePoint[i]   = UINT(50000.f * pDescHDR.WhitePoint[i]  );
        }

        pHDRMeta.MinMasteringLuminance     = UINT(pDescHDR.MinLuminance * 10000.f);
        pHDRMeta.MaxMasteringLuminance     = UINT(pDescHDR.MaxLuminance * 10000.f);
        pHDRMeta.MaxFrameAverageLightLevel = UINT(pDescHDR.MaxLuminance * 10000.f / 2.f);
        pHDRMeta.MaxContentLightLevel      = UINT(pDescHDR.MaxFullFrameLuminance * 10000.f);

        std::wstring DeviceNameWstr = (pDescHDR.DeviceName);

        // Output DXGI_OUTPUT_DESC1 for debug
        std::cout << "DXGI_OUTPUT_DESC1: \n"
            << "\tDeviceName: "            << DeviceNameWstr.c_str()         << "\n"
            << "\tAttachedToDesktop: "     << pDescHDR.AttachedToDesktop     << "\n"
            << "\tRotation: "              << pDescHDR.Rotation              << "\n" //
            << "\tMonitor: "               << pDescHDR.Monitor               << "\n"
            << "\tBitsPerColor: "          << pDescHDR.BitsPerColor          << "\n"
            << "\tColorSpace: "            << pDescHDR.ColorSpace            << "\n" //
            << "\tRedPrimary  [0]: "       << pDescHDR.RedPrimary  [0]       << "\n"
            << "\tGreenPrimary[0]: "       << pDescHDR.GreenPrimary[0]       << "\n"
            << "\tBluePrimary [0]: "       << pDescHDR.BluePrimary [0]       << "\n"
            << "\tWhitePoint  [0]: "       << pDescHDR.WhitePoint  [0]       << "\n"
            << "\tRedPrimary  [1]: "       << pDescHDR.RedPrimary  [1]       << "\n"
            << "\tGreenPrimary[1]: "       << pDescHDR.GreenPrimary[1]       << "\n"
            << "\tBluePrimary [1]: "       << pDescHDR.BluePrimary [1]       << "\n"
            << "\tWhitePoint  [1]: "       << pDescHDR.WhitePoint  [1]       << "\n"
            << "\tMinLuminance: "          << pDescHDR.MinLuminance          << "\n"
            << "\tMaxLuminance: "          << pDescHDR.MaxLuminance          << "\n"
            << "\tMaxFullFrameLuminance: " << pDescHDR.MaxFullFrameLuminance << "\n";

        gSwapchain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(pHDRMeta), &pHDRMeta);
    }

    //;gSwapchain4->SetColorSpace1(DXGI_COLOR_SPACE_TYPE::DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);

    // Clean up
    pIDXGIFactory->Release();

    // Check MSAA levels
    /*UINT maxQuality;
    for( int i = 1; i < 16; i *= 2 ) {
        gDevice->CheckMultisampleQualityLevels(scd.BufferDesc.Format, i, &maxQuality);
        std::cout << maxQuality << std::endl;
    }*/

#ifdef _DEBUG
    gDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&gDebug));
#endif

    // Set ansel state
#if USE_ANSEL
    if( cfg.Ansel ) cfg.Ansel = ansel::isAnselAvailable();
#endif

    // Get texture
    ID3D11Texture2D *BackBufferColor;
    res = gSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBufferColor);
    if( FAILED(res) ) { return 2; }

    // Create RTV
    D3D11_RENDER_TARGET_VIEW_DESC pDesc;
    pDesc.Format = scd.Format;
    pDesc.ViewDimension = (D3D11_RTV_DIMENSION)(D3D11_RTV_DIMENSION_TEXTURE2D + 2 * cfg.MSAA);
    pDesc.Texture2D.MipSlice = 0;

    res = gDevice->CreateRenderTargetView(BackBufferColor, NULL, &gRTV);
    BackBufferColor->Release(); // ?
    if( FAILED(res) ) { return 3; }

    // Create DSV

    // Descs for DSV and depth texture
    ZeroMemory(&pDesc2, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    ZeroMemory(&pTex2DDesc, sizeof(D3D11_TEXTURE2D_DESC));
    ZeroMemory(&pDSD, sizeof(D3D11_DEPTH_STENCIL_DESC));
    
    // D3D11_TEXTURE2D_DESC
    pTex2DDesc.Width = config.Width;
    pTex2DDesc.Height = config.Height;
    pTex2DDesc.MipLevels = 1;
    pTex2DDesc.ArraySize = 1;
    pTex2DDesc.Format = DXGI_FORMAT_R32_TYPELESS; // DXGI_FORMAT_R24G8_TYPELESS; // 
    pTex2DDesc.SampleDesc.Count = scd.SampleDesc.Count;
    pTex2DDesc.SampleDesc.Quality = scd.SampleDesc.Quality;
    pTex2DDesc.Usage = D3D11_USAGE_DEFAULT;
    pTex2DDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // 
    pTex2DDesc.CPUAccessFlags = 0;
    pTex2DDesc.MiscFlags = 0;

    // D3D11_DEPTH_STENCIL_DESC
    pDSD.DepthEnable = true;
    pDSD.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    pDSD.DepthFunc = D3D11_COMPARISON_GREATER; //D3D11_COMPARISON_LESS
    
    pDSD.StencilEnable = !true;
    pDSD.StencilReadMask = 0xFF;
    pDSD.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    pDSD.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    pDSD.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    pDSD.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    pDSD.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    pDSD.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    pDSD.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    pDSD.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    pDSD.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // D3D11_DEPTH_STENCIL_VIEW_DESC
    pDesc2.Format = DXGI_FORMAT_D32_FLOAT; // DXGI_FORMAT_D24_UNORM_S8_UINT; // Select format for DSV texture
    pDesc2.Texture2D.MipSlice = 0;
    pDesc2.ViewDimension = (D3D11_DSV_DIMENSION)(D3D11_DSV_DIMENSION_TEXTURE2D + 2 * cfg.MSAA);
    pDesc2.Flags = 0;

    // Create depth stencil states
    gDevice->CreateDepthStencilState(&pDSD, &pDSS_Default);

    pDSD.DepthEnable = false;
    gDevice->CreateDepthStencilState(&pDSD, &pDSS_Default_NoDepthWrite);

    pDSD.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    gDevice->CreateDepthStencilState(&pDSD, &pDSS_Default_InvDepth);

    // Create depth texture
    gDevice->CreateTexture2D(&pTex2DDesc, NULL, &gDSVTex);

    // Create depth stencil view
    gDevice->CreateDepthStencilView(gDSVTex, &pDesc2, &gDSV);

    // Bind default targets
    gContext->OMSetRenderTargets(1, &gRTV, gDSV);
    gContext->OMSetDepthStencilState(pDSS_Default, 1);

    // Viewport
    D3D11_VIEWPORT vp;
    vp.Width    = (FLOAT)config.Width;
    vp.Height   = (FLOAT)config.Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;

    gContext->RSSetViewports(1, &vp);

    // Setup the raster description which will determine how and what polygons will be drawn.
    D3D11_RASTERIZER_DESC rDesc;
    ZeroMemory(&rDesc, sizeof(D3D11_RASTERIZER_DESC));
    rDesc.AntialiasedLineEnable = false;
    rDesc.CullMode = D3D11_CULL_NONE; // Disabled culling (Default: D3D11_CULL_BACK)
    rDesc.DepthBias = 0;
    rDesc.DepthBiasClamp = 0.0f;
    rDesc.DepthClipEnable = true;
    rDesc.FillMode = D3D11_FILL_SOLID;
    rDesc.FrontCounterClockwise = false;
    rDesc.MultisampleEnable = cfg.MSAA;
    rDesc.ScissorEnable = false;
    rDesc.SlopeScaledDepthBias = 0.0f;

    D3D11_RASTERIZER_DESC rDesc2;
    ZeroMemory(&rDesc2, sizeof(D3D11_RASTERIZER_DESC));
    rDesc2.AntialiasedLineEnable = false;
    rDesc2.CullMode = D3D11_CULL_NONE;
    rDesc2.DepthBias = 0;
    rDesc2.DepthBiasClamp = 0.0f;
    rDesc2.DepthClipEnable = true;
    rDesc2.FillMode = D3D11_FILL_WIREFRAME;
    rDesc2.FrontCounterClockwise = false;
    rDesc2.MultisampleEnable = cfg.MSAA;
    rDesc2.ScissorEnable = false;
    rDesc2.SlopeScaledDepthBias = 0.0f;
    
    // Create RSs
    gDevice->CreateRasterizerState(&rDesc, &gRSDefault);
    gDevice->CreateRasterizerState(&rDesc2, &gRSDefaultWriteframe);

    // Set RS
    gContext->RSSetState(gRSDefault);

    // Create SRV for DSV
    D3D11_SHADER_RESOURCE_VIEW_DESC pDesc1;
    ZeroMemory(&pDesc1, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    pDesc1.Format                    = DXGI_FORMAT_R32_FLOAT; //DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    pDesc1.ViewDimension             = (D3D11_SRV_DIMENSION)(D3D11_SRV_DIMENSION_TEXTURE2D + 2 * cfg.MSAA);
    pDesc1.Texture2D.MipLevels       = 1;
    pDesc1.Texture2D.MostDetailedMip = 0;

    gDevice->CreateShaderResourceView(gDSVTex, &pDesc1, &gDSV_SRV);

    // Create deferred context
    //if( config.DeferredContext ) {
    //    gContextImm = 0;
    //    gDevice->CreateDeferredContext(0, &gContextImm);
    //
    //    // Swap deferred with imm. context
    //    auto t = gContextImm;
    //    gContextImm = gContext;
    //    gContext = t;
    //}

    // Obtain ID3D11DeviceN, and for ID3D11DeviceContextN
    res = gDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&gDeviceN);
    if( FAILED(res) ) { return 14; }

    res = gContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&gContextN);
    if( FAILED(res) ) { return 15; }
    
    // No errors
    return 0;
}

bool _DirectX::ShowError(int id) {
    if( id == 0 ) { return false; }
    const char *error;

    switch( id ) {
        case 1: error = "Failed to create device and swapchain."; break;
        case 2: error = "Failed to fetch back buffer texture."; break;
        case 3: error = "Failed to create render target view."; break;
        //case 4: std::cout << "Failed to initilize AnselSDK." << std::endl; return;
        default: error = (std::string("Unknown error. (") + std::to_string(id)).c_str(); break; // TODO: Fix
    }

    MessageBoxA(NULL, error, "DirectX Initialization failed", MB_OK);
    //MessageBoxA(NULL, , "Error code", MB_OK);

    return true;
}

const DirectXConfig& _DirectX::GetConfig() {
    return cfg;
}
