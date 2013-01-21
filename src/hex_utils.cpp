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
#include <algorithm>
#include <ctime>
#include <limits>
#include <ostream>
#include <random>

bool operator==(const Point &lhs, const Point &rhs)
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

bool operator!=(const Point &lhs, const Point &rhs)
{
    return !(lhs == rhs);
}

std::ostream & operator<<(std::ostream &os, const Point &p)
{
    os << '(' << p.first << ',' << p.second << ')';
    return os;
}

std::ostream & operator<<(std::ostream &&os, const Point &p)
{
    os << p;
    return os;
}

// source: Battle for Wesnoth, distance_between() in map_location.cpp.
Sint16 hexDist(const Point &h1, const Point &h2)
{
    if (h1 == hInvalid || h2 == hInvalid) {
        return std::numeric_limits<Sint16>::max();
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

HexGrid::HexGrid(Sint16 width, Sint16 height)
    : width_(width),
    height_(height),
    size_(width_ * height_)
{
}

Point HexGrid::hexFromAry(int aIndex)
{
    return {aIndex % width_, aIndex / width_};
}

int HexGrid::aryFromHex(Sint16 hx, Sint16 hy)
{
    return hy * width_ + hx;
}

int HexGrid::aryFromHex(const Point &hex)
{
    return aryFromHex(hex.first, hex.second);
}

Point HexGrid::hexRandom()
{
    static std::minstd_rand gen(static_cast<unsigned int>(std::time(nullptr)));
    static std::uniform_int_distribution<Sint16> dist(0, size_ - 1);
    Sint16 aRand = dist(gen);
    return hexFromAry(aRand);
}

int HexGrid::aryGetNeighbor(int aSrc, Dir d)
{
    // North, below the top row.
    if (d == Dir::N && aSrc >= width_) {
        return aSrc - width_;
    }
    // Northeast, not in the right column.
    else if (d == Dir::NE && (aSrc % width_ < width_ - 1)) {
        if (aSrc % 2 == 0) {
            if (aSrc >= width_) {
                return aSrc - width_ + 1;
            }
        }
        else {
            return aSrc + 1;
        }
    }
    // Southeast, not in the right column.
    else if (d == Dir::SE && (aSrc % width_ < width_ - 1)) {
        if (aSrc % 2 == 0) {
            return aSrc + 1;
        }
        else {
            if (aSrc < size_ - width_) {
                return aSrc + width_ + 1;
            }
        }
    }
    // South, above the bottom row.
    else if (d == Dir::S && (aSrc < size_ - width_)) {
        return aSrc + width_;
    }
    // Southwest, not in the left column.
    else if (d == Dir::SW && (aSrc % width_ > 0)) {
        if (aSrc % 2 == 0) {
            return aSrc - 1;
        }
        else {
            if (aSrc < size_ - width_) {
                return aSrc + width_ - 1;
            }
        }
    }
    // Northwest, not in the left column.
    else if (d == Dir::NW && (aSrc % width_ > 0)) {
        if (aSrc % 2 == 0) {
            if (aSrc >= width_) {
                return aSrc - width_ - 1;
            }
        }
        else {
            return aSrc - 1;
        }
    }

    return -1;
}

std::vector<int> HexGrid::aryNeighbors(int aIndex)
{
    std::vector<int> av;

    for (auto d : Dir()) {
        auto aNeighbor = aryGetNeighbor(aIndex, d);
        if (aNeighbor != -1) {
            av.push_back(aNeighbor);
        }
    }

    return av;
}

std::vector<Point> HexGrid::hexNeighbors(const Point &hex)
{
    std::vector<Point> hv;

    for (auto aNeighbor : aryNeighbors(aryFromHex(hex))) {
        hv.push_back(hexFromAry(aNeighbor));
    }

    return hv;
}
