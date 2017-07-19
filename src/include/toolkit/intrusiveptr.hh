#pragma once
#ifndef INCLUDED_toolkit_intrusiveptr_HH
#define INCLUDED_toolkit_intrusiveptr_HH

#include <atomic>

namespace lptk
{
    class refcounted
    {
        std::atomic<uint32_t> m_refs;

    protected:
        refcounted() : m_refs(0)
        {
        }

        virtual ~refcounted() {}
    public:
        refcounted(const refcounted&) = delete;
        refcounted& operator=(const refcounted&) = delete;

        void addRef() 
        {
            m_refs.fetch_add(1, std::memory_order_relaxed);
        }

        void decRef() 
        {
            if (m_refs.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete this;
            }
        }

        auto numRefs() const {
            return m_refs.load(std::memory_order_relaxed);
        }
    };

    template<class T>
    class intrusive_ptr
    {
        T* m_p;
    public:
        intrusive_ptr(std::nullptr_t)
            : m_p(nullptr)
        {
        }

        intrusive_ptr(T* p = nullptr)
            : m_p(p)
        {
            if (m_p) m_p->addRef();
        }

        ~intrusive_ptr() {
            if (m_p) m_p->decRef();
        }

        intrusive_ptr(const intrusive_ptr& other)
            : m_p(other.m_p)
        {
            if(m_p) m_p->addRef();
        }

        intrusive_ptr& operator=(const intrusive_ptr& other)
        {
            if (this != &other)
            {
                T* tmp = other.m_p;
                if (tmp) tmp->addRef();
                if (m_p) m_p->decRef();
                m_p = tmp;
            }
            return *this;
        }

        intrusive_ptr(intrusive_ptr&& other)
            : m_p(other.m_p)
        {
            other.m_p = nullptr;
        }
        
        intrusive_ptr& operator=(intrusive_ptr&& other)
        {
            if (this != &other)
            {
                m_p = other.m_p;
                other.m_p = nullptr;
            }
            return *this;
        }

        template<class U>
        intrusive_ptr(const intrusive_ptr<U>& other)
            : m_p(other.m_p)
        {
            m_p->addRef();
        }

        template<class U>
        intrusive_ptr& operator=(const intrusive_ptr<U>& other)
        {
            T* tmp = other.m_p;
            if (tmp) tmp->addRef();
            if (m_p) m_p->decRef();
            m_p = tmp;
            return *this;
        }
        
        template<class U>
        intrusive_ptr(intrusive_ptr<U>&& other)
            : m_p(other.m_p)
        {
            other.m_p = nullptr;
        }

        template<class U>
        intrusive_ptr& operator=(intrusive_ptr<U>&& other)
        {
            m_p = other.m_p;
            other.m_p = nullptr;
            return *this;
        }

        bool operator==(const T* other) const { return m_p == other; }
        bool operator!=(const T* other) const { return m_p != other; }
        bool operator==(const intrusive_ptr& other) const { return m_p == other.m_p; }
        bool operator!=(const intrusive_ptr& other) const { return m_p != other.m_p; }

        template<class U> bool operator==(const U* other) const { return m_p == other; }
        template<class U> bool operator!=(const U* other) const { return m_p != other; }
        template<class U> bool operator==(const intrusive_ptr<U>& other) const { return m_p == other.m_p; }
        template<class U> bool operator!=(const intrusive_ptr<U>& other) const { return m_p != other.m_p; }

        T* get() const { return m_p; }
        T& operator*() const { return *m_p; }
        T* operator->() const { return m_p; }

        bool valid() const { return m_p != nullptr; }

        explicit operator bool() const { return m_p != nullptr; }
    };

    template<class T, class... Arg>
    lptk::intrusive_ptr<T> make_intrusive_ptr(Arg&&... args)
    {
        return lptk::intrusive_ptr<T>( new T(std::forward<Arg>(args)...) );
    }
}

#endif
