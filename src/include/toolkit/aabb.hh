#pragma once
#ifndef INCLUDED_toolkit_aabb_HH
#define INCLUDED_toolkit_aabb_HH

#include <limits>
#include <cfloat>
#include "vec.hh"

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    template< class T >
    class Box2
    {
    public:
        vec2<T> m_min;
        vec2<T> m_max;
        Box2() : m_min(std::numeric_limits<T>::max()), m_max(std::numeric_limits<T>::lowest()) {}

        void Extend(const vec2<T>& v)
        {
            m_min = Min(m_min, v);
            m_max = Max(m_max, v);
        }

        vec2<T> Center() const
        {
            return 0.5f * m_min + 0.5f * m_max;
        }

        int MajorAxis() const;

        // return true if the bounds contains something
        bool Valid() const
        {
            return m_min.x <= m_max.x && m_min.y <= m_max.y;
        }

        bool HasVolume() const
        {
            return m_min.x < m_max.x && m_min.y < m_max.y;
        }

        const vec2<T>& operator[](int i) const { return i == 0 ? m_min : m_max; }
        vec2<T>& operator[](int i) { return i == 0 ? m_min : m_max; }
    };

    template<class T>
    inline int Box2<T>::MajorAxis() const
    {
        float bestDist = m_max[0] - m_min[0];
        int best = 0;
        for (int i = 1; i < 2; ++i)
        {
            const float dist = m_max[i] - m_min[i];
            if (dist > bestDist)
            {
                bestDist = dist;
                best = i;
            }
        }
        return best;
    }

    template<class T>
    inline Box2<T> Intersection(const Box2<T>& left, const Box2<T>& right)
    {
        Box2<T> result;
        result.m_min = Max(left.m_min, right.m_min);
        result.m_max = Min(left.m_max, right.m_max);
        return result;
    }

    template<class T>
    inline Box2<T> Union(const Box2<T>& left, const Box2<T>& right)
    {
        Box2<T> result;
        result.m_min = Min(left.m_min, right.m_min);
        result.m_max = Max(left.m_max, right.m_max);
        return result;
    }

    ////////////////////////////////////////////////////////////////////////////////
    template< class T >
    class Box3
    {
    public:
        vec3<T> m_min;
        vec3<T> m_max;
        Box3() : m_min(std::numeric_limits<T>::max()), m_max(std::numeric_limits<T>::lowest()) {}

        inline void Extend(const vec3<T>& v)
        {
            m_min = Min(m_min, v);
            m_max = Max(m_max, v);
        }

        vec3<T> Center() const
        {
            return T(0.5) * m_min + T(0.5) * m_max;
        }

        T SurfaceArea() const
        {
            // Surface area of 3D box: 2 * (surface area of each face)
            const vec3<T> dims = m_max - m_min;
            return T(2) * (dims.y * dims.z +
                dims.x * dims.z +
                dims.x * dims.y);
        }

        int MajorAxis() const;

        // return true if the bounds contains something
        bool Valid() const
        {
            return m_min.x <= m_max.x && m_min.y <= m_max.y && m_min.z <= m_max.z;
        }
        
        bool HasVolume() const
        {
            return m_min.x < m_max.x && m_min.y < m_max.y && m_min.z < m_max.z;
        }

        const vec3<T>& operator[](int i) const { return i == 0 ? m_min : m_max; }
        vec3<T>& operator[](int i) { return i == 0 ? m_min : m_max; }
    };

    template<class T>
    inline int Box3<T>::MajorAxis() const
    {
        float bestDist = m_max[0] - m_min[0];
        int best = 0;
        for (int i = 1; i < 3; ++i)
        {
            const float dist = m_max[i] - m_min[i];
            if (dist > bestDist)
            {
                bestDist = dist;
                best = i;
            }
        }
        return best;
    }

    template<class T>
    inline Box3<T> Intersection(const Box3<T>& left, const Box3<T>& right)
    {
        Box3<T> result;
        result.m_min = Max(left.m_min, right.m_min);
        result.m_max = Min(left.m_max, right.m_max);
        return result;
    }

    template<class T>
    inline Box3<T> Union(const Box3<T>& left, const Box3<T>& right)
    {
        Box3<T> result;
        result.m_min = Min(left.m_min, right.m_min);
        result.m_max = Max(left.m_max, right.m_max);
        return result;
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class Vec>
    struct BoxType { };
    template<class T>
    struct BoxType<vec2<T> > { typedef Box2<T> type; };
    template<class T>
    struct BoxType<vec3<T> > { typedef Box3<T> type; };

    ////////////////////////////////////////////////////////////////////////////////
    typedef Box2<int> Box2i;
    typedef Box2<float> Box2f;
    typedef Box2<double> Box2d;
    typedef Box3<int> Box3i;
    typedef Box3<float> Box3f;
    typedef Box3<double> Box3d;
   
    // the rest of the library uses this convention
    typedef Box2<int> box2i;
    typedef Box2<float> box2f;
    typedef Box2<double> box2d;
    typedef Box3<int> box3i;
    typedef Box3<float> box3f;
    typedef Box3<double> box3d;

}

#endif
