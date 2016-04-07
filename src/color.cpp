#include "toolkit/color.hh"
#include "toolkit/memstream.hh"
#include "toolkit/serialize.hh"
#include "toolkit/image.hh"

namespace lptk
{

    static const float g_inv255 = 1.f / 255.f;
    Color::Color(const ColorRGBA& c)
        : r(float(c.r))
        , g(float(c.g))
        , b(float(c.b))
        , a(float(c.a))
    {
        r *= g_inv255; g *= g_inv255; b *= g_inv255; a *= g_inv255;
    }

    ColorRGBA::ColorRGBA(const Color& c)
        : r(uint8_t(Clamp(int(c.r * 255.f), 0, 255)))
        , g(uint8_t(Clamp(int(c.g * 255.f), 0, 255)))
        , b(uint8_t(Clamp(int(c.b * 255.f), 0, 255)))
        , a(uint8_t(Clamp(int(c.a * 255.f), 0, 255)))
    {}

    void Color::SerializeTo(MemSerializer& serializer) const
    {
        serializer.PutColor(*this);
    }

    void ColorRGBA::SerializeTo(MemSerializer& serializer) const
    {
        serializer.PutColorRGBA(*this);
    }

    namespace color
    {
        void ConvertLinearFromSRGB(Image& img)
        {
            const auto numCh = lptk::Min(3u, img.GetNumChannels());
            for (auto y = 0u; y < img.GetHeight(); ++y)
            {
                for (auto x = 0u; x < img.GetWidth(); ++x)
                {
                    for (auto ch = 0u; ch < numCh; ++ch)
                    {
                        const auto csrgb = img.Get(x, y, ch);
                        if (csrgb <= 0.04045f)
                        {
                            img.Set(x, y, ch, csrgb / 12.92f);
                        }
                        else
                        {
                            img.Set(x, y, ch, lptk::Pow((csrgb + 0.055f) / (1.055f), 2.4f));
                        }
                    }
                }
            }
        }

        void ConvertSRGBFromLinear(Image& img)
        {
            const auto numCh = lptk::Min(3u, img.GetNumChannels());
            for (auto y = 0u; y < img.GetHeight(); ++y)
            {
                for (auto x = 0u; x < img.GetWidth(); ++x)
                {
                    for (auto ch = 0u; ch < numCh; ++ch)
                    {
                        const auto clin = img.Get(x, y, ch);
                        if (clin <= 0.0031308)
                        {
                            img.Set(x, y, ch, 12.92f * clin);
                        }
                        else
                        {
                            img.Set(x, y, ch, (1 + 0.055f) * lptk::Pow(clin, 1 / 2.4f) - 0.055f);
                        }
                    }
                }
            }
        }
    }
}

