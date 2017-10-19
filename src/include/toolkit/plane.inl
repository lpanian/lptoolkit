#pragma once

#include <limits>

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

    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    void ClipLine(const plane<T>& plane,
        vec3<T>& from,
        vec3<T>& to)
    {
        float fromDist = plane.Distance(from);
        if (fromDist < 0.f)
            from = from - plane.N() * (fromDist - std::numeric_limits<T>::epsilon() * fromDist) ;

        float toDist = plane.Distance(to);
        if (toDist < 0.f)
            to = to - plane.N() * (toDist - std::numeric_limits<T>::epsilon() * toDist);
    }

    template<typename T>
    bool IntersectPlane(const plane<T>& plane,
        const vec3<T>& from,
        const vec3<T>& dir,
        float& t)
    {
        const auto N = plane.N();
        const auto dDotN = lptk::Dot(N, dir);
        if (dDotN == 0.0)
            return false;

        t = -(plane.D() + lptk::Dot(N, from)) / dDotN;
        return true;
    }
}
