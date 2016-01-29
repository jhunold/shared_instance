// shared_instance.hpp -- a std::shared_ptr wrapper that cannot be null
//
// Copyright Robin Eckert 2014
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef REBOX_SHARED_INSTANCE_HPP
#define REBOX_SHARED_INSTANCE_HPP

#include "shared_instance_fwd.hpp"

#include <memory>
#include <stdexcept>

namespace rebox
{
    class throw_invalid_argument
    {
    public:
        void operator()() const
        {
            throw std::invalid_argument("attempt to set shared_instance to null");
        };
    };

    template<typename T, typename Report>
    class shared_instance
    {
    public:
        using type = T;

        shared_instance() = delete;

        // constructors from other shared_instance's
        shared_instance(shared_instance const&) = default;

        template<typename Y, typename Z>
        shared_instance(shared_instance<Y, Z> const&);

        template<typename Y, typename Z>
        shared_instance(shared_instance<Y, Z>&&);

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

        operator T&() const;
        T& get() const;

        explicit operator std::shared_ptr<T>() const;

        long use_count() const;
        bool unique() const;

        template<typename Z>
        void swap(shared_instance<T, Z>&);

        void swap(std::shared_ptr<T>&);

        std::shared_ptr<T> ptr() const;

        template<typename Y, typename Z>
        bool owner_before(const shared_instance<Y, Z>&) const;

        template<typename Y>
        bool owner_before(const std::shared_ptr<Y>&) const;

        template<typename Y>
        bool owner_before(const std::weak_ptr<Y>&) const;

    private:
        template<typename Y>
        void check(Y const&) const;

        std::shared_ptr<T> m_obj;
    };

    template<typename T, typename Report>
    template<typename Y, typename Z>
    shared_instance<T, Report>::shared_instance(const shared_instance<Y, Z>& other)
        : m_obj(other.ptr())
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    template<typename Y, typename Z>
    shared_instance<T, Report>::shared_instance(shared_instance<Y, Z>&& other)
        : m_obj(std::move(other.ptr()))
    {
    }

    template<typename T, typename Report>
    template<typename Y>
    shared_instance<T, Report>::shared_instance(Y* obj)
        : m_obj(obj)
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    template<typename Y, typename Deleter>
    shared_instance<T, Report>::shared_instance(Y* obj, Deleter deleter)
        : m_obj(obj, deleter)
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    template<typename Y, typename Deleter, typename Alloc>
    shared_instance<T, Report>::shared_instance(Y* obj, Deleter deleter, Alloc alloc)
        : m_obj(obj, deleter, alloc)
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    shared_instance<T, Report>::shared_instance(std::shared_ptr<T> const& other)
        : m_obj(other)
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    template<typename Y>
    shared_instance<T, Report>::shared_instance(std::shared_ptr<Y> const& other)
        : m_obj(other)
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    shared_instance<T, Report>::shared_instance(std::shared_ptr<T>&& other)
        : m_obj(std::move(other))
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    template<typename Y>
    shared_instance<T, Report>::shared_instance(std::shared_ptr<Y>&& other)
        : m_obj(std::move(other))
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    template<typename Y>
    shared_instance<T, Report>::shared_instance(const std::weak_ptr<Y>& other)
        : m_obj(other.lock())
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    template<typename Y, typename Deleter>
    shared_instance<T, Report>::shared_instance(std::unique_ptr<Y, Deleter>&& other)
        : m_obj(std::move(other))
    {
        check(m_obj);
    }

    template<typename T, typename Report>
    shared_instance<T, Report>&
    shared_instance<T, Report>::operator=(shared_instance const& other)
    {
        m_obj = other.m_obj;
        return *this;
    }

    template<typename T, typename Report>
    shared_instance<T, Report>&
    shared_instance<T, Report>::operator=(shared_instance&& other)
    {
        m_obj = std::move(other.m_obj);
        return *this;
    }

    template<typename T, typename Report>
    shared_instance<T, Report>&
    shared_instance<T, Report>::operator=(std::shared_ptr<T> const& other)
    {
        check(other);
        m_obj = other;
        return *this;
    }

    template<typename T, typename Report>
    shared_instance<T, Report>&
    shared_instance<T, Report>::operator=(std::shared_ptr<T>&& other)
    {
        check(other);
        m_obj = std::move(other);
        return *this;
    }

    template<typename T, typename Report>
    template<typename Y, typename Deleter>
    shared_instance<T, Report>&
    shared_instance<T, Report>::operator=(std::unique_ptr<Y,Deleter>&& other)
    {
        check(other);
        m_obj = std::move(other);
        return *this;
    }

    template<typename T, typename Report>
    shared_instance<T, Report>::operator std::shared_ptr<T>() const
    {
        return m_obj;
    }

    template<typename T, typename Report>
    shared_instance<T, Report>::operator T&() const
    {
        return *m_obj;
    }

    template<typename T, typename Report>
    T&
    shared_instance<T, Report>::get() const
    {
        return *m_obj.get();
    }

    template<typename T, typename Report>
    template<typename Z>
    void
    shared_instance<T, Report>::swap(shared_instance<T, Z>& other)
    {
        m_obj.swap(other.m_obj);
    }

    template<typename T, typename Report>
    void
    shared_instance<T, Report>::swap(std::shared_ptr<T>& ptr)
    {
        if (!ptr)
        {
            Report()();
        }

        m_obj.swap(ptr);
    }

    template<typename T, typename Report>
    template<typename Y>
    void
    shared_instance<T, Report>::check(Y const& ptr) const
    {
        if (!ptr)
        {
            Report()();
        }
    }

    template<typename T, typename Report>
    std::shared_ptr<T>
    shared_instance<T, Report>::ptr() const
    {
        return m_obj;
    }

    template<typename T, typename Report>
    long
    shared_instance<T, Report>::use_count() const
    {
        return m_obj.use_count();
    }

    template<typename T, typename Report>
    bool
    shared_instance<T, Report>::unique() const
    {
        return m_obj.unique();
    }

    template<typename T, typename Report>
    template<typename Y, typename Z>
    bool
    shared_instance<T, Report>::owner_before(const shared_instance<Y, Z>& other) const
    {
        return m_obj.owner_before(other.m_obj);
    }

    template<typename T, typename Report>
    template<typename Y>
    bool
    shared_instance<T, Report>::owner_before(const std::shared_ptr<Y>& other) const
    {
        return m_obj.owner_before(other);
    }

    template<typename T, typename Report>
    template<typename Y>
    bool
    shared_instance<T, Report>::owner_before(const std::weak_ptr<Y>& other) const
    {
        return m_obj.owner_before(other);
    }

    // compare two shared_instances
    template<typename T, typename TReport, typename U, typename UReport>
    bool operator==(const shared_instance<T, TReport>& lhs, const shared_instance<U, UReport>& rhs)
    {
        return lhs.ptr() == rhs.ptr();
    }

    template<typename T, typename TReport, typename U, typename UReport>
    bool operator!=(const shared_instance<T, TReport>& lhs, const shared_instance<U, UReport>& rhs)
    {
        return lhs.ptr() != rhs.ptr();
    }

    template<typename T, typename TReport, typename U, typename UReport>
    bool operator<(const shared_instance<T, TReport>& lhs, const shared_instance<U, UReport>& rhs)
    {
        return lhs.ptr() < rhs.ptr();
    }

    template<typename T, typename TReport, typename U, typename UReport>
    bool operator>(const shared_instance<T, TReport>& lhs, const shared_instance<U, UReport>& rhs)
    {
        return lhs.ptr() > rhs.ptr();
    }

    template<typename T, typename TReport, typename U, typename UReport>
    bool operator<=(const shared_instance<T, TReport>& lhs, const shared_instance<U, UReport>& rhs)
    {
        return lhs.ptr() <= rhs.ptr();
    }

    template<typename T, typename TReport, typename U, typename UReport>
    bool operator>=(const shared_instance<T, TReport>& lhs, const shared_instance<U, UReport>& rhs)
    {
        return lhs.ptr() >= rhs.ptr();
    }

    // compare shared_instance with shared_ptr (rhs)
    template<typename T, typename Report, typename U>
    bool operator==(const shared_instance<T, Report>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() == rhs;
    }

    template<typename T, typename Report, typename U>
    bool operator!=(const shared_instance<T, Report>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() != rhs;
    }

    template<typename T, typename Report, typename U>
    bool operator<(const shared_instance<T, Report>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() < rhs;
    }

    template<typename T, typename Report, typename U>
    bool operator>(const shared_instance<T, Report>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() > rhs;
    }

    template<typename T, typename Report, typename U>
    bool operator<=(const shared_instance<T, Report>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() <= rhs;
    }

    template<typename T, typename Report, typename U>
    bool operator>=(const shared_instance<T, Report>& lhs, const std::shared_ptr<U>& rhs)
    {
        return lhs.ptr() >= rhs;
    }

    // compare shared_instance with shared_ptr (lhs)
    template<typename T, typename U, typename Report>
    bool operator==(const std::shared_ptr<T>& lhs, const shared_instance<U, Report>& rhs)
    {
        return lhs == rhs.ptr();
    }

    template<typename T, typename U, typename Report>
    bool operator!=(const std::shared_ptr<T>& lhs, const shared_instance<U, Report>& rhs)
    {
        return lhs != rhs.ptr();
    }

    template<typename T, typename U, typename Report>
    bool operator<(const std::shared_ptr<T>& lhs, const shared_instance<U, Report>& rhs)
    {
        return lhs < rhs.ptr();
    }

    template<typename T, typename U, typename Report>
    bool operator>(const std::shared_ptr<T>& lhs, const shared_instance<U, Report>& rhs)
    {
        return lhs > rhs.ptr();
    }

    template<typename T, typename U, typename Report>
    bool operator<=(const std::shared_ptr<T>& lhs, const shared_instance<U, Report>& rhs)
    {
        return lhs <= rhs.ptr();
    }

    template<typename T, typename U, typename Report>
    bool operator>=(const std::shared_ptr<T>& lhs, const shared_instance<U, Report>& rhs)
    {
        return lhs >= rhs.ptr();
    }

    template<typename T, typename U, typename V, typename Report>
    std::basic_ostream<U, V>& operator<< (std::basic_ostream<U, V>& out, shared_instance<T, Report> const& obj)
    {
        out << obj.ptr();
        return out;
    }

    template<typename Deleter, typename T, typename Report>
    Deleter* get_deleter(shared_instance<T, Report> const& ptr)
    {
        return get_deleter<Deleter>(ptr.ptr());
    }

    template<typename Target, typename Source, typename Report>
    shared_instance<Target, Report> static_pointer_cast(shared_instance<Source, Report> const& obj)
    {
        return shared_instance<Target, Report>{static_pointer_cast<Target>(obj.ptr())};
    }

    template<typename Target, typename Source, typename Report>
    shared_instance<Target, Report> const_pointer_cast(shared_instance<Source, Report> const& obj)
    {
        return shared_instance<Target, Report>{const_pointer_cast<Target>(obj.ptr())};
    }

    template<typename T, typename Report>
    void
    swap(shared_instance<T, Report>& foo, shared_instance<T, Report>& bar)
    {
        foo.swap(bar);
    }


    template<typename T, typename Report = throw_invalid_argument, typename... Args>
    shared_instance<T, Report>
    make_shared_instance(Args&&... args)
    {
        return shared_instance<T, Report>{std::make_shared<T>(args...)};
    }

}

#endif
