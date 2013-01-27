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

template <class Container, class T>
bool contains(const Container& c, const T& elem)
{
    return find(std::begin(c), std::end(c), elem) != std::end(c);
}

#endif
