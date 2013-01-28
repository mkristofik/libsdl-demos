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

namespace {
    std::vector<SdlSurface> tiles;
    std::vector<SdlSurface> edges;

    void loadTiles()
    {
        assert(SDL_WasInit(SDL_INIT_VIDEO));

        // Load the tiles in the same order as Terrain enum.
        if (tiles.empty()) {
            tiles.push_back(sdlLoadImage("../img/grass.png"));
            tiles.push_back(sdlLoadImage("../img/dirt.png"));
            tiles.push_back(sdlLoadImage("../img/desert.png"));
            tiles.push_back(sdlLoadImage("../img/water.png"));
            tiles.push_back(sdlLoadImage("../img/swamp.png"));
            tiles.push_back(sdlLoadImage("../img/snow.png"));
        }
        if (edges.empty()) {
            edges.push_back(sdlLoadImage("../img/grass-n.png"));
            edges.push_back(sdlLoadImage("../img/grass-ne.png"));
            edges.push_back(sdlLoadImage("../img/grass-se.png"));
            edges.push_back(sdlLoadImage("../img/grass-s.png"));
            edges.push_back(sdlLoadImage("../img/grass-sw.png"));
            edges.push_back(sdlLoadImage("../img/grass-nw.png"));
            edges.push_back(sdlLoadImage("../img/dirt-n.png"));
            edges.push_back(sdlLoadImage("../img/dirt-ne.png"));
            edges.push_back(sdlLoadImage("../img/dirt-se.png"));
            edges.push_back(sdlLoadImage("../img/dirt-s.png"));
            edges.push_back(sdlLoadImage("../img/dirt-sw.png"));
            edges.push_back(sdlLoadImage("../img/dirt-nw.png"));
            edges.push_back(sdlLoadImage("../img/beach-n.png"));
            edges.push_back(sdlLoadImage("../img/beach-ne.png"));
            edges.push_back(sdlLoadImage("../img/beach-se.png"));
            edges.push_back(sdlLoadImage("../img/beach-s.png"));
            edges.push_back(sdlLoadImage("../img/beach-sw.png"));
            edges.push_back(sdlLoadImage("../img/beach-nw.png"));
        }
    }
}

RandomMap::RandomMap(Sint16 hWidth, Sint16 hHeight)
    : mgrid_(hWidth, hHeight),
    numRegions_(18),
    regions_(mgrid_.size(), -1),
    centers_(),
    regionGraph_(numRegions_),
    tgrid_(hWidth + 2, hHeight + 2),
    terrain_(tgrid_.size())
{
    loadTiles();
    generateRegions();
    buildRegionGraph();
    assignTerrain();
}

void RandomMap::draw()
{
    for (Sint16 hx = -1; hx <= mgrid_.width(); ++hx) {
        for (Sint16 hy = -1; hy <= mgrid_.height(); ++hy) {
            drawTile(hx, hy);
        }
    }
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
        auto neighbor = adjacent(Point{hx, hy}, dir);
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
    return tIndex(Point{hx, hy});
}
