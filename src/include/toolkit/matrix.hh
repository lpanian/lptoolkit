#pragma once
#ifndef INCLUDED_toolkit_matrix_HH
#define INCLUDED_toolkit_matrix_HH

#include <cstring>
#include "vec.hh"

////////////////////////////////////////////////////////////////////////////////
template<class T>
class mat44
{
public:
	T m[16];
	typedef T baseType;
	enum IdentityType { identity };

	mat44<T>();
	mat44<T>(IdentityType);
	mat44<T>(T m0, T m1, T m2, T m3,
		T m4, T m5, T m6, T m7,
		T m8, T m9, T m10, T m11,
		T m12, T m13, T m14, T m15);
	mat44<T>(const T* m_);
	inline mat44<T>(const mat44<T>& r) { Copy(r); }

	mat44<T>& operator=(const mat44<T>& r) ;
	mat44<T> operator*(const mat44<T>& r) const;
	mat44<T>& operator*=(const mat44<T>& r) ;

	T operator[](int index) const { return m[index]; }
	T& operator[](int index) { return m[index]; }

	void Copy(const mat44<T>& r);

	vec4<T> Col(int idx) const;
	vec4<T> Row(int idx) const;

	bool Equal(const mat44<T>& other, float eps = 1e-3f) const;
};
	
template<class T>
std::ostream& operator<<(std::ostream& s, const mat44<T>& mat);

#include "matrix.inl"

////////////////////////////////////////////////////////////////////////////////
typedef mat44<float> m44f;
typedef mat44<double> m44d;

#endif


