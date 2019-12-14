#pragma once

#include <d3d11.h>

size_t Format2BPP(DXGI_FORMAT format);
size_t Format2Ch(DXGI_FORMAT format);
DXGI_FORMAT Channel2Format(size_t channels, UINT bpc);
DXGI_FORMAT BPP2DepthFormat(UINT bpp);

