#pragma once
#ifndef INCLUDED_toolkit_mathcommon_HH
#define INCLUDED_toolkit_mathcommon_HH

#include <type_traits>

#ifdef _WINDOWS
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)
#endif

#include <cmath>
#include <limits>

#define EPSILON (1e-3f)
#define EPSILON_SQ (1e-3f * 1e-3f)

namespace lptk
{

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
inline float Atan2(float y, float x) { return atan2f(y, x); }
inline double Atan2(double y, double x) { return atan2(y, x); }
inline float Sqrt(float x) { return sqrtf(x); }
inline double Sqrt(double x) { return sqrt(x); }
inline float Exp(float x) { return expf(x); }
inline double Exp(double x) { return exp(x); }
inline float Pow(float x, float y) { return powf(x, y); }
inline double Pow(double x, double y) { return pow(x, y); }
inline int IntFloor(float x) { return static_cast<int>(x); }
inline int IntFloor(double x) { return static_cast<int>(x); }
inline int IntCeil(float x) { return static_cast<int>(ceill(x)); }
inline int IntCeil(double x) { return static_cast<int>(ceill(x)); }

static const float kPi = 4.f * Atan2(1.f, 1.f);
static const double kPiD = 4.0 * Atan2(1.0, 1.0);
static const float kInvPi = 1.f / kPi;
static const double kInvPiD = 1.0 / kPiD;
static const float kDegToRad = kPi / 180.f;
static const double kDegToRadD = kPiD / 180.0;
static const float kRadToDeg = 180.f / kPi;
static const double kRadToDegD = 180.0 / kPiD;

inline float RadFromDeg(float deg) { return kDegToRad * deg; }
inline double RadFromDeg(double deg) { return kDegToRadD * deg; }
inline float DegFromRad(float rad) { return kRadToDeg * rad; }
inline double DegFromRad(double rad) { return kRadToDegD * rad; }

inline float AngleWrap(float rad) { 
    while(rad < 0.f) rad += 2.f * kPi;
    while(rad > 2.f * kPi) rad -= 2.f * kPi;
    return rad;
}
inline double AngleWrap(double rad) { 
    while(rad < 0.0) rad += 2.0 * kPiD;
    while(rad > 2.0 * kPiD) rad -= 2.0 * kPiD;
    return rad;
}

template<class T>
inline bool Equal(T l, T r, T eps = T(1e-3)) { 
	return Abs(l-r) <= eps;
}

template<class T>
inline bool IsPower2(T value) { return (value & (value-1)) == 0; }

template<class T>
inline T AlignValue(T val, T align)  {
	const T alignMask = align - 1;
	return (val + alignMask) & ~alignMask;
}
template<class T>
inline T Clamp(T v, T lo, T hi) { return Min(Max(v,lo), hi); }
template<class T>
inline T Lerp(float t, T a, T b) { return (1.f - t) * a + t * b; }

template<typename T>
inline std::enable_if_t<std::is_floating_point<T>::value, T> Mod(const T val, const T limit)
{
    T result = val;
    while (result >= limit)
        result -= limit;
    while (result < 0)
        result += limit;
    return result;
}

template<typename T>
inline std::enable_if_t<std::is_integral<T>::value &&
    std::is_signed<T>::value, T> Mod(const T val, const T limit)
{
    if (val >= 0)
        return val % limit;
    else
    {
        T result = val;
        while (result < 0)
            result += limit;
        return result;
    }
}

template<typename T>
inline std::enable_if_t<std::is_integral<T>::value &&
    std::is_unsigned<T>::value, T> Mod(T val, T limit)
{
    return val % limit;
}



template<class TO, class FROM>
inline TO ClampCast(const FROM& value)
{
	auto temp = value;
    constexpr auto hi = std::numeric_limits<TO>::max();
    constexpr auto lo = std::numeric_limits<TO>::lowest();
    if (temp > hi)
        temp = hi;
    if (temp < lo)
        temp = lo;
	return static_cast<TO>(temp);
}

inline unsigned long IntLog2(unsigned long num)
{
#ifdef _WINDOWS
	unsigned long index;
	_BitScanReverse(&index, num);
	return index;
#else
	return 8*sizeof(num) - 1 - __builtin_clzl(num);
#endif
}

inline unsigned long FirstBitIndex(unsigned long num)
{
#ifdef _WINDOWS
	unsigned long index;
    if (_BitScanForward(&index, num))
    {
        return index;
    }
    else
    {
        return 0;
    }
#else
    return num ? __builtin_ctzl(num) : 0;
#endif

}

inline unsigned long IntNearestPower2(unsigned long num)
{
    return (1u << IntLog2(num));
}

inline unsigned long IntNextPower2(unsigned long num)
{
    const auto log2 = IntLog2(num - 1) + 1;
    return 1u << (log2);
}

inline float Log2(float x)
{
    static const float invLog2 = 1.f / logf(2.f);
    return invLog2 * logf(x);
}

template<class T>
inline void Swap(T& l, T& r) {
    T temp = std::move(l);
    l = std::move(r);
    r = std::move(temp);
}

inline float LanczosSinc(float x, float tau) 
{
    x = Abs(x);
    if(x < 1e-5f) return 1.f;
    if(x > 1.f) return 0.f;
    x *= kPi;

    const float xtau = x * tau;
    const float sinc = Sin(xtau) / xtau;
    const float lanczos = Sin(x) / x;
    return sinc * lanczos;
}

bool SolveQuadratic(float a, float b, float c, float *t1, float *t2);
bool SolveLinear2x2(const float* A, const float* b, float* x0, float *x1);

template<class T>
inline T Remap(T param, T oldMin, T oldMax, T newMin, T newMax)
{
    const auto t = (param - oldMin) / (oldMax - oldMin);
    return t * (newMax - newMin) + newMin;
}

}

#endif
