#pragma once
#ifndef INCLUDED_toolkit_stlwrap_HH
#define INCLUDED_toolkit_stlwrap_HH

#include <vector>

// Use this to make the typing a little less painful if you want an std::vector 
// and just want to track the memory, instead of replacing it altogether.
template<class T, MemPoolId POOL = MEMPOOL_General>
struct mkvector
{
	typedef typename std::vector<T, MemPoolSTLAlloc<T, POOL, alignof(T)> > type;
};

#endif
