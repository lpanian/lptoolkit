#pragma once

#include "aabb.hh"
#include "vec.hh"
#include "quaternion.hh"

namespace lptk
{
    template<class T>
    inline Box3<T> TransformBox(const Box3<T>& box,
        const lptk::vec3<T>& translate,
        const lptk::quaternion<T>& rotate,
        const lptk::vec3<T>& scale = lptk::vec3<T>{ 1 })
    {
        if (!box.Valid())
            return box;

        const auto lo = translate + lptk::Rotate(
            lptk::ComponentwiseMultiply(box.m_min, scale), rotate);
        const auto hi = translate + lptk::Rotate(
            lptk::ComponentwiseMultiply(box.m_max, scale), rotate);

        Box3<T> result;
        result.Extend(lptk::v3f{ lo[0], lo[1], lo[2] });
        result.Extend(lptk::v3f{ lo[0], lo[1], hi[2] });
        result.Extend(lptk::v3f{ lo[0], hi[1], hi[2] });
        result.Extend(lptk::v3f{ lo[0], hi[1], lo[2] });
        result.Extend(lptk::v3f{ hi[0], lo[1], lo[2] });
        result.Extend(lptk::v3f{ hi[0], lo[1], hi[2] });
        result.Extend(lptk::v3f{ hi[0], hi[1], hi[2] });
        result.Extend(lptk::v3f{ hi[0], hi[1], lo[2] });
        return result;
    }
}

