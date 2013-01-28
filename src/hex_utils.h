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
#ifndef HEX_UTILS_H
#define HEX_UTILS_H

#include "SDL_types.h"
#include "iterable_enum_class.h"
#include <string>
#include <utility>
#include <vector>

using Point = std::pair<Sint16, Sint16>;
const Point hInvalid = {-1, -1};
const Sint16 pHexSize = 72;

bool operator==(const Point &lhs, const Point &rhs);
bool operator!=(const Point &lhs, const Point &rhs);
Point operator+(const Point &lhs, const Point &rhs);

// String representation of a Point.  Use this instead of writing operator<<
// because...
// - Point is really a std::pair.
// - ADL won't find operator<< that isn't in namespace std.
// - It's illegal to add overloads to namespace std.
// source: http://stackoverflow.com/q/5076206/46821
std::string str(const Point &p);

enum class Dir {N, NE, SE, S, SW, NW, _last, _first = N};
ITERABLE_ENUM_CLASS(Dir);

// Return true if either dimension is negative.
bool invalid(const Point &p);

// Distance between hexes, 1 step per tile.
Sint16 hexDist(const Point &h1, const Point &h2);

// Return the hex adjancent to the source hex in the given direction.  No
// bounds checking.
Point adjacent(const Point &hSrc, Dir d);

// Given a list of hexes, return the index of the hex closest to the target.
int findClosest(const Point &hTarget, const std::vector<Point> &hexes);

#endif
