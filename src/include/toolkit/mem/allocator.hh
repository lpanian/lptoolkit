#pragma once

#include <cstdint>
#include <type_traits>

namespace lptk
{
    namespace mem
    {
        ////////////////////////////////////////////////////////////////////////////////
        class Allocator
        {
        public:
            virtual ~Allocator() = default;

            virtual void* Alloc(size_t size, unsigned align) = 0;
            virtual void Free(void* ptr) = 0;

            template<typename T, typename... Args>
            T* Create(Args&&... args)
            {
                void* mem = Alloc(sizeof(T), alignof(T));
                if (!mem)
                    return nullptr;
                return new (mem) T(std::forward<Args>(args)...);
            }

            template<typename T>
            T* CreateN(size_t N)
            {
                const auto requiredSize = N * sizeof(T);
                constexpr auto requiredAlign = unsigned(alignof(T));
                void* mem = Alloc(requiredSize, requiredAlign);
                if (!mem)
                    return nullptr;
                for (size_t i = 0; i < N; ++i)
                    new (&reinterpret_cast<T*>(mem)[i]) T{};
                return reinterpret_cast<T*>(mem);
            }

            template<typename T, typename... Args>
            T* CreateWithExtra(size_t N, unsigned align, Args&&... args)
            {
                const auto requiredSize = N > sizeof(T) ? N : sizeof(T);
                const auto requiredAlign = align > unsigned(alignof(T)) ? align : unsigned(alignof(T));
                void* mem = Alloc(requiredSize, requiredAlign);
                if (!mem)
                    return nullptr;
                return new (mem) T(std::forward<Args>(args)...);
            }

            template<typename T>
            void Destroy(T* ptr) {
                ptr->~T();
                Free(ptr);
            }

            template<typename T>
            void DestroyN(size_t N, T* ptr) {
                for (size_t i = 0; i < N; ++i)
                    ptr[i].~T();
                Free(ptr);
            }
        };

        ////////////////////////////////////////////////////////////////////////////////
        // helper class for unique_ptr to delete via allocator returned by
        // class's GetAllocator function. This is good if you already have the
        // allocator stored and don't need to store it alongisde the pointer.
        struct Deleter
        {
            template<typename T>
            void operator()(T* ptr) const
            {
                if (ptr)
                {
                    Allocator* allocator = ptr->GetAllocator();
                    allocator->Destroy(ptr);
                }
            }
        };

        ////////////////////////////////////////////////////////////////////////////////
        // Helper class for unique_ptr that contains the actual allocator pointer. This
        // effectively stores the allocator pointer next to the allocated pointer.
        struct PtrDeleter
        {
            mem::Allocator* m_alloc;
            PtrDeleter(mem::Allocator* alloc = nullptr) : m_alloc(alloc) {}

            template<typename T>
            void operator()(T* ptr) const
            {
                if (ptr && m_alloc)
                {
                    m_alloc->Destroy(ptr);
                }
            }
        };

        ////////////////////////////////////////////////////////////////////////////////
        // Helper class for unique_ptr with arrays, that contains the actual
        // allocator pointer. This stores the alloc pointer and the allocated count next
        // to the allocated pointer.
        struct PtrDeleterN
        {
            mem::Allocator* m_alloc;
            size_t m_count;
            PtrDeleterN(size_t count = 0, mem::Allocator* alloc = nullptr) : m_alloc(alloc), m_count(count) {}
            template<typename T>
            void operator()(T* ptr) const
            {
                if (ptr && m_alloc)
                {
                    m_alloc->DestroyN(m_count, ptr);
                }
            }
        };

        ////////////////////////////////////////////////////////////////////////////////
        // helpers for allocating to unique_ptrs with the above deleters
        template<typename T>
        using PtrWithDeleter = std::unique_ptr<T, PtrDeleter>;
        template<typename T, typename... Args>
        inline PtrWithDeleter<T> Create(mem::Allocator* alloc, Args&&... args)
        {
            return PtrWithDeleter<T>{
                alloc->Create<T>(std::forward<Args>(args)...), PtrDeleter{ alloc }
            };
        }

        template<typename T>
        using ArrayPtrWithDeleter = std::unique_ptr<T[], PtrDeleterN>;
        template<typename T>
        inline ArrayPtrWithDeleter<T> CreateN(size_t N, mem::Allocator* alloc)
        {
            return ArrayPtrWithDeleter<T>{
                alloc->CreateN<T>(N), PtrDeleterN{ N, alloc }
            };
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Helper for simple allocations that just alloc/free, no ctor/dtor.
        using RawPtrWithDeleter = std::unique_ptr<int8_t, PtrDeleter>;
        inline RawPtrWithDeleter Alloc(size_t N, unsigned align, mem::Allocator* alloc)
        {
            return RawPtrWithDeleter{
                reinterpret_cast<int8_t*>(alloc->Alloc(N, align)),
                PtrDeleter{alloc}
            };
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Default allocator providing whatever lptk::mem_allocate and
        // lptk::mem_free do.
        class DefaultAllocator : public Allocator
        {
        public:
            void* Alloc(size_t size, unsigned align) override;
            void Free(void* ptr) override;
        };

        DefaultAllocator* GetDefaultAllocator();
    }
}


