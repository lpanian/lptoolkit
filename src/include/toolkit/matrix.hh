#pragma once
#ifndef INCLUDED_toolkit_matrix_HH
#define INCLUDED_toolkit_matrix_HH

#include <cstring>
#include "vec.hh"

////////////////////////////////////////////////////////////////////////////////
// Matrix classes are both in column order (OpenGL) and all operations are 
// usually considered to be left multiplying the matrix, eg:
// v' = M * v

namespace lptk
{
////////////////////////////////////////////////////////////////////////////////
// Matrix class stored in column order:
// 0 3 6
// 1 4 7
// 2 5 8
template<class T>
class mat33
{
public:
    T m[9];
    typedef T baseType;
    enum IdentityType { identity };

    mat33();
    mat33(IdentityType);
    mat33(
        T m0, T m1, T m2,
        T m3, T m4, T m5,
        T m6, T m7, T m8);
    mat33<T>(const T* m_);
    inline mat33<T>(const mat33<T>& r) { Copy(r); }

    mat33<T>& operator=(const mat33<T>& r) ;
    mat33<T> operator*(const mat33<T>& r) const;
    mat33<T>& operator*=(const mat33<T>& r) ;
    mat33<T> operator*(T r) const;
    mat33<T>& operator*=(T r) ;
    mat33<T> operator+(const mat33<T>& r) const;
    mat33<T>& operator+=(const mat33<T>& r) ;
    mat33<T> operator-(const mat33<T>& r) const;
    mat33<T>& operator-=(const mat33<T>& r) ;

    T operator[](int index) const { return m[index]; }
    T& operator[](int index) { return m[index]; }

    void Copy(const mat33<T>& r);

    vec3<T> Col(int idx) const;
    vec3<T> Row(int idx) const;

    bool Equal(const mat33<T>& other, T eps = T(1e-3)) const;
};

template<class T>
std::ostream& operator<<(std::ostream& s, const mat33<T>& mat);

template<class T>
mat33<T> operator*(T l, const mat33<T>& r);

template<class T>
T NormDiff(const mat33<T>& a, const mat33<T>& b);

template<class T>
T NormDiffSq(const mat33<T>& a, const mat33<T>& b);


////////////////////////////////////////////////////////////////////////////////
// Matrix class stored in column order:
// 0 4 8  12
// 1 5 9  13
// 2 6 10 14
// 3 7 11 15
template<class T>
class mat44
{
public:
    T m[16];
    typedef T baseType;
    enum IdentityType { identity };

    mat44();
    mat44(IdentityType);
    mat44(
        T m0, T m1, T m2, T m3,
        T m4, T m5, T m6, T m7,
        T m8, T m9, T m10, T m11,
        T m12, T m13, T m14, T m15);
    mat44(const T* m_);
    mat44(const mat33<T>& m33);
    inline mat44(const mat44<T>& r) { Copy(r); }

    mat44& operator=(const mat44<T>& r) ;
    mat44 operator*(const mat44<T>& r) const;
    mat44& operator*=(const mat44<T>& r) ;
    mat44 operator*(T r) const;
    mat44& operator*=(T r) ;
    mat44 operator+(const mat44<T>& r) const;
    mat44& operator+=(const mat44<T>& r) ;
    mat44 operator-(const mat44<T>& r) const;
    mat44& operator-=(const mat44<T>& r) ;

    T operator[](int index) const { return m[index]; }
    T& operator[](int index) { return m[index]; }

    void Copy(const mat44<T>& r);

    vec4<T> Col(int idx) const;
    vec4<T> Row(int idx) const;

    bool Equal(const mat44<T>& other, T eps = T(1e-3)) const;
};

template<class T>
std::ostream& operator<<(std::ostream& s, const mat44<T>& mat);

template<class T>
mat44<T> operator*(T l, const mat44<T>& r);

template<class T>
T NormDiff(const mat44<T>& a, const mat44<T>& b);

template<class T>
T NormDiffSq(const mat44<T>& a, const mat44<T>& b);



////////////////////////////////////////////////////////////////////////////////
typedef mat33<float> m33f;
typedef mat33<double> m33d;
typedef mat44<float> m44f;
typedef mat44<double> m44d;
}

#include "matrix.inl"


#endif


