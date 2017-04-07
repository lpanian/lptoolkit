#pragma once

#include <cstddef>
#include <utility>
#include <new>

/*

Small function wrapper with a small object buffer - good for capturing lambdas
that have small captured args without allocating more memory.

*/
namespace lptk
{
    template<size_t N, typename... T>
    class FunctionN;

    template<size_t N, typename R, typename... Args>
    class alignas(alignof(std::max_align_t)) 
        FunctionN<N, R(Args...) >
    {
        class ImplBase
        {
        public:
            virtual ~ImplBase() = default;
            virtual R operator()(Args... args) = 0;
            virtual size_t GetSize() const = 0;
            virtual ImplBase* CopyTo(char*) const = 0;
            virtual ImplBase* MoveTo(char*) const = 0;
            virtual ImplBase* Clone() const = 0;
        };
        template<typename T>
        class Impl : public ImplBase
        {
            T m_fn;
        public:
            Impl(T fn) : m_fn(std::move(fn)) {}
            Impl(const Impl&) = default;
            Impl& operator=(const Impl&) = default;
            Impl(Impl&&) = default;
            Impl& operator=(Impl&&) = default;

            R operator()(Args... args) override
            {
                return m_fn(std::forward<Args>(args)...);
            }
            
            size_t GetSize() const override { return sizeof(Impl<T>); }
            ImplBase* CopyTo(char* buffer) const override
            {
                return new (buffer) Impl<T>(*this);
            }
            ImplBase* MoveTo(char* buffer) const override
            {
                return new (buffer) Impl<T>(std::move(*this));
            }
            ImplBase* Clone() const override {
                return new Impl<T>(*this);
            }

        };

    public:
        static constexpr size_t byte_size = N;
        using result_type = R;

        FunctionN() = default;
        FunctionN(std::nullptr_t) : FunctionN() {}

        template<typename T>
        FunctionN(T fn)
        {
            if (sizeof(Impl<T>) <= ARRAY_SIZE(m_storage))
            {
                m_impl = new (m_storage) Impl<T>(std::move(fn));
            }
            else
            {
                m_impl = new Impl<T>(std::move(fn));
            }
        }

        ~FunctionN() 
        {
            Clear();
        }

        FunctionN(const FunctionN& other)
        {
            if (other.m_impl)
            {
                if (other.m_impl->GetSize() <= ARRAY_SIZE(m_storage))
                    m_impl = other.m_impl->CopyTo(m_storage);
                else
                    m_impl = other.m_impl->Clone();
            }
        }
        FunctionN& operator=(const FunctionN& other)
        {
            if (this != &other)
            {
                if (other.m_impl->GetSize() <= ARRAY_SIZE(m_storage))
                    m_impl = other.m_impl->CopyTo(m_storage);
                else
                    m_impl = other.m_impl->Clone();
            }
            return *this;
        }

        FunctionN(FunctionN&& other)
        {
            Move(std::move(other));
        }

        FunctionN& operator=(FunctionN&& other)
        {
            if (this != &other)
            {
                Move(std::move(other));
            }
            return *this;
        }

        explicit operator bool() const 
        { 
            return m_impl != nullptr; 
        }

        R operator()(Args... args) const 
        {
            return (*m_impl)(std::forward<Args>(args)...);
        }

    private:
        void Copy(const FunctionN& other)
        {
            if (other.m_impl)
            {
                if (other.m_impl->GetSize() <= ARRAY_SIZE(m_storage))
                    m_impl = other.m_impl->CopyTo(m_storage);
                else
                    m_impl = other.m_impl->Clone();
            }
            else
                m_impl = nullptr;
        }

        void Move(FunctionN&& other)
        {
            if (other.m_impl)
            {
                if (other.m_impl->GetSize() <= ARRAY_SIZE(m_storage))
                {
                    m_impl = other.m_impl->MoveTo(m_storage);
                    other.m_impl = nullptr;
                }
                else
                {
                    m_impl = std::exchange(other.m_impl, nullptr);
                }
            }
            else
            {
                m_impl = nullptr;
            }
        }

        void Clear() 
        {
            if (m_impl)
            {
                if (m_impl->GetSize() > ARRAY_SIZE(m_storage))
                    delete m_impl;
                else
                    m_impl->~ImplBase();
                m_impl = nullptr;
            }
        }
        char m_storage[N - sizeof(ImplBase*)];
        ImplBase* m_impl = nullptr;
    };
        
    static_assert(sizeof(FunctionN<32, int(void)>) == 32, "FunctionN is wrong size");
    static_assert(sizeof(FunctionN<64, int(void)>) == 64, "FunctionN is wrong size");

    template<typename R, typename... Args>
    using Function = FunctionN<32, R, Args...>;
}

