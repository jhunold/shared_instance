shared_instance
===============

a wrapper around `std::shared_ptr` that cannot be null

Author: Robin Eckert `<const@const-iterator.de>`

### Test results

@               | Build        
----------------|--------------
Master branch:  | [![Build Status](https://travis-ci.org/jhunold/shared_instance.svg?branch=master)](https://travis-ci.org/jhunold/shared_instance) 
Travis branch:  | [![Build Status](https://travis-ci.org/jhunold/shared_instance.svg?branch=travis)](https://travis-ci.org/jhunold/shared_instance) 


Introduction
------------

`shared_instance` is a wrapper around `std::shared_ptr` ensuring that
the pointer is always set to value other than `nullptr`. Attempts to
construct a `shared_instance` with a null value lead to an
exception. The purpose is to notice errorneous null pointers as soon
as possible -- when the pointer is set rather than when the pointer is
used.

Motivation
----------

`std::shared_ptr`s are an effective way to store data that needs shared
ownership. Like plain pointers, they also support to be set as a null
pointer. This is adequate (and needed) in many places but can lead to
delayed detection of errors. Often, `shared_ptr`s are required and
expected to be non-null. If such a constraint is violated, it must be
manually checked or is only detected when the pointer is dereferenced
which can be too late. I will present two practical examples: function
calls and containers of `shared_instance`s.

### Example: Function Call ###

Suppose you have an instance of type `Foo` which is needed to have
shared ownership:

    class Foo;

    std::shared_ptr<Foo> f{std::make_shared<Foo>()};

Second, the instance needs to be processed by a function `bar`:

    void bar(std::shared_ptr<Foo const>);

    bar(f);
    bar(std::shared_ptr<Foo>());

In this example, `bar` receives a (shared pointer to an) instance of
`Foo` or a null pointer. Let's assume that is doesn't make sense for
`bar` to get a null pointer, as it always needs an instance. If `bar`
only uses the pointer locally and does not store it, it is better to
pass per reference:

    void bar(Foo const&);

    bar(*f);

Now, bar doesn't need to check its argument if it is a null
pointer. It transfers the responsibility to provide a valid instance
to its caller. This is has the following advantages:

 - The interface of `bar` states explicitly that it needs a valid
   instance of `Foo`.

 - The interface of `bar` becomes simpler, as it doesn't need to
   specify error reporting means as exceptions in the interface.

 - If `bar` calls multiple subfunctions itself, these would need to
   check the null pointer just as well, leading to excessive checking
   in the code.

 - `bar` now also supports other instances of `Foo` (like on the
   stack, managed differently) as it doesn't depend on `shared_ptr`
   anymore.

 - The interface of `bar` also explicitly states that it doesn't
   claim ownership of its argument.

As of these advantages, I prefer this type of calling method. But this
only works if `bar` doesn't need to claim ownership. For example, if
`bar` would be a constructor of a class storing a shared `Foo`, this
calling method won't work. If we resort to passing a
`shared_ptr<Foo>`, we lose the advantages above. To overcome this
problem, we can use the `shared_instance` class described in this
document. A `shared_instance` simply resembles a `shared_ptr` which
cannot be set to null. Attempts to set a `shared_instance` to null
will yield an `invalid_argument` exception. Also, default constructors
and methods like `reset()` aren't available and won't compile in the
first place.

So we define the interface of `bar` as follows:

    void bar(shared_instance<Foo>);

When calling `bar` we now need an actual instance:

    shared_instance<Foo> f{std::make_shared<Foo>()};
    bar(f);

Let's try to call `bar` with a null pointer:

    shared_ptr<Foo> f;  // is null
    bar(shared_instance<Foo>(f));

This will throw an `std::invalid_argument` exception when constructing
the `shared_instance<Foo>`. So the error is already detected before
calling `bar` and not deep inside `bar` (or its subfunctions) when the
pointer is finally dereferenced and used. Also, no checks in `bar` are
needed, simplifying its implementation.

### Example: Container of `shared_instance`s ###

Consider a vector of object of type `Node` which need shared ownership:

    class Node;
    std::vector<std::shared_ptr<Node>> v;

Often when building such constructs, the pointers in the vector are
expected to be always set. Suppose we need to do some operation on all
elements of `v`:

    void op(Node const& node);

    for (auto p : v)
    {
        op(*p);
    }

In this implementation, we trust the creator of v that all pointers
are set. If we want to be safe, we would need to check each pointer
either in the loop, in the operation itself or someplace earlier in
our library/program. Especially when choosing the latter option, these
checks can easily be forgotten. We also need to specify a behaviour if
we encounter a null pointer (throw an exception, ignore the element,
set an error code, ...).

As it is probably incorrect to insert a null pointer into the vector
in the first place, this should be enforced as early as possible. So
we define the vector using `shared_instance` instead of `shared_ptr`:

    std::vector<shared_instance<Foo>> v;

Now, all parts of the program using such a vector don't need to check
if an instance is there -- it is enforced by shared_instance when
inserting into it. If an errorneous attempt is made to insert a null
pointer, the exception is thrown directly where the error occurs.


Usage
-----

The `shared_instance` template is designed to be used in tight
conjunction with `std::shared_ptr`. The interface resembles that of
`std::shared_ptr` with the extension that it throws
`std::invalid_argument` whenever an attempt is made to set it to null.

A shared_instance can be created from plain pointers or from
`std::shared_ptr`s:

    shared_instance f{new Foo()};
    shared_instance f{std::shared_ptr<Foo>{new Foo()}};
    shared_instance f{std::make_shared<Foo>()};

For constructing new instances, I recommend using the last
alternative. Constructors specifying custom allocators and deleters
are also available.

Attempts to create null `shared_instance`s yield an `std::invalid_argument` exceptions:

    Foo *plainEmpty{nullptr};

    shared_instance f{plainEmpty};              // throws std::invalid_argument
    shared_instance f{std::shared_ptr<Foo>{}};  // throws std::invalid_argument

Construction from `std::weak_ptr` and move construction from
`std::unique_ptr` is supported as well. When constructing from
`std::weak_ptr`, the exception is throws both if the `std::weak_ptr`
is null and if the object pointed to by it was deleted.

The instance pointed to can be used like with
`std::reference_wrapper`:

    shared_instance f{std::make_shared<Foo>()};
    doSomething(f);                             // always safe, no checks needed
    f.get(). ...();                             // always safe, no checks needed

Information members from `shared_ptr` are also available:

    ... = f.use_count();
    ... = f.unique();

Also, when a `shared_ptr` is needed, `shared_instance` it can be
explicitly casted to one or a `shared_ptr` can be obtained using
`ptr()`:

    shared_instance instance{std::make_shared<Foo>()};
    shared_ptr<Foo> shared{instance.ptr()};

Cast are also possible:

    class Base { };
    class Derived : public Base { };

    shared_instance<Base const> instance{std::make_shared<Derived>()};

    shared_instance<Derived const> derived = static_pointer_cast<Derived const>(instance);
    shared_instance<Base> deconsted = const_pointer_cast<Base>(instance);

`dynamic_pointer_cast` is not available. Use `instance.ptr()` instead:

    shared_instance<Derived const> derived = std::dynamic_pointer_cast<Target>(instance.ptr());

Reference
---------

$TODO

Implementation notes
--------------------

The implementation is implemented as a simple wrapper around
`std::shared_ptr`. This reduces the error potential implementing the
shared ownership correctly, yields better integration with
`std::shared_ptr` and simplifies the implementation.

Prospect
--------

I'm yet undecided if it would be useful to have `unique_instance`. It
could probably be useful when the need arises to have single-ownership
objects which don't fit on the stack, especially when returning them
from a function.

License
-------

`shared_instance` is distributed under the Boost Software License,
Version 1.0.  See accompanying file `LICENSE` or copy at
[http://www.boost.org/LICENSE\_1\_0.txt](http://www.boost.org/LICENSE_1_0.txt).
