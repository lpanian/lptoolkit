#pragma once
#ifndef INCLUDED_toolkit_common_HH
#define INCLUDED_toolkit_common_HH

#include <utility>
#include <iostream>
#include <cstdint>
#include <memory>
#include "compat.hh"
#include "mem.hh"

////////////////////////////////////////////////////////////////////////////////
// array helpers
#define CARRAY_SIZE(ary) (sizeof(ary)/sizeof(ary[0]))

template<class T, int N>
char (&array_size_helper(T (&)[N]))[N];
#define ARRAY_SIZE(ary) (sizeof(array_size_helper((ary))))

////////////////////////////////////////////////////////////////////////////////
// ASSERT
#ifdef USING_VS
#define DEBUGBREAK() __debugbreak() ;
#else
#define DEBUGBREAK() __asm__("int $3") ;
#endif

#ifdef USING_VS
	#define L__ASSERT_END() while(0, 0)
	#define L__ASSERT_TEST(cond) (cond, cond)
#else
	#define L__ASSERT_TEST(cond) (cond)
	#define L__ASSERT_END() while(0)
#endif

#ifdef DEBUG
#define ASSERT(condition) do { bool cond = (condition); if(!L__ASSERT_TEST(cond)) { \
	std::cout << "ASSERT FAILED @ " << __FILE__ << ":" << __LINE__ << std::endl; \
	DEBUGBREAK() } } L__ASSERT_END()
#else
#define ASSERT(condition) do { (void)sizeof(condition); } L__ASSERT_END()
#endif

////////////////////////////////////////////////////////////////////////////////
// C++11 helpers
template<typename T>
struct at_scope_exiter {
    at_scope_exiter(T&& fn) : m_fn(std::forward<T>(fn)) {}
    ~at_scope_exiter() { m_fn(); }
private:
    at_scope_exiter operator=(const at_scope_exiter&) DELETED;
    T m_fn;
};

template<typename T>
inline at_scope_exiter<T> at_scope_exit(T&& fn) { 	
    return at_scope_exiter<T>(std::forward<T>(fn));
}

#if defined(_MSC_VER) && _MSC_VER >= 1800
using std::make_unique;
#else
template<class T, typename... Args>
inline std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

namespace lptk 
{

//template<class T, MemPoolId POOL>
//inline std::unique_ptr<T> make_unique() 
//{
//	return std::unique_ptr<T>(new (POOL, alignof(T)) T());
//}
//
//template<class T, MemPoolId POOL, class A0>
//inline std::unique_ptr<T> make_unique(A0&& a0) 
//{
//	return std::unique_ptr<T>(new (POOL, alignof(T)) T(std::forward<A0>(a0)));
//}
//
//template<class T, MemPoolId POOL, class A0, class A1>
//inline std::unique_ptr<T> make_unique(A0&& a0, A1&& a1) 
//{
//	return std::unique_ptr<T>(new (POOL, alignof(T)) T(std::forward<A0>(a0), std::forward<A1>(a1)));
//}
//
//template<class T, MemPoolId POOL, class A0, class A1, class A2>
//inline std::unique_ptr<T> make_unique(A0&& a0, A1&& a1, A2&& a2) 
//{
//	return std::unique_ptr<T>(new (POOL, alignof(T)) T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)));
//}
//
//template<class T, MemPoolId POOL, class A0, class A1, class A2, class A3>
//inline std::unique_ptr<T> make_unique(A0&& a0, A1&& a1, A2&& a2, A3&& a3) 
//{
//	return std::unique_ptr<T>(new (POOL, alignof(T)) T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)));
//}
//
//template<class T, MemPoolId POOL, class A0, class A1, class A2, class A3, class A4>
//inline std::unique_ptr<T> make_unique(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4) 
//{
//	return std::unique_ptr<T>(new (POOL, alignof(T)) T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4)));
//}

////////////////////////////////////////////////////////////////////////////////
// This probably isn't the right place for this. But I don't think I have enough
// uses to create a core/grammarhelper.hh

template<class T>
inline const char* ChoosePlural(T count, const char* singular, const char* plural)
{
    if(count == T(1)) return singular;
    else return plural;
}

union EndianHelper {
#ifndef USING_VS
    constexpr 
#endif
    EndianHelper(int val) : i(val) {}
    uint32_t i;
    char c[4];
};

#ifndef USING_VS
constexpr 
#else
inline
#endif
bool IsNativeBigEndian() 
{
    return EndianHelper(0x01020304).c[0] == 0x01;
}

#ifdef USING_VS
inline
#else
constexpr 
#endif
bool IsNativeLittleEndian() 
{ 
    return !IsNativeBigEndian();
}
#endif

template<class T> struct ReversedType {
    T m_t;
    ReversedType (T& t) : m_t(t) {}
    auto begin() -> decltype(m_t.rbegin()) { return m_t.rbegin(); }
    auto end() -> decltype(m_t.rend()) { return m_t.rend(); }
};

template<class T> ReversedType<T> reversed(T& t) { return ReversedType<T>(t); }

template<typename T>
inline void unused_arg(const T&){}
template<typename... T>
inline void unused_args(const T&...){}

} // end lptk

#include "strhash.hh"

