#pragma once

#include <cmath>
#include <iostream>
#include <tuple>
//#include "core/mathhelpers.hh"
//class MemSerializer;

template<class T> inline T Min(T x, T y) { return x < y ? x : y; }
template<class T> inline T Max(T x, T y) { return x > y ? x : y; }
template<class T> inline T Abs(T x) { return x < 0 ? -x : x; }
inline float Cos(float x)  { return cosf(x); }
inline double Cos(double x)  { return cos(x); }
inline float Sin(float x)  { return sinf(x); }
inline double Sin(double x)  { return sin(x); }

template<class T>
class vec3
{
public:
	typedef T baseType;

	T x, y, z;

	inline int dims() const { return 3; }

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

	//void SerializeTo(MemSerializer&) const;
};

typedef vec3<float> v3f;
typedef vec3<double> v3d;


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
	return (&x)[i];
}

template<class T>
T& vec3<T>::operator[](int i) 
{
	return (&x)[i];
}

////////////////////////////////////////////////////////////////////////////////
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
	return sqrtf(l.x * l.x + l.y * l.y + l.z * l.z);
};

template<class T>
inline T Dist(const vec3<T>& l, const vec3<T>& r) {
	return Length(l-r);
}

template<class T>
inline T DistSq(const vec3<T>& l, const vec3<T>& r) {
	return LengthSq(l-r);
}

template<class T>
inline vec3<T> Normalize(const vec3<T> &v) {
	return v / Length(v);
}

template<class T>
inline void Normal(vec3<T>& normal, T &len, const vec3<T> &v) {
	len = Length(v);
	normal = v / len;
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
inline vec3<T> VecMin(const vec3<T>& l, const vec3<T>& r) { return Min(l,r); }

template<class T>
inline vec3<T> VecMax(const vec3<T>& l, const vec3<T>& r) { return Max(l,r); }

template<class T>
inline vec3<T> RotateAround(const vec3<T>& v, const vec3<T>& axis, T angle)
{
	T c = Cos(angle);
	T s = Sin(angle);
	T one_minus_c = T(1) - c;
	T axis_dot_v = Dot(axis, v);
	T k = one_minus_c * axis_dot_v;
	vec3<T> axis_cross_v = Cross(axis, v);
	return vec3<T>(
	 c * v.x + k * axis.x + s * axis_cross_v.x,
	 c * v.y + k * axis.y + s * axis_cross_v.y,
	 c * v.z + k * axis.z + s * axis_cross_v.z);
}


////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void vec3<T>::Normalize()
{
	T invLen = 1.f/ Length(*this);
	x*=invLen;
	y*=invLen;
	z*=invLen;
}

// return a side and up vector to form an orthogonal basis perpendicular to dir.
// if dir has no length then the results are undefined.
template<class T>
std::tuple<vec3<T>, vec3<T> > CreateArbitraryBasis(const vec3<T>& dir)
{
	T dirVals[3] = {Abs(dir.x), Abs(dir.y), Abs(dir.z)};
	int smallest = 0;
	for(int i = 1; i < 3; ++i)
		if(dirVals[smallest] > dirVals[i])
			smallest = i;
	T minAxisVals[3] = {0,0,0};
	minAxisVals[smallest] = 1.f;
	const vec3 minAxis = vec3(minAxisVals[0], minAxisVals[1], minAxisVals[2]);

	const vec3 up = Cross(minAxis, dir);
	const vec3 side = Cross(dir, up);

	const vec3 expectedDir = Cross(side, up);
	if(Dot(expectedDir, dir) < 0)
		return std::make_tuple(-Normalize(side), -Normalize(up));
	else
		return std::make_tuple(Normalize(side), Normalize(up));
}

