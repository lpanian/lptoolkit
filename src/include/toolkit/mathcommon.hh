#pragma once
#ifndef INCLUDED_toolkit_mathcommon_HH
#define INCLUDED_toolkit_mathcommon_HH

#ifdef _WINDOWS
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#endif

#include <cmath>
#include <limits>

#define EPSILON (1e-3f)
#define EPSILON_SQ (1e-3f * 1e-3f)

template<class T> inline T Min(T x, T y) { return x < y ? x : y; }
template<class T> inline T Max(T x, T y) { return x > y ? x : y; }
template<class T> inline T Abs(T x) { return x < 0 ? -x : x; }
inline float Cos(float x)  { return cosf(x); }
inline double Cos(double x)  { return cos(x); }
inline float Acos(float x)  { return acosf(x); }
inline double Acos(double x)  { return acos(x); }
inline float Sin(float x)  { return sinf(x); }
inline double Sin(double x)  { return sin(x); }
inline float Asin(float x)  { return asinf(x); }
inline double Asin(double x)  { return asin(x); }
inline float Tan(float x) { return tanf(x); }
inline double Tan(double x) { return tan(x); }
inline float Atan(float x) { return atanf(x); }
inline double Atan(double x) { return atan(x); }
inline float Atan2(float x, float y) { return atan2f(x, y); }
inline double Atan2(double x, double y) { return atan2(x, y); }
inline float Sqrt(float x) { return sqrtf(x); }
inline double Sqrt(double x) { return sqrt(x); }

static const float kPi = 4.f * Atan2(1.f, 1.f);
static const double kPiD = 4.0 * Atan2(1.0, 1.0);
static const float kDegToRad = kPi / 180.f;
static const double kDegToRadD = kPiD / 180.0;
static const float kRadToDeg = 180.f / kPi;
static const double kRadToDegD = 180.0 / kPiD;

inline float RadFromDeg(float deg) { return kDegToRad * deg; }
inline double RadFromDeg(double deg) { return kDegToRadD * deg; }
inline float DegFromRad(float rad) { return kRadToDeg * rad; }
inline double DegFromRad(double rad) { return kRadToDegD * rad; }

template<class T>
inline bool Equal(T l, T r, T eps = T(1e-3)) { 
	return Abs(l-r) <= eps;
}

template<class T>
inline bool IsPower2(T value) { return (value & (value-1)) == 0; }
inline size_t AlignValue(size_t val, size_t align)  {
	const size_t alignMask = align-1;
	return (val+alignMask) & ~alignMask;
}
template<class T>
inline T Clamp(const T& v, const T& lo, const T& hi) { return Min(Max(v,lo), hi); }
template<class T>
inline T Lerp(float t, const T& a, const T& b) { return (1.f - t) * a + t * b; }

template<class TO, class FROM>
inline TO ClampCast(const FROM& value)
{
	FROM temp = value;
	if(temp > std::numeric_limits<TO>::max())
		temp = std::numeric_limits<TO>::max();
	else if(temp < std::numeric_limits<TO>::min())
		temp = std::numeric_limits<TO>::min();
	return static_cast<TO>(temp);
}

inline unsigned long IntLog2(unsigned long num)
{
#ifdef _WINDOWS
	unsigned long index;
	_BitScanReverse(&index, num);
	return index;
#else
	return 8*sizeof(num) - __builtin_clzl(num);
#endif
}

#endif
