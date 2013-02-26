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
#include "hex_utils.h"
#include <sstream>

bool operator==(const Point &lhs, const Point &rhs)
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

bool operator!=(const Point &lhs, const Point &rhs)
{
    return !(lhs == rhs);
}

Point operator+(const Point &lhs, const Point &rhs)
{
    return {lhs.first + rhs.first, lhs.second + rhs.second};
}

std::string str(const Point &p)
{
    std::ostringstream strm;
    strm << '(' << p.first << ',' << p.second << ')';
    return strm.str();
}

// source: Battle for Wesnoth, distance_between() in map_location.cpp.
Sint16 hexDist(const Point &h1, const Point &h2)
{
    if (h1 == hInvalid || h2 == hInvalid) {
        return Sint16_max;
    }

    Sint16 dx = abs(h1.first - h2.first);
    Sint16 dy = abs(h1.second - h2.second);

    // Since the x-axis of the hex grid is staggered, we need to add a step in
    // certain cases.
    Sint16 vPenalty = 0;
    if ((h1.second < h2.second && h1.first % 2 == 0 && h2.first % 2 == 1) ||
        (h1.second > h2.second && h1.first % 2 == 1 && h2.first % 2 == 0)) {
        vPenalty = 1;
    }

    return std::max<Sint16>(dx, dy + vPenalty + dx / 2);
}

Point adjacent(const Point &hSrc, Dir d)
{
    auto hx = hSrc.first;

    switch (d) {
        case Dir::N:
            return hSrc + Point{0, -1};
        case Dir::NE:
            if (hx % 2 == 0) {
                return hSrc + Point{1, -1};
            }
            else {
                return hSrc + Point{1, 0};
            }
        case Dir::SE:
            if (hx % 2 == 0) {
                return hSrc + Point{1, 0};
            }
            else {
                return hSrc + Point{1, 1};
            }
        case Dir::S:
            return hSrc + Point{0, 1};
        case Dir::SW:
            if (hx % 2 == 0) {
                return hSrc + Point{-1, 0};
            }
            else {
                return hSrc + Point{-1, 1};
            }
        case Dir::NW:
            if (hx % 2 == 0) {
                return hSrc + Point{-1, -1};
            }
            else {
                return hSrc + Point{-1, 0};
            }
        default:
            return hInvalid;
    }
}

int findClosest(const Point &hTarget, const std::vector<Point> &hexes)
{
    int closest = -1;
    int size = static_cast<int>(hexes.size());
    Sint16 bestSoFar = Sint16_max;

    for (int i = 0; i < size; ++i) {
        Sint16 dist = hexDist(hTarget, hexes[i]);
        if (dist < bestSoFar) {
            closest = i;
            bestSoFar = dist;
        }
    }

    return closest;
}
