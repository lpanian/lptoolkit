#include "toolkit/mathcommon.hh"
#include "toolkit/statistics.hh"
#include "toolkit/binsearch.hh"

namespace 
{
    static float kPiOver4 = lptk::kPi / 4.f;
}

namespace lptk
{
    //////////////////////////////////////////////////////////////////////////////// 
    // calculate p(x) as f(x) / int(0, 1, f(x))
    // approximate as 1/N sized blocks at the given values
    // calculate P(x) as p(x) * 1/N
    // Below factors out the integral (funcTotal), and divides through
    // by the total at the end.
    Distribution1D::Distribution1D(const float* vals, int count)
        : m_vals(vals, vals+count)
        , m_cdf(count+1)
        , m_funcTotal(0)
    {
        float funcTotal = 0.f;
        for(int i = 0; i < count; ++i)
        {
            m_cdf[i] = funcTotal;
            // each p(x) is v[i] / vTotal
            // each cdf entry is prev entry + v[i] / (vTotal * N)
            funcTotal += vals[i] / count; 
        }
        m_cdf[count] = funcTotal;
        m_funcTotal = funcTotal;

        const float invFuncTotal = 1.f / funcTotal;
        for(auto& val : m_cdf)
            val *= invFuncTotal;

        ASSERT(m_cdf[0] == 0.f);
        ASSERT(lptk::Abs(m_cdf[count] - 1.f) < 1e-3f);
        m_cdf[count] = 1.f;
    }


    float Distribution1D::SampleContinuous(float u, float* pdf) const
    {
        // binary search cdf values, return lower index
        const auto lowerIndex = Clamp(
            binSearchLower(m_cdf.begin(), m_cdf.end(), u) - m_cdf.begin(), 
            0, 
            int(m_vals.size() - 1));
        const auto upperIndex = Min(lowerIndex + 1, int(m_vals.size()) - 1);
        
        const auto cdfLo = m_cdf[lowerIndex];
        const auto cdfHi = m_cdf[upperIndex];

        const auto t = Clamp(u - cdfLo / (cdfHi - cdfLo), 0.f, 1.f);

        // probability of getting this value is the same for all values in this
        // 'bucket'. Integral of all pdf values should be 1.
        if(pdf)
            *pdf = m_vals[lowerIndex] / m_funcTotal;

        return (lowerIndex + t) / float(m_vals.size());
    }
        
    int Distribution1D::SampleDiscrete(float u, float* pdf) const
    {
        const auto lowerIndex = Clamp(
            binSearchLower(m_cdf.begin(), m_cdf.end(), u) - m_cdf.begin(), 
            0, 
            int(m_vals.size() - 1));

        // probability is the same as for continuous, but over a bucket of size 1/N
        // sum of all pdf values should sum to 1
        if(pdf)
            *pdf = m_vals[lowerIndex] / (m_funcTotal * m_vals.size());

        return lowerIndex;
    }

    //////////////////////////////////////////////////////////////////////////////// 
    void ConcentricSampleDisk(float u, float v, float *outU, float *outV)
    {
        const float x = 2.f * u - 1.f;
        const float y = 2.f * v - 1.f;

        if(x == 0.f && y == 0.f) 
        {
            *outU = 0.f;
            *outV = 0.f;
            return;
        }

        // break into quadrants, where each quadrant is centered over an axis and spans 90 degrees

        float r = 0.f, theta = 0.f;
        if(x >= -y) // above the x = -y line
        {
            if(x > y) // below the x=y line
            {
                r = x;
                if(y >= 0.f) theta = y / r;
                else theta = 8.f + y / r;
            }
            else // above the x=y line
            {
                r = y;
                theta = 2.f - x / r;
            }
        }
        else // below the x = -y line
        {
            if(x <= y)
            {
                r = -x;
                theta = 4.f - y / r;
            }
            else
            {
                r = -y;
                theta = 6.f + x / r;
            }
        }

        theta *= kPiOver4;
        *outU = r * Cos(theta);
        *outV = r * Sin(theta);
    }

    lptk::v3f CosineSampleHemisphere(float u0, float u1)
    {   
        lptk::v3f result;
        ConcentricSampleDisk(u0, u1, &result.x, &result.y);
        result.z = lptk::Sqrt(lptk::Max(0.f, 1.f - 
            result.x * result.x -
            result.y * result.y));
        return result;
    }
    
    lptk::v3f UniformSampleHemisphere(float u0, float u1)
    {
        const float z = u0;
        const float r = lptk::Sqrt(lptk::Max(0.f, 1.f - z*z));
        const float phi = 2 * kPi * u1;
        const float x = lptk::Cos(phi) * r;
        const float y = lptk::Sin(phi) * r;

        return lptk::v3f(x, y, z);
    }

}

