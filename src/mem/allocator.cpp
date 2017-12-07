#include "toolkit/mem/allocator.hh"
#include "toolkit/mem.hh"

namespace lptk 
{
    namespace mem
    {
        void* DefaultAllocator::Alloc(size_t size, unsigned align)
        {
            return lptk::mem_allocate(size, lptk::MEMPOOL_General, align);
        }

        void DefaultAllocator::Free(void* ptr)
        {
            return lptk::mem_free(ptr);
        }
       
        static DefaultAllocator s_defaultAlloc;
        DefaultAllocator* GetDefaultAllocator()
        {
            return &s_defaultAlloc;
        }
    }
}
