#pragma once

#include <d3d11.h>
#include <stdint.h>
#include <locale>
#include <string>
#include <sstream>

size_t Format2BPP(DXGI_FORMAT format);
size_t Format2Ch(DXGI_FORMAT format);
DXGI_FORMAT Channel2Format(size_t channels, UINT bpc);
DXGI_FORMAT BPP2DepthFormat(UINT bpp);

float ieee_float(uint32_t f);
uint32_t ieee_uint32(float f);

// "dds", "obj", etc...
std::string_view path_ext(std::string_view str);

template<typename Dest, typename Src>
Dest ieee(Src f) {
    static_assert(sizeof(Dest) >= sizeof(f), "Src & Dest doesn't match size.");
    Dest ret;
    memcpy(&ret, &f, sizeof(Dest));
    return ret;
}

std::wstring widen(const std::string& str);
