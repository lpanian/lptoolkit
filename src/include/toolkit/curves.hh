#pragma once

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    // curve helpers that don't rely on the vector classes
    namespace curves
    {
        // tvec should have 4 elements, and t should be between 0 and 1 
        inline void PrepareHermiteParameters(float* tvec, float t)
        {
            const auto tt = t*t;
            const auto ttt = tt*t;
            tvec[0] = 2.f * ttt - 3.f * tt + 1.f;
            tvec[1] = ttt - 2.f * tt + t;
            tvec[2] = -2.f * ttt + 3.f * tt;
            tvec[3] = ttt - tt;
        }

        // tvec must have 4 elements, all other pointer parameters must have 'count' elements
        inline void EvaluateHermiteSpline(
            float* result,
            const float* tvec,
            const float* p0,
            const float* p1,
            const float* m0,
            const float* m1,
            unsigned count)
        {
            for (unsigned i = 0; i < count; ++i)
            {
                result[i] =
                    p0[i] * tvec[0] +
                    m0[i] * tvec[1] +
                    p1[i] * tvec[2] +
                    m1[i] * tvec[3];
            }
        }

        // m0 and m1 should have at least valLen elements, keys should have 4 elements, and values should have 4 * valStride elements.
        // Keys/values contains the key/value pairs of the 4 interpolation points (p(t-1), p(t), p(t+1), p(t+2)). 
        // The key in this case is usually time.
        inline void ComputeCatmullRomSlopes(
            float* m0,
            float* m1,
            const float* keys,
            const float* values,
            unsigned valLen,
            unsigned valStride)
        {
            const auto interval = keys[2] - keys[1];
            const auto m0inv = interval / (keys[2] - keys[0]);
            const auto m1inv = interval / (keys[3] - keys[1]);

            const auto offset1 = valStride;
            const auto offset2 = offset1 + valStride;
            const auto offset3 = offset2 + valStride;
            for (unsigned i = 0; i < valLen; ++i)
            {
                m0[i] = m0inv * (values[offset2 + i] - values[i]);
                m1[i] = m1inv * (values[offset3 + i] - values[offset1 + i]);
            }
        }
    }
}
