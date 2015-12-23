#pragma once
#ifndef INCLUDED_toolkit_quaternion_INL
#define INCLUDED_toolkit_quaternion_INL

#include "common.hh"
#include "matrix.hh"
#include "quaternion.hh"
#include "mathcommon.hh"

namespace lptk
{

template<class T>
quaternion<T>::quaternion(const mat44<T> &m)
{
    T x,y,z,w;
    T trace = T(1.0) + m.m[0] + m.m[5] + m.m[10];
    if(trace > T(1e-6)) {
        T s = Sqrt(trace) * T(2);
        x = (m.m[6] - m.m[9]) / s;
        y = (m.m[8] - m.m[2]) / s;
        z = (m.m[1] - m.m[4]) / s;
        w = T(0.25) * s;
    } else if(m.m[0] > m.m[5] && m.m[0] > m.m[10]) {
        T s = Sqrt(T(1.0) + m.m[0] - m.m[5] - m.m[10] ) * T(2);
        x = T(0.25) * s;
        y = (m.m[1] + m.m[4]) / s;
        z = (m.m[8] + m.m[2]) / s;
        w = (m.m[6] - m.m[9]) / s;
    } else if(m.m[5] > m.m[10]) {
        T s = Sqrt(T(1.0) + m.m[5] - m.m[0] - m.m[10] ) * T(2);
        x = (m.m[1] + m.m[4]) / s;
        y = T(0.25) * s;
        z = (m.m[6] + m.m[9]) / s;
        w = (m.m[8] - m.m[2]) / s;
    } else {
        T s = Sqrt(T(1.0) + m.m[10] - m.m[0] - m.m[5]) * T(2);
        x = (m.m[8] + m.m[2]) / s;
        y = (m.m[6] + m.m[9]) / s;
        z = T(0.25) * s;
        w = (m.m[1] - m.m[4]) / s;
    }
    a = x;
    b = y;
    c = z;
    r = w;
}

template<class T>
mat44<T> quaternion<T>::ToMatrix() const 
{			
    T aa = a*a;
    T ab = a*b;
    T ac = a*c;
    T ar = a*r;

    T bb = b*b;
    T bc = b*c;
    T br = b*r;

    T cc = c*c;
    T cr = c*r;

    mat44<T> result;

    result.m[0] = T(1) - T(2) * (bb + cc);
    result.m[1] = T(2) * (ab + cr);
    result.m[2] = T(2) * (ac - br);
    result.m[3] = T(0);

    result.m[4] = T(2) * (ab - cr);
    result.m[5] = T(1) - T(2) * (aa + cc);
    result.m[6] = T(2) * (bc + ar);
    result.m[7] = T(0);

    result.m[8] =  T(2) * (ac + br);
    result.m[9] =  T(2) * (bc - ar);
    result.m[10] = T(1) - T(2) * (aa + bb);
    result.m[11] = T(0);

    result.m[12] = T(0);
    result.m[13] = T(0);
    result.m[14] = T(0);
    result.m[15] = T(1);

    return result;
}

template<class T>
quaternion<T> Slerp(
 	const quaternion<T>&  left,
 	const quaternion<T>&  right,
 	T param)
{
    const T qdot = Clamp(Dot(left, right), T(0), T(1));
    const T n = qdot < T(0) ? T(-1) : T(1);

    const T theta = Acos(qdot);
    const T sin_theta = Sin(theta);
    if(sin_theta == T(0))
        return left;
    const T a = Sin(theta * (T(1) - param)) / sin_theta;
    const T b = n * Sin(theta * param) / sin_theta;

    quaternion<T> result = a * left + b * right;
    ASSERT(result.AllNumeric());
    return result;
}

template<class T>
quaternion<T> MakeRotation(T radians, const vec3<T>& vec) 
{
    T half_angle = radians* T(0.5);
    T cos_a = Cos(half_angle);
    T sin_a = Sin(half_angle);
    T r = cos_a;
    T a = vec.x * sin_a;
    T b = vec.y * sin_a;
    T c = vec.z * sin_a;
    return Normalize(quaternion<T>(a,b,c,r));
}

template<class T>
void GetAxisAngle(const quaternion<T>& q, vec3<T>& axis, T &rotation)
{
    rotation = Acos(q.r);
    T sin_a = Sin(rotation);
    rotation *= T(2);
    if (sin_a == 0)
    {
        axis.Set(1, 0, 0);
    }
    else
    {
        axis.x = q.a / sin_a;
        axis.y = q.b / sin_a;
        axis.z = q.c / sin_a;
    }
}

}

#endif

