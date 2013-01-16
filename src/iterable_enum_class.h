/*
    Copyright (C) 2012-2013 by Michael Kristofik <kristo605@gmail.com>
    Part of the libsdl-demos project.
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    or at your option any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY.
 
    See the COPYING.txt file for more details.
*/

// Provide the necessary functions to treat an enum class like a container in a
// range-based for-loop.  This works for classic enums too but is probably most
// useful for a strongly-typed enum class.  Requires two sentry values at the
// end of the enum, in the exact order listed.
// source: http://stackoverflow.com/q/8498300/46821

// Example usage:
//
// enum class Foo {BAR, BAZ, QUUX, XYZZY, _last, _first = BAR};
// ITERABLE_ENUM_CLASS(Foo);
// for (auto f : Foo()) {
//     // doSomething(f);
// }

#ifndef ITERABLE_ENUM_CLASS_H
#define ITERABLE_ENUM_CLASS_H

#include <type_traits>
#define ITERABLE_ENUM_CLASS(T) \
    inline T operator++(T &t) \
    { \
        using U = std::underlying_type<T>::type; \
        return t = static_cast<T>(U(t) + 1); \
    } \
    inline T operator*(T t) { return t; } \
    inline T begin(T t) { return T::_first; } \
    inline T end(T t) { return T::_last; }

#endif
