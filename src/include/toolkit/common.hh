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
	T& m_fn;
	at_scope_exiter(T&& fn) : m_fn(fn) {}
	~at_scope_exiter() { m_fn(); }
private:
	at_scope_exiter operator=(const at_scope_exiter&) DELETED;
};

template<typename T>
at_scope_exiter<T> at_scope_exit(T&& fn) { 	
	return at_scope_exiter<T>(std::forward<T>(fn)); 
}

#ifndef USING_VS
template<class T>
inline std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

//template<class T, MemPoolId POOL>
//inline std::unique_ptr<T> make_unique(Args&&... args)
//{
//	return std::unique_ptr<T>(new (POOL, alignof(T)) T(std::forward<Args>(args)...));
//}
#else
template<class T>
inline std::unique_ptr<T> make_unique() 
{
	return std::unique_ptr<T>(new T());
}

template<class T, class A0>
inline std::unique_ptr<T> make_unique(A0&& a0) 
{
	return std::unique_ptr<T>(new T(std::forward<A0>(a0)));
}

template<class T, class A0, class A1>
inline std::unique_ptr<T> make_unique(A0&& a0, A1&& a1) 
{
	return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1)));
}

template<class T, class A0, class A1, class A2>
inline std::unique_ptr<T> make_unique(A0&& a0, A1&& a1, A2&& a2) 
{
	return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)));
}

template<class T, class A0, class A1, class A2, class A3>
inline std::unique_ptr<T> make_unique(A0&& a0, A1&& a1, A2&& a2, A3&& a3) 
{
	return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)));
}

template<class T, class A0, class A1, class A2, class A3, class A4>
inline std::unique_ptr<T> make_unique(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4) 
{
	return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4)));
}


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
#endif

////////////////////////////////////////////////////////////////////////////////
// This probably isn't the right place for this. But I don't think I have enough
// uses to create a core/grammarhelper.hh
template<class T>
inline const char* ChoosePlural(T count, const char* singular, const char* plural)
{
	if(count == T(1)) return singular;
	else return plural;
}


#ifdef USING_VS
inline bool IsNativeBigEndian() {
	union EndianHelper {
		EndianHelper(int val) : i(val) {}
		uint32_t i;
		char c[4];
	};
	return EndianHelper(0x01020304).c[0] == 0x01;
}
#else
constexpr bool IsNativeBigEndian() {
	union EndianHelper {
		constexpr EndianHelper() : i{0x01020304} {}
		uint32_t i;
		char c[4];
	};
	return EndianHelper().c[0] == 0x01;
}
#endif

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

