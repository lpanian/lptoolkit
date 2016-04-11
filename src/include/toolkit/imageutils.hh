#pragma once

#include "toolkit/image.hh"
#include "toolkit/dynary.hh"

namespace lptk
{
    bool LoadTGA(Image& result, const char* filename);
    bool LoadTGA(Image& result, const char* memory, size_t n);
    bool SaveTGA(const Image& image, const char* filename);
    bool SaveTGA(const Image& image, lptk::DynAry<char> mem);

    bool LoadTIFF(Image& result, const char* filename);
    bool LoadTIFF(Image& result, const char* memory, size_t n);
}
