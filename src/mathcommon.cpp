#include "toolkit/mathcommon.hh"

namespace lptk 
{
    bool SolveQuadratic(float a, float b, float c, float *t1, float *t2)
    {
        const float discrSq = (b*b - 4.0f * a * c);
        if(discrSq < 0.f) 
            return false;

        const float discr = Sqrt(discrSq);

        float q;
        if(b < 0.f) 
            q = 0.5f * (-b + discr);
        else
            q = 0.5f * (-b - discr);
        *t1 = q / a;
        *t2 = c / q;

        if(*t1 > *t2)
            Swap(*t1, *t2);
        return true;
    }

    bool SolveLinear2x2(const float* A, const float* b, float* x0, float *x1)
    {
        // a0 a2   x0 = b0
        // a1 a3   x1 = b1

        const float det = A[0] * A[3] - A[2] * A[1];  
        if(det == 0.f)
            return false;

        // b0 a2
        // b1 a3
        *x0 = (b[0] * A[3] - A[2] * b[1]) / det;
        
        // a0 b0
        // a1 b1
        *x1 = (A[0] * b[1] - b[0] * A[1]) / det;
        return true;
    }
}

