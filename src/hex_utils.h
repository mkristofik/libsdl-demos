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
#include <iosfwd>
#include <vector>
#include <utility>

using Point = std::pair<Sint16, Sint16>;
const Point hInvalid = {-1, -1};

enum class Dir {N, NE, SE, S, SW, NW, _last, _first = N};
ITERABLE_ENUM_CLASS(Dir);

bool operator==(const Point &lhs, const Point &rhs);
bool operator!=(const Point &lhs, const Point &rhs);
std::ostream & operator<<(std::ostream &os, const Point &p);
std::ostream & operator<<(std::ostream &&os, const Point &p);

// Distance between hexes, 1 step per tile.
Sint16 hexDist(const Point &h1, const Point &h2);

// Logical view of a hex grid.  Designed to be cheap to create, copy, etc.
class HexGrid
{
public:
    HexGrid(Sint16 width, Sint16 height);

    // Two ways to view a hex map: a 2D map of (x,y) coordinates, and a
    // contiguous array.  These functions convert between the two
    // representations.
    Point hexFromAry(int aIndex);
    int aryFromHex(Sint16 hx, Sint16 hy);
    int aryFromHex(const Point &hex);

    // Generate a random hex in the range [(0,0), (width-1,height-1)].
    Point hexRandom();

    // Return the neighbor hex in a given direction from the source hex.  Return
    // -1 if the neighbor hex would be off the map.
    int aryGetNeighbor(int aSrc, Dir d);

    // Compute all neighbors of a given hex.  Might have fewer than 6.
    std::vector<int> aryNeighbors(int aIndex);
    std::vector<Point> hexNeighbors(const Point &hex);

private:
    Sint16 width_;
    Sint16 height_;
    Sint16 size_;
};

#endif
