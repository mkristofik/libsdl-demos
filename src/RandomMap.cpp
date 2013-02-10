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
#include "RandomMap.h"
#include "algo.h"
#include "sdl_helper.h"
#include "terrain.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <tuple>

namespace {
    std::vector<SdlSurface> tiles;
    std::vector<SdlSurface> edges;

    void loadTiles()
    {
        assert(SDL_WasInit(SDL_INIT_VIDEO));

        // Load the tiles in the same order as Terrain enum.
        if (tiles.empty()) {
            tiles.emplace_back(sdlLoadImage("../img/grass.png"));
            tiles.emplace_back(sdlLoadImage("../img/dirt.png"));
            tiles.emplace_back(sdlLoadImage("../img/desert.png"));
            tiles.emplace_back(sdlLoadImage("../img/water.png"));
            tiles.emplace_back(sdlLoadImage("../img/swamp.png"));
            tiles.emplace_back(sdlLoadImage("../img/snow.png"));
        }
        if (edges.empty()) {
            edges.emplace_back(sdlLoadImage("../img/grass-n.png"));
            edges.emplace_back(sdlLoadImage("../img/grass-ne.png"));
            edges.emplace_back(sdlLoadImage("../img/grass-se.png"));
            edges.emplace_back(sdlLoadImage("../img/grass-s.png"));
            edges.emplace_back(sdlLoadImage("../img/grass-sw.png"));
            edges.emplace_back(sdlLoadImage("../img/grass-nw.png"));
            edges.emplace_back(sdlLoadImage("../img/dirt-n.png"));
            edges.emplace_back(sdlLoadImage("../img/dirt-ne.png"));
            edges.emplace_back(sdlLoadImage("../img/dirt-se.png"));
            edges.emplace_back(sdlLoadImage("../img/dirt-s.png"));
            edges.emplace_back(sdlLoadImage("../img/dirt-sw.png"));
            edges.emplace_back(sdlLoadImage("../img/dirt-nw.png"));
            edges.emplace_back(sdlLoadImage("../img/beach-n.png"));
            edges.emplace_back(sdlLoadImage("../img/beach-ne.png"));
            edges.emplace_back(sdlLoadImage("../img/beach-se.png"));
            edges.emplace_back(sdlLoadImage("../img/beach-s.png"));
            edges.emplace_back(sdlLoadImage("../img/beach-sw.png"));
            edges.emplace_back(sdlLoadImage("../img/beach-nw.png"));
        }
    }

    Point rectCorner(const SDL_Rect &rect, Dir d)
    {
        switch (d) {
            case Dir::NW:
                return {rect.x, rect.y};
            case Dir::NE:
                return {rect.x + rect.w - 1, rect.y};
            case Dir::SE:
                return {rect.x + rect.w - 1, rect.y + rect.h - 1};
            case Dir::SW:
                return {rect.x, rect.y + rect.h - 1};
            default:
                assert(false);
        }
    }
}

RandomMap::RandomMap(Sint16 hWidth, Sint16 hHeight, SDL_Rect pDisplayArea)
    : mgrid_(hWidth, hHeight),
    numRegions_(18),
    regions_(mgrid_.size(), -1),
    centers_(),
    regionGraph_(numRegions_),
    tgrid_(hWidth + 2, hHeight + 2),
    terrain_(tgrid_.size()),
    pDisplayArea_(pDisplayArea),
    px_(0),
    py_(0)
{
    generateRegions();
    buildRegionGraph();
    assignTerrain();
}

void RandomMap::draw(Sint16 mpx, Sint16 mpy)
{
    // TODO: verify we won't draw past the map boundary.
    px_ = mpx;
    py_ = mpy;

    auto nwHex = getHexAt(rectCorner(pDisplayArea_, Dir::NW));
    auto seHex = getHexAt(rectCorner(pDisplayArea_, Dir::SE));
    assert(nwHex != hInvalid);
    assert(seHex != hInvalid);

    loadTiles();
    for (Sint16 hx = nwHex.first; hx <= seHex.first; ++hx) {
        for (Sint16 hy = nwHex.second; hy <= seHex.second; ++hy) {
            drawTile(hx, hy);
        }
    }
}

// source: Battle for Wesnoth, pixel_position_to_hex() in display.cpp.
Point RandomMap::getHexAt(Sint16 spx, Sint16 spy) const
{
    return getHexAt({spx, spy});
}

Point RandomMap::getHexAt(const Point &sp) const
{
    if (!insideRect(sp, pDisplayArea_)) {
        return hInvalid;
    }

    // tilingWidth
    // |   |
    //  _     _
    // / \_    tilingHeight
    // \_/ \  _
    //   \_/
    const Sint16 tilingWidth = pHexSize * 3 / 2;
    const Sint16 tilingHeight = pHexSize;

    // Convert screen coordinates to map pixel coordinates.
    Sint16 mpx = 0;
    Sint16 mpy = 0;
    std::tie(mpx, mpy) = mPixel(sp);

    // I'm not going to pretend to know why the rest of this works.
    Sint16 hx = mpx / tilingWidth * 2;
    Sint16 xMod = mpx % tilingWidth;
    Sint16 hy = mpy / tilingHeight;
    Sint16 yMod = mpy % tilingHeight;

    if (yMod < tilingWidth / 2) {
        if ((xMod * 2 + yMod) < (pHexSize / 2)) {
            --hx;
            --hy;
        }
        else if ((xMod * 2 - yMod) < (pHexSize * 3 / 2)) {
            // do nothing
        }
        else {
            ++hx;
            --hy;
        }
    }
    else {
        if ((xMod * 2 - (yMod - pHexSize / 2)) < 0) {
            --hx;
        }
        else if ((xMod * 2 + (yMod - pHexSize / 2)) < pHexSize * 2) {
            // do nothing
        }
        else {
            ++hx;
        }
    }

    return {hx, hy};
}

void RandomMap::generateRegions()
{
    // Start with a set of random hexes.  Don't worry if there are duplicates.
    generate_n(std::back_inserter(centers_),
               numRegions_,
               [this] { return mgrid_.hexRandom(); });

    // Find the closest center to each hex on the map.  The set of hexes
    // closest to center #0 will be region 0, etc.  Repeat this several times
    // for more regular-looking regions.
    for (int i = 0; i < 4; ++i) {
        for (int aIndex = 0; aIndex < mgrid_.size(); ++aIndex) {
            regions_[aIndex] = findClosest(mgrid_.hexFromAry(aIndex), centers_);
        }
        recalcHexCenters();
    }

    // Assign each hex to its final region.
    for (int aIndex = 0; aIndex < mgrid_.size(); ++aIndex) {
        regions_[aIndex] = findClosest(mgrid_.hexFromAry(aIndex), centers_);
    }
}

void RandomMap::recalcHexCenters()
{
    std::vector<Point> hexSums(numRegions_);
    std::vector<int> numHexes(numRegions_);

    for (Sint16 hx = 0; hx < mgrid_.width(); ++hx) {
        for (Sint16 hy = 0; hy < mgrid_.height(); ++hy) {
            int region = regions_[mgrid_.aryFromHex(hx, hy)];
            assert(region >= 0 && region < numRegions_);

            auto &hs = hexSums[region];
            hs.first += hx;
            hs.second += hy;
            ++numHexes[region];
        }
    }

    for (int r = 0; r < numRegions_; ++r) {
        // The Voronoi algorithm sometimes leads to regions being "absorbed" by
        // their neighbors.  Leave the default (invalid) center hex in place
        // for an empty region.
        if (numHexes[r] > 0) {
            auto &hc = centers_[r];
            auto &hs = hexSums[r];
            hc.first = hs.first / numHexes[r];
            hc.second = hs.second / numHexes[r];
        }
    }
}

void RandomMap::buildRegionGraph()
{
    for (int i = 0; i < mgrid_.size(); ++i) {
        auto reg = regions_[i];
        assert(reg >= 0 && reg < numRegions_);

        for (const auto &an : mgrid_.aryNeighbors(i)) {
            auto rNeighbor = regions_[an];
            // If an adjacent hex is in a different region and we haven't
            // already recorded that region as a neighbor, save it.
            if (rNeighbor != reg && !contains(regionGraph_[reg], rNeighbor)) {
                regionGraph_[reg].push_back(rNeighbor);
            }
        }
    }
}

void RandomMap::assignTerrain()
{
    auto rTerrain = graphTerrain(regionGraph_);

    // Assign the terrain for the main grid.
    for (auto i = 0u; i < regions_.size(); ++i) {
        terrain_[tIndex(i)] = rTerrain[regions_[i]];
    }

    // Corners of the terrain grid mirror those of the main grid.
    auto nw = tgrid_.aryCorner(Dir::NW);
    auto nwMirror = tIndex(mgrid_.aryCorner(Dir::NW));
    terrain_[nw] = terrain_[nwMirror];
    auto ne = tgrid_.aryCorner(Dir::NE);
    auto neMirror = tIndex(mgrid_.aryCorner(Dir::NE));
    terrain_[ne] = terrain_[neMirror];
    auto se = tgrid_.aryCorner(Dir::SE);
    auto seMirror = tIndex(mgrid_.aryCorner(Dir::SE));
    terrain_[se] = terrain_[seMirror];
    auto sw = tgrid_.aryCorner(Dir::SW);
    auto swMirror = tIndex(mgrid_.aryCorner(Dir::SW));
    terrain_[sw] = terrain_[swMirror];

    // Hexes along the top and bottom edges mirror those directly below and
    // above, respectively.
    for (Sint16 hx = 0; hx < mgrid_.width(); ++hx) {
        Point top = {hx, -1};
        auto topMirror = adjacent(top, Dir::S);
        terrain_[tIndex(top)] = terrain_[tIndex(topMirror)];

        Point bottom = {hx, mgrid_.height()};
        auto bottomMirror = adjacent(bottom, Dir::N);
        terrain_[tIndex(bottom)] = terrain_[tIndex(bottomMirror)];
    }
    // Hexes along the left and right edges mirror their NE and SW neighbors,
    // respectively.
    for (Sint16 hy = 0; hy < mgrid_.height(); ++hy) {
        Point left = {-1, hy};
        auto leftMirror = adjacent(left, Dir::NE);
        terrain_[tIndex(left)] = terrain_[tIndex(leftMirror)];

        Point right = {mgrid_.width(), hy};
        auto rightMirror = adjacent(right, Dir::SW);
        terrain_[tIndex(right)] = terrain_[tIndex(rightMirror)];
    }
}

void RandomMap::drawTile(Sint16 hx, Sint16 hy)
{
    Sint16 px = hx * pHexSize * 0.75;
    Sint16 py = (hy + 0.5 * abs(hx % 2)) * pHexSize;
    auto terrainType = terrain_[tIndex(hx, hy)];
    sdlBlit(tiles[terrainType], px, py);

    // Draw edge transitions for each neighboring tile.
    for (auto dir : Dir()) {
        auto neighbor = adjacent({hx, hy}, dir);
        auto neighborIndex = tIndex(neighbor);
        if (neighborIndex == -1) continue;
        auto edgeType = getEdge(terrainType, terrain_[neighborIndex]);
        if (edgeType >= 0) {
            int e = edgeType * 6 + int(dir);
            sdlBlit(edges[e], px, py);
        }
    }
}

int RandomMap::tIndex(int mIndex) const
{
    assert(mIndex >= 0 && mIndex < mgrid_.size());
    return tIndex(mgrid_.hexFromAry(mIndex));
}

int RandomMap::tIndex(const Point &mHex) const
{
    auto tHex = mHex + Point{1, 1};
    if (tgrid_.offGrid(tHex)) {
        return -1;
    }

    return tgrid_.aryFromHex(tHex);
}

int RandomMap::tIndex(Sint16 hx, Sint16 hy) const
{
    return tIndex({hx, hy});
}

Point RandomMap::mPixel(const Point &sp) const
{
    Sint16 mpx = px_ + sp.first - pDisplayArea_.x;
    Sint16 mpy = py_ + sp.second - pDisplayArea_.y;
    return {mpx, mpy};
}
