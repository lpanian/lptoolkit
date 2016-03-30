#include "toolkit/image.hh"

namespace lptk
{
    Image::Image(Image&& other)
        : m_width(0)
        , m_height(0)
        , m_numChannels(0)
        , m_lineStride(0)
        , m_inverted(false)
        , m_pixels()
    {
        lptk::Swap(other.m_width, m_width);
        lptk::Swap(other.m_height, m_height);
        lptk::Swap(other.m_numChannels, m_numChannels);
        lptk::Swap(other.m_lineStride, m_lineStride);
        lptk::Swap(other.m_inverted, m_inverted);
        lptk::Swap(other.m_pixels, m_pixels);
    }

    Image& Image::operator=(Image&& other)
    {
        if (this != &other)
        {
            lptk::Swap(other.m_width, m_width);
            lptk::Swap(other.m_height, m_height);
            lptk::Swap(other.m_numChannels, m_numChannels);
            lptk::Swap(other.m_lineStride, m_lineStride);
            lptk::Swap(other.m_inverted, m_inverted);
            lptk::Swap(other.m_pixels, m_pixels);
        }
        return *this;
    }

    void Image::Init(unsigned width, unsigned height, unsigned numChannels)
    {
        const auto numValues = width * height * numChannels;
        m_pixels.reset(new float[numValues]);
        
        m_width = width;
        m_height = height;
        m_numChannels = numChannels;
        m_lineStride = m_width * m_numChannels;

        for (auto i = 0u; i < numValues; ++i)
            m_pixels[i] = 0.f;
    }
    void Image::Clear()
    {
        m_pixels.reset();
        m_width = 0;
        m_height = 0;
        m_numChannels = 0;
        m_lineStride = 0;
    }

    void Image::Set(unsigned x, unsigned y, unsigned channel, float value)
    {
        if (x < GetWidth() && y < GetHeight() && channel < m_numChannels)
        {
            if (m_inverted) y = GetHeight() - y - 1;
            const auto offset = y * m_lineStride + x * m_numChannels + channel;
            m_pixels[offset] = value;
        }
    }

    float Image::Get(unsigned x, unsigned y, unsigned channel) const
    {
        if (x < GetWidth() && y < GetHeight() && channel < m_numChannels)
        {
            if (m_inverted) y = GetHeight() - y - 1;
            const auto offset = y * m_lineStride + x * m_numChannels + channel;
            return m_pixels[offset];
        }
        return 0.f;
    }

}
