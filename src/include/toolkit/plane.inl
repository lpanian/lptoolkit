#pragma once

#include "matrixutils.hh"

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    plane<T> TransformPlane(const mat44<T>& mat, const plane<T>& plane)
    {
        const auto inv = Transpose(TransformInverse(mat));
        return TransformVec(inv, plane.AsVec4());
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    void TransformPlanes(
        plane<T>* outPlanes,
        const mat44<T>& mat, const plane<T>* planes, size_t numPlanes)
    {
        const auto inv = Transpose(TransformInverse(mat));
        for (size_t i = 0; i < numPlanes; ++i)
        {
            outPlanes = TransformVec(inv, planes[i].AsVec4());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    void TransformPlanes(
        const mat44<T>& mat, plane<T>* planes, size_t numPlanes)
    {
        const auto inv = Transpose(TransformInverse(mat));
        for (size_t i = 0; i < numPlanes; ++i)
        {
            planes = TransformVec(inv, planes[i].AsVec4());
        }
    }

}
