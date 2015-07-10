#pragma once
#ifndef INCLUDED_lptk_format_HH
#define INCLUDED_lptk_format_HH

#include "toolkit/str.hh"
#include <cstring>
#include <cinttypes>

// TODO: Inline the crap out of this 
// also do a version where the string inside is constant sized and has a limited buffer!
namespace lptk
{
    // can probably do this so it becomes a single call to fprintf, but then we'd have to save a representation
    // of the printf variables...
    class FormatContainer
    {
        friend FormatContainer Format(const char* const formatStr);
        FormatContainer(const char* const formatStr);

    public:
        FormatContainer(FormatContainer&& other);
        FormatContainer& operator=(FormatContainer&& other);
        Str GetStr() const;
        
        FormatContainer With(char const* const s);
        FormatContainer With(Str const& s);
        FormatContainer With(uint32_t const i);
        FormatContainer With(uint64_t const i);
        FormatContainer With(int32_t const i);
        FormatContainer With(int64_t const i);
        FormatContainer With(float const f);
        FormatContainer With(double const f);
    private:
        struct ArgNode;
        struct Data;

        FormatContainer(const FormatContainer& other) DELETED;
        FormatContainer& operator=(const FormatContainer&) DELETED;
        
        ArgNode* AllocNode(size_t const len);

        std::unique_ptr<Data> m_data;
    };

    FormatContainer Format(const char* const formatStr);
}

#include "format.inl"

#endif

