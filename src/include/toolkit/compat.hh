#pragma once
#ifndef INCLUDED_toolkit_compat_HH
#define INCLUDED_toolkit_compat_HH
bool StrCaseEqual(const char* str1, const char* str2);
bool StrNCaseEqual(const char* str1, const char* str2, unsigned int len);
void SleepMS(unsigned int ms);

#ifdef _MSC_VER
#if _MSC_VER < 1600
#error "Require at least vs2010"
#else

#include <cstring>

// for special cases in code
#define USING_VS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef max
#undef min

#pragma warning( disable: 4351)

// windows test
// you get alignof, but no alignas. Anything requiring that level of control will probably
// have to be aligned manually.
#define alignof __alignof
// constexpr isn't implemented in vs2010, so... this.
#define constexpr const

inline void bzero(void* ptr, size_t n) {
	memset(ptr, 0, n);
}
#endif

#define DELETED /* nothing */

#define isnan _isnan

#define CONSTEXPR_CONST const
#else // linux
#define DELETED =delete
#define CONSTEXPR_CONST constexpr const
#endif

#endif
