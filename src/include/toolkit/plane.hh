#pragma once
#ifndef INCLUDED_toolkit_plane_HH
#define INCLUDED_toolkit_plane_HH

#include "vec.hh"
#include "matrix.hh"

namespace lptk
{
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class plane
{
    lptk::vec4<T> m_plane; // xyz = n, w = d
public:

    plane() {}
    plane(const vec3<T>& n, float d)
        : m_plane(n.x, n.y, n.z, d)
    {}
    plane(const vec3<T>& n, const vec3<T>& p)
    {
        : m_plane(n.x, n.y, n.z, -Dot(n, p))
    }
    plane(const vec4<T>& p) : m_plane(p) { }

    void Set(const vec3<T>& n, float d)
    {
        m_plane.x = n.x;
        m_plane.y = n.y;
        m_plane.z = n.z;
        m_plane.w = d;
    }
    
    void Set(const vec3<T>& n, const vec3<T>& p)
    {
        m_plane.x = n.x;
        m_plane.y = n.y;
        m_plane.z = n.z;
        m_plane.w = -Dot(n, p);
    }

    const vec4<T>& AsVec4() const { return m_plane; }
    vec3<T> N() const { return ToVec3(m_plane); }
    float D() const { return m_plane.w; }
	
    // Prefer this instead of inside/outside so caller can choose whether
    // to include the plane or not in the comparison
    T Distance(const vec3<T>& p) const
    {
        return p.x * m_plane.x + 
            p.y * m_plane.y + 
            p.z * m_plane.z + 
            m_plane.w;
    }
};

template<typename T>
plane<T> TransformPlane(const mat44<T>& mat, const plane<T>& plane);
template<typename T>
void TransformPlanes(
    plane<T>* outPlanes,
    const mat44<T>& mat, const plane<T>* planes, size_t numPlanes);
template<typename T>
void TransformPlanes(
    const mat44<T>& mat, plane<T>* planes, size_t numPlanes);

template<typename T>
void ClipLine(const plane<T>& plane,
    vec3<T>& from,
    vec3<T>& to);

template<typename T>
bool IntersectPlane(const plane<T>& plane,
    const vec3<T>& from,
    const vec3<T>& dir,
    float& t);

// TODO: Frustum Planes from proj matrix
// TODO: Frustum Planes from fov/aspec

////////////////////////////////////////////////////////////////////////////////
typedef plane<float> planef;
typedef plane<double> planed;

}

#include "plane.inl"
#endif

