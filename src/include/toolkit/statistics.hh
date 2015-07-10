#pragma once
#ifndef INCLUDED_lptk_statistics_HH
#define INCLUDED_lptk_statistics_HH

#include "toolkit/dynary.hh"
#include "toolkit/vec.hh"

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    // Sample from a 1D function. Given a random variable between 0 and 1, this uses
    // an inverse lookup on the CDF of the function to find the appropriate function
    // value. 
    // This normalizes the funtion's domain to [0, 1]
    class Distribution1D
    {
    public:
        Distribution1D(const float* vals, int count);
        float SampleContinuous(float u, float* pdf = nullptr) const;
        int SampleDiscrete(float u, float* pdf = nullptr) const;

    private:
        DynAry<float> m_vals;
        DynAry<float> m_cdf;
        float m_funcTotal;
    };

    ////////////////////////////////////////////////////////////////////////////////
    class Distribution2D
    {
    public:
        Distribution2D(const float* vals, int nx, int ny);
    };

    ////////////////////////////////////////////////////////////////////////////////
    void ConcentricSampleDisk(float u, float v, float *outU, float *outV);

    lptk::v3f CosineSampleHemisphere(float u0, float u1);
    lptk::v3f UniformSampleHemisphere(float u0, float u1);
    
    
    
    ////////////////////////////////////////////////////////////////////////////////
    inline float PowerHeuristic(int nf, float pf, int ng, float pg)
    {
        const float f = nf * pf;
        const float g = ng * pg;
        const float fSq = f*f;
        const float gSq = g*g;
        return fSq / (fSq + gSq);
    }

    inline float BalanceHeuristic(int nf, float pf, int ng, float pg)
    {
        const float f = nf * pf;
        const float g = ng * pg;
        return f / (f + g);
    }
}

#endif

