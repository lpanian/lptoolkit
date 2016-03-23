#pragma once

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
char (&array_size_helper(T (&)[N]))[N] = delete;
#define ARRAY_SIZE(ary) (sizeof(array_size_helper((ary))))

////////////////////////////////////////////////////////////////////////////////
// ASSERT
#ifdef USING_VS
#define DEBUGBREAK() __debugbreak() ;
#else
#define DEBUGBREAK() __asm__("int $3") ;
#endif

#define L__OWL() (0,0)
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
#define ASSERT(condition) do { if(L__OWL()) { (void)(condition); } } L__ASSERT_END()
#endif

////////////////////////////////////////////////////////////////////////////////
// C++11 helpers
template<typename T>
struct at_scope_exiter {
    at_scope_exiter(T&& fn) : m_fn(std::forward<T>(fn)) {}
    ~at_scope_exiter() { if(!m_triggered) m_fn(); }

    void trigger() {
        if (!m_triggered)
        {
            m_fn();
            m_triggered = true;
        }
    }
private:
    at_scope_exiter operator=(const at_scope_exiter&) DELETED;
    T m_fn;
    bool m_triggered = false;
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

////////////////////////////////////////////////////////////////////////////////
// This probably isn't the right place for this. But I don't think I have enough
// uses to create a core/grammarhelper.hh

template<class T>
inline const char* ChoosePlural(T count, const char* singular, const char* plural)
{
    if(count == T(1)) return singular;
    else return plural;
}

// TODO: move this to a range helper header
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

