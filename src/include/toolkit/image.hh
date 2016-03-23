#pragma once
// Very generic image representation.
//
// The image uses linear color.

#include <memory>

namespace lptk
{
    class Image
    {
    public:
        void Init(unsigned width, unsigned height, unsigned numChannels);
        void Clear();

        auto GetWidth() const { return m_width; }
        auto GetHeight() const { return m_height; }
        auto GetNumChannels() const { return m_numChannels; }

        void Set(unsigned x, unsigned y, unsigned channel, float value);
        float Get(unsigned x, unsigned y, unsigned channel) const;

        void FlipVertical() { m_inverted = !m_inverted; }
    private:
        unsigned m_width = 0;
        unsigned m_height = 0;
        unsigned m_numChannels = 0;
        unsigned m_lineStride = 0;
        bool m_inverted = false;

        std::unique_ptr<float[]> m_pixels;
    };
}


