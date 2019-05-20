#include "DirectX.h"
#include <iostream>

#include "Engine/Extensions/Default.h"

#if USE_ANSEL
#include "Vendor/Ansel/AnselSDK.h"
#endif

_DirectX::_DirectX() {
}

int _DirectX::Create(DirectXConfig config) {
    // Save config
    cfg = config;

    // Temp variables
    D3D_FEATURE_LEVEL level;
    HRESULT res;
    D3D_FEATURE_LEVEL pFeatureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

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

        // THO! You still can use DXGI_FORMAT_R16G16B16A16_FLOAT !
        // I hope that they just forgot to update their guide.

        //if( config.Ansel ) format = DXGI_FORMAT_R10G10B10A2_UNORM;
        //else               
        format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    }

    // Create swapchain
    SecureZeroMemory(&scd, sizeof(scd));
    scd.BufferDesc.Width = config.Width;
    scd.BufferDesc.Height = config.Height;
    scd.BufferDesc.Format = format;
    scd.BufferDesc.RefreshRate.Numerator = config.RefreshRate;
    scd.BufferDesc.RefreshRate.Denominator = 1;

    scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    scd.BufferCount = config.BufferCount;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = config.m_hwnd;
    scd.Windowed = config.Windowed;
    scd.Flags = 0;
    scd.SwapEffect = (config.BufferCount >= 2) ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_SEQUENTIAL;

    UINT DeviceFlags = 0;

#ifdef _DEBUG
    DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    
    if( config.MSAA ) {
        scd.SampleDesc.Count = config.MSAA_Samples;
        scd.SampleDesc.Quality = config.MSAA_Quality;
    } else {
        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;
    }

    // Create device and swapchain
    res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, DeviceFlags, pFeatureLevels, 1, D3D11_SDK_VERSION,
                                        &scd, &gSwapchain, &gDevice, &level, &gContext);
    if( FAILED(res) ) { return 1; }

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
    pDesc.Format = scd.BufferDesc.Format;
    pDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
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
    pTex2DDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // 
    pTex2DDesc.SampleDesc.Count = 1;
    pTex2DDesc.SampleDesc.Quality = 0;
    pTex2DDesc.Usage = D3D11_USAGE_DEFAULT;
    pTex2DDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // 
    pTex2DDesc.CPUAccessFlags = 0;
    pTex2DDesc.MiscFlags = 0;

    // D3D11_DEPTH_STENCIL_DESC
    pDSD.DepthEnable = true;
    pDSD.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    pDSD.DepthFunc = D3D11_COMPARISON_LESS;
    
    pDSD.StencilEnable = true;
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
    pDesc2.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Select format for DSV texture
    pDesc2.Texture2D.MipSlice = 0;
    pDesc2.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    pDesc2.Flags = 0;

    // Create depth stencil state
    gDevice->CreateDepthStencilState(&pDSD, &pDSS_Default);

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
    rDesc.MultisampleEnable = false;
    rDesc.ScissorEnable = false;
    rDesc.SlopeScaledDepthBias = 0.0f;

    D3D11_RASTERIZER_DESC rDesc2;
    ZeroMemory(&rDesc, sizeof(D3D11_RASTERIZER_DESC));
    rDesc2.AntialiasedLineEnable = false;
    rDesc2.CullMode = D3D11_CULL_BACK;
    rDesc2.DepthBias = 0;
    rDesc2.DepthBiasClamp = 0.0f;
    rDesc2.DepthClipEnable = true;
    rDesc2.FillMode = D3D11_FILL_WIREFRAME;
    rDesc2.FrontCounterClockwise = false;
    rDesc2.MultisampleEnable = false;
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
    pDesc1.Format                    = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    pDesc1.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    pDesc1.Texture2D.MipLevels       = -1;
    pDesc1.Texture2D.MostDetailedMip = 0;

    gDevice->CreateShaderResourceView(gDSVTex, &pDesc1, &gDSV_SRV);

    // Setup 2D rendering
    /*HRESULT hr = S_OK;

    // Create 2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g2DFactory);

    // Create DirectWrite factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g2DWriteFactory));

    // Create text format
    hr = g2DWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, 
                                           DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 
                                           ConvertPointSizeToDIP(12.f), L"en-us", &gTextFormat_Arial);

    // Set text alignment
    gTextFormat_Arial->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

    // Create render target
    D2D1_SIZE_U size = D2D1::SizeU(config.Width, config.Height);
    g2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), 
                                       D2D1::HwndRenderTargetProperties(config.m_hwnd, size), 
                                       &g2DRT);

    // Create brushie
    g2DRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Lime), &gSolidLimeBrush);

    // Create text layout
    std::wstring text = L"YA YEBAL V ROT ETOGO KAZINO!";
    g2DWriteFactory->CreateTextLayout(text.c_str(), text.length(), gTextFormat_Arial, size.width, size.height, &gTextLayout);

    // Set font size
    DWRITE_TEXT_RANGE range = {28, 4};
    gTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);*/

    // No errors
    return 0;
}

void _DirectX::ShowError(int id) {
    if( id == 0 ) { return; }
    const wchar_t *error;

    switch( id ) {
        case 1: error = L"Failed to create device and swapchain."; break;
        case 2: error = L"Failed to fetch back buffer texture."; break;
        case 3: error = L"Failed to create render target view."; break;
        //case 4: std::cout << "Failed to initilize AnselSDK." << std::endl; return;
        default: error = L"Unknown error."; break;
    }

    MessageBox(NULL, error, L"DirectX Initialization failed", MB_OK);

}

DirectXConfig* _DirectX::GetConfig() {
    return &cfg;
}
