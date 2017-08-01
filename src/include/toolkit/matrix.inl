#pragma once
#ifndef INCLUDED_toolkit_matrix_INL
#define INCLUDED_toolkit_matrix_INL

#include <cstring>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include "matrix.hh"
#include "vec.hh"
#include "common.hh"
#include "mathcommon.hh"

namespace lptk
{
////////////////////////////////////////////////////////////////////////////////
template<class T>
mat33<T>::mat33(const T* m_)
{ 
	std::copy(m_, m_+9, m);
}
	
template<class T>
inline mat33<T>::mat33() {}

template<class T>
mat33<T>::mat33(IdentityType) { 
	bzero(m, sizeof(m));
	m[8] = m[4] = m[0] = 1.f;
}

template<class T>
mat33<T>::mat33(
    T m0, T m1, T m2,
    T m3, T m4, T m5,
    T m6, T m7, T m8)
{
    m[0] = m0;
    m[1] = m1;
    m[2] = m2;

    m[3] = m3;
    m[4] = m4;
    m[5] = m5;

    m[6] = m6;
    m[7] = m7;
    m[8] = m8;
}
   
template<class T>
mat33<T>::mat33(
    const vec3<T>& col0,
    const vec3<T>& col1,
    const vec3<T>& col2)
{
    m[0] = col0.x;
    m[1] = col0.y;
    m[2] = col0.z;
    
    m[3] = col1.x;
    m[4] = col1.y;
    m[5] = col1.z;

    m[6] = col2.x;
    m[7] = col2.y;
    m[8] = col2.z;
}

template<class T>
mat33<T>::mat33(
    ByRowsType,
    const vec3<T>& row0,
    const vec3<T>& row1,
    const vec3<T>& row2)
{
    m[0] = row0.x;
    m[1] = row1.x;
    m[2] = row2.x;
    
    m[3] = row0.y;
    m[4] = row1.y;
    m[5] = row2.y;

    m[6] = row0.z;
    m[7] = row1.z;
    m[8] = row2.z;
}

template<class T>
inline void mat33<T>::Copy(const mat33<T>& r)
{
	memcpy(m, r.m, sizeof(m));
}

template<class T>
inline mat33<T>& mat33<T>::operator=(const mat33<T>& r) 
{
	if(this != &r)
		Copy(r);
	return *this;
}
	
template<class T>
inline mat33<T> mat33<T>::operator*(const mat33<T>& r) const
{
	mat33<T> result;
	for(int off = 0; off < 9; off+=3)
	{
		result.m[off] = 0.f; result.m[off+1] = 0.f; result.m[off+2] = 0.f; 
		for(int i = 0, j = 0; j < 3; ++j)
		{
			const auto bval = r.m[off+j];
			result.m[off]   += bval * m[i]; ++i;
			result.m[off+1] += bval * m[i]; ++i;
			result.m[off+2] += bval * m[i]; ++i;
		}
	}
	return result;
}

template<class T>
mat33<T> mat33<T>::operator*(T r) const
{
    mat33<T> result = *this;
    for(int i = 0; i < 9; ++i)
        result.m[i] *= r;
    return result;
}

template<class T>
mat33<T>& mat33<T>::operator*=(T r) 
{
    for(int i = 0; i < 9; ++i)
        m[i] *= r;
    return *this;
}

template<class T>
inline mat33<T>& mat33<T>::operator*=(const mat33<T>& r) 
{
	mat33<T> cur = *this;
	for(int off = 0; off < 9; off+=3)
	{
		m[off] = 0.f; m[off+1] = 0.f; m[off+2] = 0.f; 
		for(int i = 0, j = 0; j < 3; ++j)
		{
			const auto bval = r.m[off+j];
			m[off]   += bval * cur.m[i]; ++i;
			m[off+1] += bval * cur.m[i]; ++i;
			m[off+2] += bval * cur.m[i]; ++i;
		}
	}
	return *this;
}

template<class T>
mat33<T> mat33<T>::operator+(const mat33<T>& r) const
{
    mat33<T> result = *this;
    for(int i = 0; i < 9; ++i) {
        result.m[i] += r.m[i];
    }
    return result;
}

template<class T>
mat33<T>& mat33<T>::operator+=(const mat33<T>& r) 
{
    for(int i = 0; i < 9; ++i) {
        this->m[i] += r.m[i];
    }
    return *this;
}

template<class T>
mat33<T> mat33<T>::operator-(const mat33<T>& r) const
{
    mat33<T> result = *this;
    for(int i = 0; i < 9; ++i) {
        result.m[i] -= r.m[i];
    }
    return result;
    
}

template<class T>
mat33<T>& mat33<T>::operator-=(const mat33<T>& r) 
{
    for(int i = 0; i < 9; ++i) {
        this->m[i] -= r.m[i];
    }
    return *this;
}

template<class T>
inline vec3<T> mat33<T>::Col(int idx) const
{
	const int off = idx * 3;
	return vec3<T>(m[off], m[off+1], m[off+2]);
}

template<class T>
inline vec3<T> mat33<T>::Row(int idx) const
{
	return vec3<T>(m[idx], m[idx+3], m[idx+6]);	
}
	
template<class T>
bool mat33<T>::Equal(const mat33<T>& other, T eps) const
{
    return NormDiffSq(*this, other) <= eps;
}

template<class T>
inline std::ostream& operator<<(std::ostream& s, const mat33<T>& mat)
{
	for(int i = 0; i < 3; ++i)
	{
		std::cout << "[ " << mat.m[i] << ", " << mat.m[i+3] << 
			", " << mat.m[i+6] << " ]" << std::endl;
	}
	return s;
}

template<class T>
inline T NormDiffSq(const mat33<T>& a, const mat33<T>& b)
{
    T result = T(0);
    for(int i = 0; i < 9; i+=3) {
        const vec3<T> leftCol(a[i], a[i+1], a[i+2]);
        const vec3<T> rightCol(b[i], b[i+1], b[i+2]);
        const vec3<T> diff = leftCol - rightCol;
        const auto diffSq = LengthSq(diff);
        if(diffSq > result) {
            result = diffSq;
        }
    }
    return result;
}

template<class T>
inline T NormDiff(const mat33<T>& a, const mat33<T>& b) {
    return Sqrt(NormDiffSq(a, b));
}

template<class T>
inline mat33<T> operator*(T l, const mat33<T>& r)
{
    return r * l;
}

////////////////////////////////////////////////////////////////////////////////
template<class T>
mat44<T>::mat44(const T* m_)
{ 
	std::copy(m_, m_+16, m);
}
	
template<class T>
inline mat44<T>::mat44() {}

template<class T>
mat44<T>::mat44(IdentityType) { 
	bzero(m, sizeof(m));
	m[15] = m[10] = m[5] = m[0] = 1.f;
}

template<class T>
mat44<T>::mat44(T m0, T m1, T m2, T m3,
    T m4, T m5, T m6, T m7,
    T m8, T m9, T m10, T m11,
    T m12, T m13, T m14, T m15)
{
    m[0] = m0;
    m[1] = m1;
    m[2] = m2;
    m[3] = m3;

    m[4] = m4;
    m[5] = m5;
    m[6] = m6;
    m[7] = m7;

    m[8] = m8;
    m[9] = m9;
    m[10] = m10;
    m[11] = m11;

    m[12] = m12;
    m[13] = m13;
    m[14] = m14;
    m[15] = m15;
}
    
template<class T>
mat44<T>::mat44(
    const vec4<T>& col0,
    const vec4<T>& col1,
    const vec4<T>& col2,
    const vec4<T>& col3)
{
    m[0] = col0.x;
    m[1] = col0.y;
    m[2] = col0.z;
    m[3] = col0.w;

    m[4] = col1.x;
    m[5] = col1.y;
    m[6] = col1.z;
    m[7] = col1.w;

    m[8] =  col2.x;
    m[9] =  col2.y;
    m[10] = col2.z;
    m[11] = col2.w;

    m[12] = col3.x;
    m[13] = col3.y;
    m[14] = col3.z;
    m[15] = col3.w;
}

template<class T>
mat44<T>::mat44(
    ByRowsType,
    const vec4<T>& row0,
    const vec4<T>& row1,
    const vec4<T>& row2,
    const vec4<T>& row3)
{
    m[0] = row0.x;
    m[1] = row1.x;
    m[2] = row2.x;
    m[3] = row3.x;

    m[4] = row0.y;
    m[5] = row1.y;
    m[6] = row2.y;
    m[7] = row3.y;

    m[8] =  row0.z;
    m[9] =  row1.z;
    m[10] = row2.z;
    m[11] = row3.z;

    m[12] = row0.w;
    m[13] = row1.w;
    m[14] = row2.w;
    m[15] = row3.w;
}
    
template<class T>
mat44<T>::mat44(const mat33<T>& m33)
{
    m[0] = m33.m[0];
    m[1] = m33.m[1];
    m[2] = m33.m[2];
    m[3] = 0;
    
    m[4] = m33.m[3];
    m[5] = m33.m[4];
    m[6] = m33.m[5];
    m[7] = 0;
    
    m[8] = m33.m[6];
    m[9] = m33.m[7];
    m[10] = m33.m[8];
    m[11] = 0;

    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}

template<class T>
inline void mat44<T>::Copy(const mat44<T>& r)
{
	memcpy(m, r.m, sizeof(m));
}

template<class T>
inline mat44<T>& mat44<T>::operator=(const mat44<T>& r) 
{
	if(this != &r)
		Copy(r);
	return *this;
}
	
template<class T>
inline mat44<T> mat44<T>::operator*(const mat44<T>& r) const
{
	mat44<T> result;
	for(int off = 0; off < 16; off+=4)
	{
		result.m[off] = 0.f; result.m[off+1] = 0.f; result.m[off+2] = 0.f; result.m[off+3] = 0.f;
		for(int i = 0, j = 0; j < 4; ++j)
		{
			T bval = r.m[off+j];
			result.m[off]   += bval * m[i]; ++i;
			result.m[off+1] += bval * m[i]; ++i;
			result.m[off+2] += bval * m[i]; ++i;
			result.m[off+3] += bval * m[i]; ++i;
		}
	}
	return result;
}

template<class T>
mat44<T> mat44<T>::operator*(T r) const
{
    mat44<T> result = *this;
    for(int i = 0; i < 16; ++i)
        result.m[i] *= r;
    return result;
}

template<class T>
mat44<T>& mat44<T>::operator*=(T r) 
{
    for(int i = 0; i < 16; ++i)
        m[i] *= r;
    return *this;
}

template<class T>
inline mat44<T>& mat44<T>::operator*=(const mat44<T>& r) 
{
	mat44<T> cur = *this;
	for(int off = 0; off < 16; off+=4)
	{
		m[off] = 0.f; m[off+1] = 0.f; m[off+2] = 0.f; m[off+3] = 0.f;
		for(int i = 0, j = 0; j < 4; ++j)
		{
			T bval = r.m[off+j];
			m[off]   += bval * cur.m[i]; ++i;
			m[off+1] += bval * cur.m[i]; ++i;
			m[off+2] += bval * cur.m[i]; ++i;
			m[off+3] += bval * cur.m[i]; ++i;
		}
	}
	return *this;
}

template<class T>
mat44<T> mat44<T>::operator+(const mat44<T>& r) const
{
    mat44<T> result = *this;
    for(int i = 0; i < 16; ++i) {
        result.m[i] += r.m[i];
    }
    return result;
}

template<class T>
mat44<T>& mat44<T>::operator+=(const mat44<T>& r) 
{
    for(int i = 0; i < 16; ++i) {
        this->m[i] += r.m[i];
    }
    return *this;
}

template<class T>
mat44<T> mat44<T>::operator-(const mat44<T>& r) const
{
    mat44<T> result = *this;
    for(int i = 0; i < 16; ++i) {
        result.m[i] -= r.m[i];
    }
    return result;
    
}

template<class T>
mat44<T>& mat44<T>::operator-=(const mat44<T>& r) 
{
    for(int i = 0; i < 16; ++i) {
        this->m[i] -= r.m[i];
    }
    return *this;
}

template<class T>
inline vec4<T> mat44<T>::Col(int idx) const
{
	const int off = idx << 2;
	return vec4<T>(m[off], m[off+1], m[off+2], m[off+3]);
}

template<class T>
inline vec4<T> mat44<T>::Row(int idx) const
{
	return vec4<T>(m[idx], m[idx+4], m[idx+8], m[idx+12]);	
}
	
template<class T>
bool mat44<T>::Equal(const mat44<T>& other, T eps) const
{
    return NormDiffSq(*this, other) <= eps;
}

template<class T>
inline std::ostream& operator<<(std::ostream& s, const mat44<T>& mat)
{
	for(int i = 0; i < 4; ++i)
	{
		std::cout << "[ " << mat.m[i] << ", " << mat.m[i+4] << 
			", " << mat.m[i+8] << ", " << mat.m[i+12] << " ]" << std::endl;
	}
	return s;
}

template<class T>
inline T NormDiffSq(const mat44<T>& a, const mat44<T>& b)
{
    T result = T(0);
    for(int i = 0; i < 16; i+=4) {
        const vec4<T> leftCol(a[i], a[i+1], a[i+2], a[i+3]);
        const vec4<T> rightCol(b[i], b[i+1], b[i+2], b[i+3]);
        const vec4<T> diff = leftCol - rightCol;
        const T diffSq = LengthSq(diff);
        if(diffSq > result) {
            result = diffSq;
        }
    }
    return result;
}

template<class T>
inline T NormDiff(const mat44<T>& a, const mat44<T>& b) {
    return Sqrt(NormDiffSq(a, b));
}

template<class T>
inline mat44<T> operator*(T l, const mat44<T>& r)
{
    return r * l;
}


////////////////////////////////////////////////////////////////////////////////
}

#endif

