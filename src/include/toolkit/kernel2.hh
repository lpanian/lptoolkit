#pragma once

#include "toolkit/dynary.hh"

namespace lptk
{
    class Image;

    class Kernel2
    {
        lptk::DynAry<float> m_data;
        unsigned m_windowSize = 0;
    public:
        enum KernelType {
            Sobel,
        };
        Kernel2(KernelType type, unsigned dim);
        void Normalize();
        void Transpose();

        float Apply(const Image& img, int x, int y, int channel, bool bWrap = true) const;

    private:
        Kernel2(const Kernel2&) = delete;
        Kernel2& operator=(const Kernel2&) = delete;

        void InitSobel(unsigned dim);
    };
}


