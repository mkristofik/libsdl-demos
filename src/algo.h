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
#ifndef ALGO_H
#define ALGO_H

#include <algorithm>
#include <memory>

template <class Container, class T>
bool contains(const Container &c, const T &elem)
{
    return find(std::begin(c), std::end(c), elem) != std::end(c);
}

template <class T, class U, class V>
T bound(const T &val, const U &minVal, const V &maxVal)
{
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

template <class T, class... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif
