// shared_instance.hpp -- a std::shared_ptr wrapper that cannot be null
//
// Copyright Robin Eckert 2014
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _SHARED_INSTANCE_H
#define _SHARED_INSTANCE_H

#include <memory>
#include <stdexcept>

namespace rebox
{
    template<typename T>
    class shared_instance
    {
    public:
        using type = T;

        shared_instance() = delete;

        // constructors from other shared_instance's
        shared_instance(shared_instance const&) = default;

        template<typename Y>
        shared_instance(shared_instance<Y> const&);

        shared_instance(shared_instance&&) = default;

        template<typename Y>
        shared_instance(shared_instance<Y>&&);

        // constructors from std::shared_ptr's
        explicit shared_instance(std::shared_ptr<T> const&);

        template<typename Y>
        explicit shared_instance(std::shared_ptr<Y> const&);

        explicit shared_instance(std::shared_ptr<T>&&);

        template<typename Y>
        explicit shared_instance(std::shared_ptr<Y>&&);

        // constructors from std::weak_ptr's
        template<typename Y>
        explicit shared_instance(std::weak_ptr<Y> const&);

        // constructors from std:unique_ptr's
        template<typename Y, typename Deleter>
        shared_instance(std::unique_ptr<Y, Deleter>&&);

        // constructors from plain pointers
        template<typename Y>
        explicit shared_instance(Y*);

        template<typename Y, typename Deleter>
        shared_instance(Y*, Deleter);

        template<typename Y, typename Deleter, typename Alloc>
        shared_instance(Y*, Deleter, Alloc);

        shared_instance& operator=(shared_instance const& other);
        shared_instance& operator=(shared_instance&& other);

        shared_instance& operator=(std::shared_ptr<T> const& other);
        shared_instance& operator=(std::shared_ptr<T>&& other);

        template<typename Y, typename Deleter>
        shared_instance& operator=(std::unique_ptr<Y,Deleter>&& r);

        T& operator&() const;
        T& get() const;

        explicit operator std::shared_ptr<T>() const;

        long use_count() const;
        bool unique() const;

        void swap(shared_instance<T>&);
        void swap(std::shared_ptr<T>&);

        std::shared_ptr<T> ptr() const;

        template<typename Y>
        bool owner_before(const shared_instance<Y>&) const;

        template<typename Y>
        bool owner_before(const std::shared_ptr<Y>&) const;

        template<typename Y>
        bool owner_before(const std::weak_ptr<Y>&) const;

    private:
        template<typename Y>
        void check(Y const&) const;

        std::shared_ptr<T> m_obj;
    };

    template<typename T>
    template<typename Y>
    shared_instance<T>::shared_instance(const shared_instance<Y>& other)
        : m_obj(other.ptr())
    {
        check(m_obj);
    }

    template<typename T>
    template<typename Y>
    shared_instance<T>::shared_instance(shared_instance<Y>&& other)
        : m_obj(std::move(other.ptr()))
    {
    }

    template<typename T>
    template<typename Y>
    shared_instance<T>::shared_instance(Y* obj)
        : m_obj(obj)
    {
        check(m_obj);
    }

    template<typename T>
    template<typename Y, typename Deleter>
    shared_instance<T>::shared_instance(Y* obj, Deleter deleter)
        : m_obj(obj, deleter)
    {
        check(m_obj);
    }

    template<typename T>
    template<typename Y, typename Deleter, typename Alloc>
    shared_instance<T>::shared_instance(Y* obj, Deleter deleter, Alloc alloc)
        : m_obj(obj, deleter, alloc)
    {
        check(m_obj);
    }

    template<typename T>
    shared_instance<T>::shared_instance(std::shared_ptr<T> const& other)
        : m_obj(other)
    {
        check(m_obj);
    }

    template<typename T>
    template<typename Y>
    shared_instance<T>::shared_instance(std::shared_ptr<Y> const& other)
        : m_obj(other)
    {
        check(m_obj);
    }

    template<typename T>
    shared_instance<T>::shared_instance(std::shared_ptr<T>&& other)
        : m_obj(std::move(other))
    {
        check(m_obj);
    }

    template<typename T>
    template<typename Y>
    shared_instance<T>::shared_instance(std::shared_ptr<Y>&& other)
        : m_obj(std::move(other))
    {
        check(m_obj);
    }

    template<typename T>
    template<typename Y>
    shared_instance<T>::shared_instance(const std::weak_ptr<Y>& other)
        : m_obj(other.lock())
    {
        check(m_obj);
    }

    template<typename T>
    template<typename Y, typename Deleter>
    shared_instance<T>::shared_instance(std::unique_ptr<Y, Deleter>&& other)
        : m_obj(std::move(other))
    {
        check(m_obj);
    }

    template<typename T>
    shared_instance<T>& shared_instance<T>::operator=(shared_instance const& other)
    {
        m_obj = other.m_obj;
        return *this;
    }

    template<typename T>
    shared_instance<T>& shared_instance<T>::operator=(shared_instance&& other)
    {
        m_obj = std::move(other.m_obj);
        return *this;
    }

    template<typename T>
    shared_instance<T>& shared_instance<T>::operator=(std::shared_ptr<T> const& other)
    {
        check(other);
        m_obj = other;
        return *this;
    }

    template<typename T>
    shared_instance<T>& shared_instance<T>::operator=(std::shared_ptr<T>&& other)
    {
        check(other);
        m_obj = std::move(other);
        return *this;
    }

    template<typename T>
    template<typename Y, typename Deleter>
    shared_instance<T>& shared_instance<T>::operator=(std::unique_ptr<Y,Deleter>&& other)
    {
        check(other);
        m_obj = std::move(other);
        return *this;
    }

    template<typename T>
    shared_instance<T>::operator std::shared_ptr<T>() const
    {
        return m_obj;
    }

    template<typename T>
    T&
    shared_instance<T>::operator&() const
    {
        return *m_obj;
    }

    template<typename T>
    T&
    shared_instance<T>::get() const
    {
        return *m_obj.get();
    }

    template<typename T>
    void
    shared_instance<T>::swap(shared_instance<T>& other)
    {
        m_obj.swap(other.m_obj);
    }

    template<typename T>
    void
    shared_instance<T>::swap(std::shared_ptr<T>& ptr)
    {
        if (!ptr)
        {
            throw std::invalid_argument("attempt to set shared_instance to null");
        }

        m_obj.swap(ptr);
    }

    template<typename T>
    template<typename Y>
    void
    shared_instance<T>::check(Y const& ptr) const
    {
        if (!ptr)
        {
            throw std::invalid_argument("attempt to set shared_instance to null");
        }
    }

    template<typename T>
    std::shared_ptr<T>
    shared_instance<T>::ptr() const
    {
        return m_obj;
    }

    template<typename T>
    long
    shared_instance<T>::use_count() const
    {
        return m_obj.use_count();
    }

    template<typename T>
    bool
    shared_instance<T>::unique() const
    {
        return m_obj.unique();
    }

    template<typename T>
    template<typename Y>
    bool
    shared_instance<T>::owner_before(const shared_instance<Y>& other) const
    {
        return m_obj.owner_before(other.m_obj);
    }

    template<typename T>
    template<typename Y>
    bool
    shared_instance<T>::owner_before(const std::shared_ptr<Y>& other) const
    {
        return m_obj.owner_before(other);
    }

    template<typename T>
    template<typename Y>
    bool
    shared_instance<T>::owner_before(const std::weak_ptr<Y>& other) const
    {
        return m_obj.owner_before(other);
    }

    // compare two shared_instances
    template<typename T, typename U>
    bool operator==(const shared_instance<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs.ptr() == rhs.ptr();
    }

    template<typename T, typename U>
    bool operator!=(const shared_instance<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs.ptr() != rhs.ptr();
    }

    template<typename T, typename U>
    bool operator<(const shared_instance<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs.ptr() < rhs.ptr();
    }

    template<typename T, typename U>
    bool operator>(const shared_instance<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs.ptr() > rhs.ptr();
    }

    template<typename T, typename U>
    bool operator<=(const shared_instance<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs.ptr() <= rhs.ptr();
    }

    template<typename T, typename U>
    bool operator>=(const shared_instance<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs.ptr() >= rhs.ptr();
    }

    // compare shared_instance with shared_ptr (rhs)
    template<typename T, typename U>
    bool operator==(const shared_instance<T>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() == rhs;
    }

    template<typename T, typename U>
    bool operator!=(const shared_instance<T>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() != rhs;
    }

    template<typename T, typename U>
    bool operator<(const shared_instance<T>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() < rhs;
    }

    template<typename T, typename U>
    bool operator>(const shared_instance<T>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() > rhs;
    }

    template<typename T, typename U>
    bool operator<=(const shared_instance<T>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() <= rhs;
    }

    template<typename T, typename U>
    bool operator>=(const shared_instance<T>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() >= rhs;
    }

    // compare shared_instance with shared_ptr (lhs)
    template<typename T, typename U>
    bool operator==(const std::shared_ptr<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs == rhs.ptr();
    }

    template<typename T, typename U>
    bool operator!=(const std::shared_ptr<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs != rhs.ptr();
    }

    template<typename T, typename U>
    bool operator<(const std::shared_ptr<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs < rhs.ptr();
    }

    template<typename T, typename U>
    bool operator>(const std::shared_ptr<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs > rhs.ptr();
    }

    template<typename T, typename U>
    bool operator<=(const std::shared_ptr<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs <= rhs.ptr();
    }

    template<typename T, typename U>
    bool operator>=(const std::shared_ptr<T>& lhs, const shared_instance<U>& rhs)
    {
        return lhs >= rhs.ptr();
    }

    template<typename T, typename U, typename V>
    std::basic_ostream<U, V>& operator<< (std::basic_ostream<U, V>& out, shared_instance<T> const& obj)
    {
        out << obj.ptr();
        return out;
    }

    template<typename Deleter, typename T>
    Deleter* get_deleter(shared_instance<T> const& ptr)
    {
        return get_deleter<Deleter>(ptr.ptr());
    }

    template<typename Target, typename Source>
    shared_instance<Target> static_pointer_cast(shared_instance<Source> const& obj)
    {
        return shared_instance<Target>{static_pointer_cast<Target>(obj.ptr())};
    }

    template<typename Target, typename Source>
    shared_instance<Target> const_pointer_cast(shared_instance<Source> const& obj)
    {
        return shared_instance<Target>{const_pointer_cast<Target>(obj.ptr())};
    }

    template<typename T>
    void
    swap(shared_instance<T>& foo, shared_instance<T>& bar)
    {
        foo.swap(bar);
    }

}

#endif
