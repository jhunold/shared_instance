// shared_instance_test.cpp
//
// Copyright Robin Eckert 2014
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "rebox/shared_instance.hpp"


namespace rebox
{
    class Base
    {
    public:
        explicit Base(int& deleteCount)
            : m_deleteCount(deleteCount)
        {
        }

        virtual ~Base()
        {
            ++m_deleteCount;
        }

        int foo()
        {
            return 42;
        }

    private:
        int& m_deleteCount;
    };


    class Derived : public Base
    {
    public:
        using Base::Base;
    };


    class TestDeleter
    {
    public:
        TestDeleter(int& useCount)
            : m_useCount(useCount)
        {
        }

        void operator() (Base* value)
        {
            delete value;
            ++m_useCount;
        }

    private:
        int& m_useCount;
    };



    BOOST_AUTO_TEST_CASE(construct_from_plain_null_pointer)
    {
        // from null pointer
        BOOST_CHECK_THROW(shared_instance<int>(static_cast<int*>(nullptr)),
                          std::invalid_argument);
    }


    BOOST_AUTO_TEST_CASE(construct_from_plain_pointer)
    {
        int deleteCount{};

        {
            Base* ptr{new Base(deleteCount)};
            shared_instance<Base> foo(ptr);
            BOOST_CHECK_EQUAL(&foo.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }

    BOOST_AUTO_TEST_CASE(construct_from_plain_pointer_with_deleter)
    {
        int deleteCount{};
        int deleterUseCount{};

        {
            Base* ptr{new Base(deleteCount)};
            shared_instance<Base> foo(ptr, TestDeleter(deleterUseCount));

            BOOST_CHECK_EQUAL(&foo.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
            BOOST_CHECK_EQUAL(deleterUseCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_EQUAL(deleterUseCount, 1);
    }

    BOOST_AUTO_TEST_CASE(construct_from_plain_ptr_with_deleter_and_allocator)
    {
        int deleteCount{};
        int deleterUseCount{};

        {
            Base* ptr{new Base(deleteCount)};
            shared_instance<Base> foo(ptr,
                                      TestDeleter(deleterUseCount),
                                      std::allocator<Base>());

            BOOST_CHECK_EQUAL(&foo.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
            BOOST_CHECK_EQUAL(deleterUseCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_EQUAL(deleterUseCount, 1);
    }

    BOOST_AUTO_TEST_CASE(construct_from_weak_pointer)
    {
        int deleteCount{};

        {
            Base* ptr{new Base(deleteCount)};
            std::shared_ptr<Base> shared(ptr);
            std::weak_ptr<Base> weak(shared);

            shared_instance<Base> foo(weak);

            BOOST_CHECK_EQUAL(&foo.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }

    BOOST_AUTO_TEST_CASE(construct_from_deleted_weak_pointer)
    {
        int deleteCount{};
        std::weak_ptr<Base> weak;

        {
            Base* ptr{new Base(deleteCount)};
            std::shared_ptr<Base> shared(ptr);
            weak = shared;
        }
        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_THROW(shared_instance<Base> foo(weak),
                          std::invalid_argument);
    }

    BOOST_AUTO_TEST_CASE(construct_from_unique_pointer)
    {
        int deleteCount{};

        {
            Base* ptr{new Base(deleteCount)};

            shared_instance<Base> foo{std::unique_ptr<Base>{ptr}};

            BOOST_CHECK_EQUAL(&foo.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }



    BOOST_AUTO_TEST_CASE(construct_from_shared_instance)
    {
        int deleteCount{};

        {
            Base* ptr{new Base(deleteCount)};
            shared_instance<Base> foo{ptr};
            BOOST_CHECK_EQUAL(deleteCount, 0);

            {
                shared_instance<Base> bar{foo};
                BOOST_CHECK_EQUAL(&bar.get(), ptr);
                BOOST_CHECK_EQUAL(deleteCount, 0);
            }
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }


    BOOST_AUTO_TEST_CASE(construct_from_related_shared_instance)
    {
        int deleteCount{};

        {
            Derived* ptr{new Derived(deleteCount)};
            shared_instance<Derived> foo{ptr};
            BOOST_CHECK_EQUAL(deleteCount, 0);

            {
                shared_instance<Base> bar{foo};
                BOOST_CHECK_EQUAL(&bar.get(), ptr);
                BOOST_CHECK_EQUAL(deleteCount, 0);
            }
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }


    BOOST_AUTO_TEST_CASE(move_from_shared_instance)
    {
        int deleteCount{};

        {
            Base* ptr{new Base(deleteCount)};
            shared_instance<Base> foo{shared_instance<Base>{ptr}};
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }


    BOOST_AUTO_TEST_CASE(move_from_related_shared_instance)
    {
        int deleteCount{};

        {
            Derived* ptr{new Derived(deleteCount)};
            shared_instance<Base> foo{shared_instance<Derived>{ptr}};
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }


    BOOST_AUTO_TEST_CASE(construct_from_null_shared_ptr)
    {
        std::shared_ptr<int> foo;
        BOOST_CHECK_THROW(shared_instance<int>{foo}, std::invalid_argument);
    }


    BOOST_AUTO_TEST_CASE(construct_from_shared_ptr)
    {
        int deleteCount{};

        {
            Base* ptr{new Base(deleteCount)};
            std::shared_ptr<Base> foo{ptr};
            shared_instance<Base> bar{foo};
            BOOST_CHECK_EQUAL(&bar.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }


    BOOST_AUTO_TEST_CASE(construct_from_related_shared_ptr)
    {
        int deleteCount{};

        {
            Derived* ptr{new Derived(deleteCount)};
            std::shared_ptr<Derived> foo{ptr};
            shared_instance<Base> bar{foo};
            BOOST_CHECK_EQUAL(&bar.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }


    BOOST_AUTO_TEST_CASE(move_from_shared_ptr)
    {
        int deleteCount{};

        {
            Base* ptr{new Base(deleteCount)};
            shared_instance<Base> bar{std::shared_ptr<Base>{ptr}};
            BOOST_CHECK_EQUAL(&bar.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }


    BOOST_AUTO_TEST_CASE(move_from_related_shared_ptr)
    {
        int deleteCount{};

        {
            Derived* ptr{new Derived(deleteCount)};
            shared_instance<Base> bar{std::shared_ptr<Derived>{ptr}};
            BOOST_CHECK_EQUAL(&bar.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }


    BOOST_AUTO_TEST_CASE(test_pod)
    {
        int* ptr{new int{42}};
        shared_instance<int> foo{ptr};

        BOOST_CHECK_EQUAL(&foo.get(), ptr);
        BOOST_CHECK_EQUAL(foo.get(), 42);
    }


    BOOST_AUTO_TEST_CASE(test_deref)
    {
        int deleteCount{};

        Base* ptr{new Base{deleteCount}};
        shared_instance<Base> foo{ptr};

        BOOST_CHECK_EQUAL(&foo.get(), ptr);
        BOOST_CHECK_EQUAL(foo.get().foo(), 42);
    }


    BOOST_AUTO_TEST_CASE(test_assignent)
    {
        int deleteCount{};
        int deleteCount2{};
        Base* ptr{new Base{deleteCount}};
        Base* ptr2{new Base{deleteCount2}};

        {
            shared_instance<Base> foo{ptr};
            BOOST_CHECK_EQUAL(deleteCount, 0);
            BOOST_CHECK_EQUAL(deleteCount2, 0);

            {
                shared_instance<Base> bar{ptr2};
                foo = bar;

                BOOST_CHECK_EQUAL(deleteCount, 1);
                BOOST_CHECK_EQUAL(deleteCount2, 0);
            }

            BOOST_CHECK_EQUAL(deleteCount, 1);
            BOOST_CHECK_EQUAL(deleteCount2, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_EQUAL(deleteCount2, 1);
    }

    BOOST_AUTO_TEST_CASE(test_cast_and_move_assignment)
    {
        int deleteCount{};
        int deleteCount2{};
        Base* ptr{new Base{deleteCount}};

        shared_instance<Base> foo{ptr};

        {
            std::shared_ptr<Base> bar{std::shared_ptr<Base>(foo)};

            BOOST_CHECK_EQUAL(bar.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);

            foo = shared_instance<Base>(new Base{deleteCount2});
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }

    BOOST_AUTO_TEST_CASE(test_assignment_from_shared_ptr)
    {
        int deleteCount{};
        int deleteCount2{};
        Base* ptr{new Base{deleteCount}};
        Base* ptr2{new Base{deleteCount2}};

        std::shared_ptr<Base> bar{ptr2};

        BOOST_CHECK_EQUAL(deleteCount, 0);
        BOOST_CHECK_EQUAL(deleteCount2, 0);

        {
            shared_instance<Base> foo{ptr};

            BOOST_CHECK_EQUAL(deleteCount, 0);
            BOOST_CHECK_EQUAL(deleteCount2, 0);

            foo = bar;

            BOOST_CHECK_EQUAL(deleteCount, 1);
            BOOST_CHECK_EQUAL(deleteCount2, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_EQUAL(deleteCount2, 0);
    }

    BOOST_AUTO_TEST_CASE(test_assignment_from_null_shared_ptr)
    {
        int deleteCount{};
        Base* ptr{new Base{deleteCount}};

        std::shared_ptr<Base> empty;
        shared_instance<Base> foo(ptr);

        BOOST_CHECK_THROW(foo = empty, std::invalid_argument);
        BOOST_CHECK_EQUAL(deleteCount, 0);
        BOOST_CHECK_EQUAL(&foo.get(), ptr);
    }

    BOOST_AUTO_TEST_CASE(test_move_assignment_from_null_shared_ptr)
    {
        int deleteCount{};
        Base* ptr{new Base{deleteCount}};

        shared_instance<Base> foo(ptr);

        BOOST_CHECK_THROW(foo = std::shared_ptr<Base>(), std::invalid_argument);
        BOOST_CHECK_EQUAL(deleteCount, 0);
        BOOST_CHECK_EQUAL(&foo.get(), ptr);
    }

    BOOST_AUTO_TEST_CASE(test_move_assignment_from_unique_ptr)
    {
        int deleteCount{};
        int deleteCount2{};
        Base* ptr{new Base{deleteCount}};
        Derived* ptr2{new Derived{deleteCount2}};

        {
            shared_instance<Base> foo(ptr);
            foo = std::unique_ptr<Derived>(ptr2);

            BOOST_CHECK_EQUAL(deleteCount, 1);
            BOOST_CHECK_EQUAL(deleteCount2, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_EQUAL(deleteCount2, 1);
    }

    BOOST_AUTO_TEST_CASE(test_move_assignment_from_null_unique_ptr)
    {
        int deleteCount{};
        Base* ptr{new Base{deleteCount}};

        {
            shared_instance<Base> foo(ptr);
            BOOST_CHECK_THROW(foo = std::unique_ptr<Derived>(),
                              std::invalid_argument);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }

    BOOST_AUTO_TEST_CASE(test_swap_shared_instance)
    {
        int deleteCount{};
        int deleteCount2{};
        Base *ptr{new Base{deleteCount}};
        Base *ptr2{new Base{deleteCount2}};

        {
            shared_instance<Base> foo(ptr);

            {
                shared_instance<Base> bar(ptr2);

                foo.swap(bar);

                BOOST_CHECK_EQUAL(&foo.get(), ptr2);
                BOOST_CHECK_EQUAL(&bar.get(), ptr);

                BOOST_CHECK_EQUAL(deleteCount, 0);
                BOOST_CHECK_EQUAL(deleteCount2, 0);
            }

            BOOST_CHECK_EQUAL(deleteCount, 1);
            BOOST_CHECK_EQUAL(deleteCount2, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_EQUAL(deleteCount2, 1);
    }

    BOOST_AUTO_TEST_CASE(test_external_swap_shared_instance)
    {
        int deleteCount{};
        int deleteCount2{};
        Base *ptr{new Base{deleteCount}};
        Base *ptr2{new Base{deleteCount2}};

        {
            shared_instance<Base> foo(ptr);

            {
                shared_instance<Base> bar(ptr2);

                swap(foo, bar);

                BOOST_CHECK_EQUAL(&foo.get(), ptr2);
                BOOST_CHECK_EQUAL(&bar.get(), ptr);

                BOOST_CHECK_EQUAL(deleteCount, 0);
                BOOST_CHECK_EQUAL(deleteCount2, 0);
            }

            BOOST_CHECK_EQUAL(deleteCount, 1);
            BOOST_CHECK_EQUAL(deleteCount2, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_EQUAL(deleteCount2, 1);
    }


    BOOST_AUTO_TEST_CASE(test_swap_shared_ptr)
    {
        int deleteCount{};
        int deleteCount2{};
        Base *ptr{new Base{deleteCount}};
        Base *ptr2{new Base{deleteCount2}};

        {
            shared_instance<Base> foo(ptr);

            {
                std::shared_ptr<Base> bar(ptr2);

                foo.swap(bar);

                BOOST_CHECK_EQUAL(&foo.get(), ptr2);
                BOOST_CHECK_EQUAL(bar.get(), ptr);

                BOOST_CHECK_EQUAL(deleteCount, 0);
                BOOST_CHECK_EQUAL(deleteCount2, 0);
            }

            BOOST_CHECK_EQUAL(deleteCount, 1);
            BOOST_CHECK_EQUAL(deleteCount2, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
        BOOST_CHECK_EQUAL(deleteCount2, 1);
    }

    BOOST_AUTO_TEST_CASE(test_swap_null_shared_ptr)
    {
        int deleteCount{};
        Base *ptr{new Base{deleteCount}};

        {
            shared_instance<Base> foo(ptr);

            std::shared_ptr<Base> bar;
            BOOST_CHECK_THROW(foo.swap(bar), std::invalid_argument);
            BOOST_CHECK_EQUAL(&foo.get(), ptr);
            BOOST_CHECK_EQUAL(deleteCount, 0);
        }

        BOOST_CHECK_EQUAL(deleteCount, 1);
    }

    BOOST_AUTO_TEST_CASE(test_use_count)
    {
        shared_instance<int> foo{new int(42)};
        BOOST_CHECK_EQUAL(foo.use_count(), 1);

        {
            shared_instance<int> bar{foo};
            BOOST_CHECK_EQUAL(foo.use_count(), 2);
            BOOST_CHECK_EQUAL(bar.use_count(), 2);
        }

        BOOST_CHECK_EQUAL(foo.use_count(), 1);
    }

    BOOST_AUTO_TEST_CASE(test_unique)
    {
        shared_instance<int> foo{new int(42)};
        BOOST_CHECK_EQUAL(foo.unique(), true);

        {
            shared_instance<int> bar{foo};
            BOOST_CHECK_EQUAL(foo.unique(), false);
            BOOST_CHECK_EQUAL(bar.unique(), false);
        }

        BOOST_CHECK_EQUAL(foo.unique(), true);
    }

    BOOST_AUTO_TEST_CASE(test_owner_before_shared_instance)
    {
        shared_instance<int> foo{new int(42)};
        shared_instance<int> bar{new int(23)};

        BOOST_CHECK_NE(foo.owner_before(bar), bar.owner_before(foo));

        BOOST_CHECK_EQUAL(foo.owner_before(foo), false);
        BOOST_CHECK_EQUAL(bar.owner_before(bar), false);
    }

    BOOST_AUTO_TEST_CASE(test_owner_before_shared_ptr)
    {
        shared_instance<int> foo{new int(42)};
        std::shared_ptr<int> bar{new int(23)};

        foo.owner_before(bar);

        BOOST_CHECK_EQUAL(foo.owner_before(foo), false);
        BOOST_CHECK_EQUAL(bar.owner_before(bar), false);
    }

    BOOST_AUTO_TEST_CASE(test_owner_before_weak_ptr)
    {
        shared_instance<int> foo{new int(42)};
        std::shared_ptr<int> qux{new int(23)};
        std::weak_ptr<int> bar{qux};

        foo.owner_before(bar);

        BOOST_CHECK_EQUAL(foo.owner_before(foo), false);
        BOOST_CHECK_EQUAL(bar.owner_before(bar), false);
    }

    BOOST_AUTO_TEST_CASE(test_static_pointer_cast)
    {
        int deleteCount{};
        Derived* ptr{new Derived{deleteCount}};

        shared_instance<Base> foo{ptr};

        shared_instance<Derived> bar{static_pointer_cast<Derived>(foo)};

        BOOST_CHECK_EQUAL(&bar.get(), ptr);
    }

    BOOST_AUTO_TEST_CASE(test_const_pointer_cast)
    {
        shared_instance<int const> foo{new int(42)};
        shared_instance<int> bar{const_pointer_cast<int>(foo)};
        BOOST_CHECK_EQUAL(bar.get(), 42);
    }

    BOOST_AUTO_TEST_CASE(test_get_deleter)
    {
        std::shared_ptr<int> ptr{std::make_shared<int>(42)};
        shared_instance<int> foo{ptr};
        BOOST_CHECK_EQUAL(get_deleter<void(*)(int*)>(ptr),
                          get_deleter<void(*)(int*)>(foo));
    }

    BOOST_AUTO_TEST_CASE(test_compare_shared_instances)
    {
        shared_instance<int> foo{std::make_shared<int>(42)};
        shared_instance<int> bar{std::make_shared<int>(42)};

        BOOST_CHECK(foo == foo);
        BOOST_CHECK(bar == bar);
        BOOST_CHECK(!(foo == bar));
        BOOST_CHECK(!(bar == foo));

        BOOST_CHECK(foo != bar);
        BOOST_CHECK(bar != foo);
        BOOST_CHECK(!(foo != foo));
        BOOST_CHECK(!(bar != bar));

        BOOST_CHECK((foo < bar) != (bar < foo));
        BOOST_CHECK((foo > bar) != (bar > foo));

        BOOST_CHECK((foo <= bar) != (bar <= foo));
        BOOST_CHECK(foo <= foo);
        BOOST_CHECK(bar <= bar);

        BOOST_CHECK((foo >= bar) != (bar >= foo));
        BOOST_CHECK(foo >= foo);
        BOOST_CHECK(bar >= bar);
    }

    BOOST_AUTO_TEST_CASE(test_compare_shared_instance_with_shared_ptr)
    {
        shared_instance<int> foo{std::make_shared<int>(42)};
        std::shared_ptr<int> bar{std::make_shared<int>(42)};

        BOOST_CHECK(foo == foo);
        BOOST_CHECK(bar == bar);
        BOOST_CHECK(!(foo == bar));
        BOOST_CHECK(!(bar == foo));

        BOOST_CHECK(foo != bar);
        BOOST_CHECK(bar != foo);
        BOOST_CHECK(!(foo != foo));
        BOOST_CHECK(!(bar != bar));

        BOOST_CHECK((foo < bar) != (bar < foo));
        BOOST_CHECK((foo > bar) != (bar > foo));

        BOOST_CHECK((foo <= bar) != (bar <= foo));
        BOOST_CHECK(foo <= foo);
        BOOST_CHECK(bar <= bar);

        BOOST_CHECK((foo >= bar) != (bar >= foo));
        BOOST_CHECK(foo >= foo);
        BOOST_CHECK(bar >= bar);
    }

}
