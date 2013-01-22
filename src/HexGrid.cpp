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
#include "HexGrid.h"
#include <ctime>
#include <limits>
#include <random>

HexGrid::HexGrid(Sint16 width, Sint16 height)
    : width_(width),
    height_(height),
    size_(width_ * height_)
{
}

Point HexGrid::hexFromAry(int aIndex) const
{
    if (aIndex < 0 || aIndex >= size_) {
        return hInvalid;
    }

    return {aIndex % width_, aIndex / width_};
}

int HexGrid::aryFromHex(Sint16 hx, Sint16 hy) const
{
    return aryFromHex({hx, hy});
}

int HexGrid::aryFromHex(const Point &hex) const
{
    if (offGrid(hex)) {
        return -1;
    }

    return hex.second * width_ + hex.first;
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
