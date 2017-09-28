#pragma once
#ifndef INCLUDED_toolkit_compat_HH
#define INCLUDED_toolkit_compat_HH

bool StrCaseEqual(const char* str1, const char* str2);
int StrCaseCmp(const char* str1, const char* str2);
bool StrNCaseEqual(const char* str1, const char* str2, unsigned int len);
int StrNCaseCmp(const char* str1, const char* str2, unsigned int len);
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

// This is a stupid hack to get rid of warnings/errors related to having exceptions turned off
// even when we find noexcept in the code.
#ifndef _CPPUNWIND
#define noexcept
#endif

#pragma warning( disable: 4351) // disable 'new behavior: elements of array 'array' will be default initialized'

// windows test
// you get alignof, but no alignas. Anything requiring that level of control will probably
// have to be aligned manually.
#if _MSC_VER < 1910
#define alignof __alignof
#endif

// constexpr isn't implemented in vs2013, so... this. Which breaks constexpr ctors
#if _MSC_VER < 1900
#define constexpr const
#endif

inline void bzero(void* ptr, size_t n) {
	memset(ptr, 0, n);
}

#if _MSC_VER <= 1800
#define thread_local __declspec(thread)
#endif

#endif

#define DELETED =delete

#define isnan _isnan

#define CONSTEXPR_CONST const
#else // linux
#define DELETED =delete
#define CONSTEXPR_CONST constexpr 
#endif

#endif
