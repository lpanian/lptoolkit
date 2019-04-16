#pragma once

////////////////////////////////////////////////////////////////////////////////
// Include me once in your program, probably wherever 'main' is.
// This avoids link issues. There is now enlightenment to be found here, however.
////////////////////////////////////////////////////////////////////////////////

#include <toolkit/mem.hh>

////////////////////////////////////////////////////////////////////////////////
// Global override new/delete
void *operator new(std::size_t n) 
{
    return lptk::mem_allocate(n, lptk::MEMPOOL_General, 16);
}

void operator delete(void* p) noexcept
{
    lptk::mem_free(p);
}

