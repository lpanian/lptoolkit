#pragma once
#ifndef INCLUDED_toolkit_vec_HH
#define INCLUDED_toolkit_vec_HH

#include <cmath>
#include <iostream>
#include "mathcommon.hh"

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
template<class T>
class vec2
{
public:
	typedef T baseType;
    static constexpr auto Dims = 2;

	T x, y;

	static inline int dims() { return Dims; }

	vec2() {}
	explicit vec2(T v) : x(v), y(v) {}
	vec2(T x_, T y_) : x(x_), y(y_) {}
	void Set(T x_, T y_) ;
	vec2 operator+(const vec2& r) const;
	vec2& operator+=(const vec2& r) ;
	vec2 operator-(const vec2& r) const;
	vec2& operator-=(const vec2& r) ;
	vec2 operator-() const;
	vec2 operator*(T s) const;
	vec2& operator*=(T s) ;
	vec2 operator/(T s) const;
	vec2& operator/=(T s) ;
	bool operator==(const vec2& r) const;
	bool operator!=(const vec2& r) const;
	T operator[](int i) const;
	T& operator[](int i) ;

	void Normalize();
	void NormalizeSafe(const vec2& def = vec2(T(1),T(0)));

	bool Equal(const vec2& other, float eps = 1e-3f) const;
};

template<class T>
inline void vec2<T>::Set(T x_, T y_) {
	x = x_; y = y_; 
}
	
template<class T>
inline vec2<T> vec2<T>::operator+(const vec2& r) const {
	return vec2(x + r.x, y + r.y);
}

template<class T>
inline vec2<T>& vec2<T>::operator+=(const vec2& r)  {
	x += r.x;
	y += r.y;
	return *this;
}

template<class T>
inline vec2<T> vec2<T>::operator-(const vec2& r) const {
	return vec2(x - r.x, y - r.y);
}

template<class T>
inline vec2<T>& vec2<T>::operator-=(const vec2& r)  {
	x -= r.x;
	y -= r.y;
	return *this;
}
	
template<class T>
inline vec2<T> vec2<T>::operator-() const {
	return vec2(-x, -y);
}

template<class T>
inline vec2<T> vec2<T>::operator*(T s) const
{
	return vec2(x*s, y*s);
}	

template<class T>
inline vec2<T>& vec2<T>::operator*=(T s) 
{
	x *= s;
	y *= s;
	return *this;
}
	
template<class T>
inline vec2<T> operator*(T s, const vec2<T>& v)  {
	return vec2<T>(v.x*s, v.y*s);
}

template<class T>
inline vec2<T> vec2<T>::operator/(T s) const
{
	return vec2(x/s, y/s);
}	

template<class T>
inline vec2<T>& vec2<T>::operator/=(T s) 
{
	x /= s;
	y /= s;
	return *this;
}
	
template<class T>
inline bool vec2<T>::operator==(const vec2& r) const {
	return x == r.x && y == r.y;
}

template<class T>
inline bool vec2<T>::operator!=(const vec2& r) const {
	return x != r.x || y != r.y;
}

template<class T>
inline std::ostream& operator<<(std::ostream& s, const vec2<T>& v) {
	s << '<' << v.x << ',' << v.y << '>';
	return s;
}
	
template<class T>
T vec2<T>::operator[](int i) const
{
    ASSERT(i < 2);
	return (&x)[i];
}

template<class T>
T& vec2<T>::operator[](int i) 
{
    ASSERT(i < 2);
	return (&x)[i];
}

template<class T>
inline void vec2<T>::Normalize()
{
	const T invLen = T(1) / Length(*this);
	x*=invLen;
	y*=invLen;
}

template<class T>
inline void vec2<T>::NormalizeSafe(const vec2& def)
{
	const T len = Length(*this);
	if(len != T(0)) {
		const T invLen = T(1) / len;
		x*=invLen;
		y*=invLen;
	} else *this = def;
}

template<class T>
inline bool vec2<T>::Equal(const vec2& other, float eps) const
{
	T result = 0;
	for(int i = 0; i < dims(); ++i)
	{
		const T diff = (*this)[i] - other[i];
		result += diff * diff;
	}
	return result <= eps;
}

////////////////////////////////////////
template<class T>
inline T Dot(const vec2<T>& l, const vec2<T>& r) {
	return l.x * r.x + l.y * r.y;
}

template<class T>
inline T LengthSq(const vec2<T>& l) {
	return (l.x * l.x + l.y * l.y);
}

template<class T>
inline T Length(const vec2<T>& l) {
	return Sqrt(l.x * l.x + l.y * l.y);
};

template<class T>
inline vec2<T> Normalize(const vec2<T> &v) {
	return v * (T(1) / Length(v));
}

template<class T>
inline vec2<T> NormalizeSafe(const vec2<T>& v, const vec2<T>& def = vec2<T>(T(1),T(0)))
{
	const T len = Length(v);
	if(len == T(0))
		return def;
	return v * (T(1) / len);
}

template<class T>
inline void Normal(vec2<T>* normal, T *len, const vec2<T> &v) {
	*len = Length(v);
	*normal = v * (T(1) / *len);
}

template<class T>
inline vec2<T> Min(const vec2<T>& l, const vec2<T>& r)
{
	return vec2<T>(Min(l.x,r.x), Min(l.y,r.y));
}

template<class T>
inline vec2<T> Max(const vec2<T>& l, const vec2<T>& r)
{
	return vec2<T>(Max(l.x,r.x), Max(l.y,r.y));
}

template<class T>
inline vec2<T> RadFromDeg(const vec2<T>& v)
{
    return { RadFromDeg(v.x), RadFromDeg(v.y) };
}

template<class T>
inline vec2<T> DegFromRad(const vec2<T>& v)
{
    return { DegFromRad(v.x), DegFromRad(v.y) };
}


////////////////////////////////////////////////////////////////////////////////
template<class T>
class vec3
{
public:
	typedef T baseType;
    static constexpr auto Dims = 3;

	T x, y, z;

	static inline int dims() { return Dims; }

	vec3() {}
	explicit vec3(T v) : x(v), y(v), z(v) {}
	vec3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}
	void Set(T x_, T y_, T z_) ;
	vec3 operator+(const vec3& r) const;
	vec3& operator+=(const vec3& r) ;
	vec3 operator-(const vec3& r) const;
	vec3& operator-=(const vec3& r) ;
	vec3 operator-() const;
	vec3 operator*(T s) const;
	vec3& operator*=(T s) ;
	vec3 operator/(T s) const;
	vec3& operator/=(T s) ;
	bool operator==(const vec3& r) const;
	bool operator!=(const vec3& r) const;
	T operator[](int i) const;
	T& operator[](int i) ;

	void Normalize();
	void NormalizeSafe(const vec3& def = vec3(T(1),T(0),T(0)));
	bool Equal(const vec3& other, float eps = 1e-3f) const;
};

template<class T>
inline void vec3<T>::Set(T x_, T y_, T z_) {
	x = x_; y = y_; z = z_;
}
	
template<class T>
inline vec3<T> vec3<T>::operator+(const vec3& r) const {
	return vec3(x + r.x, y + r.y, z + r.z);
}

template<class T>
inline vec3<T>& vec3<T>::operator+=(const vec3& r)  {
	x += r.x;
	y += r.y;
	z += r.z;
	return *this;
}

template<class T>
inline vec3<T> vec3<T>::operator-(const vec3& r) const {
	return vec3(x - r.x, y - r.y, z - r.z);
}

template<class T>
inline vec3<T>& vec3<T>::operator-=(const vec3& r)  {
	x -= r.x;
	y -= r.y;
	z -= r.z;
	return *this;
}
	
template<class T>
inline vec3<T> vec3<T>::operator-() const {
	return vec3(-x, -y, -z);
}

template<class T>
inline vec3<T> vec3<T>::operator*(T s) const
{
	return vec3(x*s,y*s,z*s);
}	

template<class T>
inline vec3<T>& vec3<T>::operator*=(T s) 
{
	x *= s;
	y *= s;
	z *= s;
	return *this;
}
	
template<class T>
inline vec3<T> operator*(T s, const vec3<T>& v)  {
	return vec3<T>(v.x*s, v.y*s, v.z*s);
}

template<class T>
inline vec3<T> vec3<T>::operator/(T s) const
{
	return vec3(x/s,y/s,z/s);
}	

template<class T>
inline vec3<T>& vec3<T>::operator/=(T s) 
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
}
	
template<class T>
inline bool vec3<T>::operator==(const vec3& r) const {
	return x == r.x && y == r.y && z == r.z;
}

template<class T>
inline bool vec3<T>::operator!=(const vec3& r) const {
	return x != r.x || y != r.y || z != r.z;
}

template<class T>
inline std::ostream& operator<<(std::ostream& s, const vec3<T>& v) {
	s << '<' << v.x << ',' << v.y << ',' << v.z << '>';
	return s;
}
	
template<class T>
T vec3<T>::operator[](int i) const
{
    ASSERT(i < 3);
	return (&x)[i];
}

template<class T>
T& vec3<T>::operator[](int i) 
{
    ASSERT(i < 3);
	return (&x)[i];
}

template<class T>
inline void vec3<T>::Normalize()
{
	T invLen = T(1) / Length(*this);
	x*=invLen;
	y*=invLen;
	z*=invLen;
}

template<class T>
inline void vec3<T>::NormalizeSafe(const vec3& def)
{
	const T len = Length(*this);
	if(len != T(0)) {
		T invLen = T(1) / len;
		x*=invLen;
		y*=invLen;
		z*=invLen;
	}
	else *this = def;
}

template<class T>
inline bool vec3<T>::Equal(const vec3& other, float eps) const
{
	T result = 0;
	for(int i = 0; i < dims(); ++i)
	{
		const T diff = (*this)[i] - other[i];
		result += diff * diff;
	}
	return result <= eps;
}

////////////////////////////////////////
template<class T>
inline T Dot(const vec3<T>& l, const vec3<T>& r) {
	return l.x * r.x + l.y * r.y + l.z * r.z;
}

template<class T>
inline vec3<T> Cross(const vec3<T>& l, const vec3<T>& r) {
	return vec3<T>(
			l.y*r.z - l.z*r.y,
			l.z*r.x - l.x*r.z,
			l.x*r.y - l.y*r.x);
}

template<class T>
inline T LengthSq(const vec3<T>& l) {
	return (l.x * l.x + l.y * l.y + l.z * l.z);
}

template<class T>
inline T Length(const vec3<T>& l) {
	return Sqrt(l.x * l.x + l.y * l.y + l.z * l.z);
};

template<class T>
inline vec3<T> Normalize(const vec3<T> &v) {
	return v * (T(1) / Length(v));
}

template<class T>
inline vec3<T> NormalizeSafe(const vec3<T>& v, const vec3<T>& def = vec3<T>(T(1),T(0),T(0)))
{
	const T len = Length(v);
	if(len == T(0))
		return def;
	return v * (T(1) / len);
}

template<class T>
inline void Normal(vec3<T>* normal, T *len, const vec3<T> &v) {
	*len = Length(v);
	*normal = v * (T(1) / *len);
}

template<class T>
inline vec3<T> Min(const vec3<T>& l, const vec3<T>& r)
{
	return vec3<T>(Min(l.x,r.x), Min(l.y,r.y), Min(l.z,r.z));
}

template<class T>
inline vec3<T> Max(const vec3<T>& l, const vec3<T>& r)
{
	return vec3<T>(Max(l.x,r.x), Max(l.y,r.y), Max(l.z,r.z));
}

template<class T>
inline vec3<T> RadFromDeg(const vec3<T>& v)
{
    return { RadFromDeg(v.x), RadFromDeg(v.y), RadFromDeg(v.z) };
}

template<class T>
inline vec3<T> DegFromRad(const vec3<T>& v)
{
    return { DegFromRad(v.x), DegFromRad(v.y), DegFromRad(v.z) };
}


////////////////////////////////////////////////////////////////////////////////
template<class T>
class vec4
{
public:
	typedef T baseType;
    static constexpr auto Dims = 4;

	T x, y, z, w;

	static inline int dims() { return Dims; }

	vec4() {}
	explicit vec4(T v) : x(v), y(v), z(v), w(v) {}
	vec4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
        vec4(const vec3<T>& o) : x(o.x), y(o.y), z(o.z), w(0.f) {}
        vec4(const vec2<T>& o) : x(o.x), y(o.y), z(0.f), w(0.f) {}
	void Set(T x_, T y_, T z_, T w_) ;
	vec4 operator+(const vec4& r) const;
	vec4& operator+=(const vec4& r) ;
	vec4 operator-(const vec4& r) const;
	vec4& operator-=(const vec4& r) ;
	vec4 operator-() const;
	vec4 operator*(T s) const;
	vec4& operator*=(T s) ;
	vec4 operator/(T s) const;
	vec4& operator/=(T s) ;
	bool operator==(const vec4& r) const;
	bool operator!=(const vec4& r) const;
	T operator[](int i) const;
	T& operator[](int i) ;

	void Normalize();
	void NormalizeSafe(const vec4& def = vec4(T(1),T(0),T(0),T(0)));
	bool Equal(const vec4& other, float eps = 1e-3f) const;
};

template<class T>
inline void vec4<T>::Set(T x_, T y_, T z_, T w_) {
	x = x_; y = y_; z = z_; w = w_;
}
	
template<class T>
inline vec4<T> vec4<T>::operator+(const vec4& r) const {
	return vec4(x + r.x, y + r.y, z + r.z, w + r.w);
}

template<class T>
inline vec4<T>& vec4<T>::operator+=(const vec4& r)  {
	x += r.x;
	y += r.y;
	z += r.z;
	w += r.w;
	return *this;
}

template<class T>
inline vec4<T> vec4<T>::operator-(const vec4& r) const {
	return vec4(x - r.x, y - r.y, z - r.z, w - r.w);
}

template<class T>
inline vec4<T>& vec4<T>::operator-=(const vec4& r)  {
	x -= r.x;
	y -= r.y;
	z -= r.z;
	w -= r.w;
	return *this;
}
	
template<class T>
inline vec4<T> vec4<T>::operator-() const {
	return vec4(-x, -y, -z, -w);
}

template<class T>
inline vec4<T> vec4<T>::operator*(T s) const
{
	return vec4(x*s, y*s, z*s, w*s);
}	

template<class T>
inline vec4<T>& vec4<T>::operator*=(T s) 
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}
	
template<class T>
inline vec4<T> operator*(T s, const vec4<T>& v)  {
	return vec4<T>(v.x*s, v.y*s, v.z*s, v.w*s);
}

template<class T>
inline vec4<T> vec4<T>::operator/(T s) const
{
	return vec4(x/s, y/s, z/s, w/s);
}	

template<class T>
inline vec4<T>& vec4<T>::operator/=(T s) 
{
	x /= s;
	y /= s;
	z /= s;
	w /= s;
	return *this;
}
	
template<class T>
inline bool vec4<T>::operator==(const vec4& r) const {
	return x == r.x && y == r.y && z == r.z && w == r.w;
}

template<class T>
inline bool vec4<T>::operator!=(const vec4& r) const {
	return x != r.x || y != r.y || z != r.z | w != r.w;
}

template<class T>
inline std::ostream& operator<<(std::ostream& s, const vec4<T>& v) {
	s << '<' << v.x << ',' << v.y << ',' << v.z << ',' << v.w << '>';
	return s;
}
	
template<class T>
T vec4<T>::operator[](int i) const
{
    ASSERT(i < 4);
	return (&x)[i];
}

template<class T>
T& vec4<T>::operator[](int i) 
{
    ASSERT(i < 4);
	return (&x)[i];
}

template<class T>
inline void vec4<T>::Normalize()
{
	const T invLen = T(1) / Length(*this);
	x*=invLen;
	y*=invLen;
	z*=invLen;
	w*=invLen;
}

template<class T>
inline void vec4<T>::NormalizeSafe(const vec4& def)
{
	const T len = Length(*this);
	if(len != T(0))
	{
		const T invLen = T(1) / len;
		x*=invLen;
		y*=invLen;
		z*=invLen;
		w*=invLen;
	}
	else *this = def;
}

template<class T>
inline bool vec4<T>::Equal(const vec4& other, float eps) const
{
	T result = 0;
	for(int i = 0; i < dims(); ++i)
	{
		const T diff = (*this)[i] - other[i];
		result += diff * diff;
	}
	return result <= eps;
}

////////////////////////////////////////
template<class T>
inline T Dot(const vec4<T>& l, const vec4<T>& r) {
	return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
}

template<class T>
inline vec4<T> Cross(const vec4<T>& l, const vec4<T>& r) {
	return vec4<T>(
        l.y*r.z - l.z*r.y,
        l.z*r.x - l.x*r.z,
        l.x*r.y - l.y*r.x,
        0);
}

template<class T>
inline T LengthSq(const vec4<T>& l) {
	return (l.x * l.x + l.y * l.y + l.z * l.z + l.w * l.w);
}

template<class T>
inline T Length(const vec4<T>& l) {
	return Sqrt(l.x * l.x + l.y * l.y + l.z * l.z + l.w * l.w);
};

template<class T>
inline vec4<T> Normalize(const vec4<T> &v) {
	return v * (T(1) / Length(v));
}

template<class T>
inline vec4<T> NormalizeSafe(const vec4<T>& v, const vec4<T>& def = vec4<T>(T(1),T(0),T(0),T(0)))
{
	const T len = Length(v);
	if(len == T(0))
		return def;
	return v * (T(1) / len);
}

template<class T>
inline void Normal(vec4<T>* normal, T *len, const vec4<T> &v) {
	*len = Length(v);
	*normal = v * (T(1) / *len);
}

template<class T>
inline vec4<T> Min(const vec4<T>& l, const vec4<T>& r)
{
	return vec4<T>(Min(l.x,r.x), Min(l.y,r.y), Min(l.z,r.z), Min(l.w, r.w));
}

template<class T>
inline vec4<T> Max(const vec4<T>& l, const vec4<T>& r)
{
	return vec4<T>(Max(l.x,r.x), Max(l.y,r.y), Max(l.z,r.z), Max(l.w, r.w));
}

template<class T>
inline vec4<T> RadFromDeg(const vec4<T>& v)
{
    return { RadFromDeg(v.x), RadFromDeg(v.y), RadFromDeg(v.z), RadFromDeg(v.w) };
}

template<class T>
inline vec4<T> DegFromRad(const vec4<T>& v)
{
    return { DegFromRad(v.x), DegFromRad(v.y), DegFromRad(v.z), DegFromrad(v.w) };
}

////////////////////////////////////////////////////////////////////////////////
template<class T>
inline vec2<T> ToVec2(const vec3<T>& v) {
    return vec2<T>{v.x, v.y};
}

template<class T>
inline vec2<T> ToVec2(const vec4<T>& v) {
    return vec2<T>{v.x, v.y};
}

template<class T>
inline vec3<T> ToVec3(const vec2<T>& v, float z = 0.f) {
    return vec3<T>{v.x, v.y, z};
}

template<class T>
inline vec3<T> ToVec3(const vec4<T>& v) {
    return vec3<T>{v.x, v.y, v.z};
}

template<class T>
inline vec4<T> ToVec4(const vec2<T>& v, float z = 0.f, float w = 0.f) {
    return vec4<T>{v.x, v.y, z, w};
}

template<class T>
inline vec4<T> ToVec4(const vec3<T>& v, float w = 0.f) {
    return vec4<T>{v.x, v.y, v.z, w};
}

////////////////////////////////////////////////////////////////////////////////
template<class VecT>
inline VecT ComponentwiseMultiply(const VecT& lhs, const VecT& rhs)
{
    VecT result;
    for (int i = 0; i < VecT::Dims; ++i)
    {
        result[i] = lhs[i] * rhs[i];
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
typedef vec2<int32_t> v2i;
typedef vec3<int32_t> v3i;
typedef vec4<int32_t> v4i;

typedef vec2<uint32_t> v2ui;
typedef vec3<uint32_t> v3ui;
typedef vec4<uint32_t> v4ui;

typedef vec2<float> v2f;
typedef vec3<float> v3f;
typedef vec4<float> v4f;

typedef vec2<double> v2d;
typedef vec3<double> v3d;
typedef vec4<double> v4d;
}

#endif

