#pragma once
#ifndef INCLUDED_lptoolkit_strhash_HH
#define INCLUDED_lptoolkit_strhash_HH

#include "toolkit/str.hh"

namespace std 
{
    template<lptk::MemPoolId POOL>
    struct hash<lptk::StringImpl<POOL> >
    {
        size_t operator()(const lptk::StringImpl<POOL>& x) const {
            return ComputeHash(x);
        }
        
    };
}

#endif
