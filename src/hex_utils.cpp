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
#include <random>
#include <sstream>

bool operator==(const Point &lhs, const Point &rhs)
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

bool operator!=(const Point &lhs, const Point &rhs)
{
    return !(lhs == rhs);
}

std::string str(const Point &p)
{
    std::ostringstream strm;
    strm << '(' << p.first << ',' << p.second << ')';
    return strm.str();
}

bool invalid(const Point &p)
{
    return p.first < 0 || p.second < 0;
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

Point adjacent(const Point &hSrc, Dir d)
{
    auto hx = hSrc.first;
    auto hy = hSrc.second;

    switch (d) {
        case Dir::N:
            return {hx, hy - 1};
        case Dir::NE:
            if (hx % 2 == 0) {
                return {hx + 1, hy - 1};
            }
            else {
                return {hx + 1, hy};
            }
        case Dir::SE:
            if (hx % 2 == 0) {
                return {hx + 1, hy};
            }
            else {
                return {hx + 1, hy + 1};
            }
        case Dir::S:
            return {hx, hy + 1};
        case Dir::SW:
            if (hx % 2 == 0) {
                return {hx - 1, hy};
            }
            else {
                return {hx - 1, hy + 1};
            }
        case Dir::NW:
            if (hx % 2 == 0) {
                return {hx - 1, hy - 1};
            }
            else {
                return {hx - 1, hy};
            }
        default:
            return hInvalid;
    }
}

HexGrid::HexGrid(Sint16 width, Sint16 height)
    : width_(width),
    height_(height),
    size_(width_ * height_)
{
}

Point HexGrid::hexFromAry(int aIndex) const
{
    return {aIndex % width_, aIndex / width_};
}

int HexGrid::aryFromHex(Sint16 hx, Sint16 hy) const
{
    return hy * width_ + hx;
}

int HexGrid::aryFromHex(const Point &hex) const
{
    return aryFromHex(hex.first, hex.second);
}

Point HexGrid::hexRandom() const
{
    static std::minstd_rand gen(static_cast<unsigned int>(std::time(nullptr)));
    static std::uniform_int_distribution<Sint16> dist(0, size_ - 1);
    Sint16 aRand = dist(gen);
    return hexFromAry(aRand);
}

int HexGrid::aryGetNeighbor(int aSrc, Dir d) const
{
    auto neighbor = adjacent(hexFromAry(aSrc), d);
    if (offGrid(neighbor)) {
        return -1;
    }

    return aryFromHex(neighbor);
}

Point HexGrid::hexGetNeighbor(const Point &hSrc, Dir d) const
{
    auto neighbor = adjacent(hSrc, d);
    if (offGrid(neighbor)) {
        return hInvalid;
    }

    return neighbor;
}

std::vector<int> HexGrid::aryNeighbors(int aIndex) const
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

std::vector<Point> HexGrid::hexNeighbors(const Point &hex) const
{
    std::vector<Point> hv;

    for (auto aNeighbor : aryNeighbors(aryFromHex(hex))) {
        hv.push_back(hexFromAry(aNeighbor));
    }

    return hv;
}

bool HexGrid::offGrid(const Point &hex) const
{
    return invalid(hex) ||
           hex.first >= width_ ||
           hex.second >= height_;
}
