#include "toolkit/kernel2.hh"
#include "toolkit/mathcommon.hh"
#include "toolkit/image.hh"

namespace lptk
{
    Kernel2::Kernel2(KernelType type, unsigned dim)
    {
        switch (type)
        {
        case KernelType::Sobel:
            InitSobel(dim);
            break;
        default:
            break;
        }
        Normalize();
    }

	void Kernel2::Normalize()
	{
		auto total = 0.f;
		for(auto i = 0u; i < m_windowSize * m_windowSize; ++i)
			total += fabs(m_data[i]);

		const auto inv_total = 1.f / total;
		for(auto i = 0u; i < m_windowSize * m_windowSize; ++i)
			m_data[i] = m_data[i] * inv_total;
	}
	
	void Kernel2::Transpose()
	{
		for(auto i = 0u; i < m_windowSize; ++i)
			for(auto j = i+1; j < m_windowSize; ++j)
				Swap(m_data[i * m_windowSize + j], m_data[j * m_windowSize + i]);
	}

    static inline float SobelSeq(unsigned x, unsigned halfDim)
    {
        return x <= halfDim ? float(x) : float(halfDim - x);
    }
        
    void Kernel2::InitSobel(unsigned dim)
    {
        m_windowSize = dim;
        m_data.resize(m_windowSize * m_windowSize);

        const auto halfDim = m_windowSize / 2;
        auto offset = unsigned(0);
        for (auto y = 0u; y < dim; ++y)
        {
            for (auto x = 0u; x < dim; ++x)
            {
                if (x == halfDim)
                {
                    m_data[offset++] = 0.f;
                }
                else
                {
                    const auto signMult = x < halfDim ? -1.f : 1.f;
                    m_data[offset++] = signMult *
                        (SobelSeq(x, halfDim) + 1.f + SobelSeq(y, halfDim));
                }
            }
        }
    }

	float Kernel2::Apply(const Image& img, int x, int y, int channel, bool bWrap) const
	{
		const auto windowSize = m_windowSize;
		const auto windowOffset = windowSize / 2;

		float result = 0.f;
		for(auto j = 0u; j < windowSize; ++j)
		{
			for(auto i = 0u; i < windowSize; ++i)
			{
                auto xpos = x - windowOffset + i;
                auto ypos = y - windowOffset + j;
                if (bWrap)
                {
                    while (xpos < 0) xpos += img.GetWidth();
                    while (xpos >= img.GetWidth()) xpos -= img.GetWidth();

                    while (ypos < 0) ypos += img.GetHeight();
                    while (ypos >= img.GetHeight()) ypos -= img.GetHeight();
                }
				const auto val = img.Get(xpos, ypos, channel);
				result += m_data[i + j * windowSize] * val;
			}
		}
		return result;
	}
}


