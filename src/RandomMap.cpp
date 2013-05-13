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

#include "Pathfinder.h"
#include "algo.h"
#include "terrain.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <queue>
#include <random>
#include <tuple>

#include <iostream> // XXX

namespace {
    std::vector<SdlSurface> tiles;
    std::vector<SdlSurface> edges;
    std::vector<SdlSurface> grassObstacles;
    std::vector<SdlSurface> dirtObstacles;
    std::vector<SdlSurface> sandObstacles;
    std::vector<SdlSurface> waterObstacles;
    std::vector<SdlSurface> swampObstacles;
    std::vector<SdlSurface> snowObstacles;

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
        if (grassObstacles.empty()) {
            grassObstacles.emplace_back(sdlLoadImage("../img/grass-trees-1.png"));
            grassObstacles.emplace_back(sdlLoadImage("../img/grass-trees-2.png"));
            grassObstacles.emplace_back(sdlLoadImage("../img/grass-trees-3.png"));
        }
        if (dirtObstacles.empty()) {
            dirtObstacles.emplace_back(sdlLoadImage("../img/dirt-trees-1.png"));
            dirtObstacles.emplace_back(sdlLoadImage("../img/dirt-trees-2.png"));
            dirtObstacles.emplace_back(sdlLoadImage("../img/dirt-trees-3.png"));
        }
        if (sandObstacles.empty()) {
            sandObstacles.emplace_back(sdlLoadImage("../img/desert-plants-1.png"));
            sandObstacles.emplace_back(sdlLoadImage("../img/desert-plants-2.png"));
            sandObstacles.emplace_back(sdlLoadImage("../img/desert-plants-3.png"));
            sandObstacles.emplace_back(sdlLoadImage("../img/desert-plants-4.png"));
        }
        if (waterObstacles.empty()) {
            waterObstacles.emplace_back(sdlLoadImage("../img/water-reef-1.png"));
            waterObstacles.emplace_back(sdlLoadImage("../img/water-reef-2.png"));
            waterObstacles.emplace_back(sdlLoadImage("../img/water-reef-3.png"));
        }
        if (swampObstacles.empty()) {
            swampObstacles.emplace_back(sdlLoadImage("../img/swamp-mushrooms-1.png"));
            swampObstacles.emplace_back(sdlLoadImage("../img/swamp-mushrooms-2.png"));
            swampObstacles.emplace_back(sdlLoadImage("../img/swamp-mushrooms-3.png"));
        }
        if (snowObstacles.empty()) {
            // TODO: all of these have harsh edges to them.  Needs to be more
            // desert-like.
            snowObstacles.emplace_back(sdlLoadImage("../img/snow-trees-1.png"));
            snowObstacles.emplace_back(sdlLoadImage("../img/snow-trees-2.png"));
            snowObstacles.emplace_back(sdlLoadImage("../img/snow-trees-3.png"));
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

    SdlSurface getObstacle(int terrain)
    {
        std::vector<SdlSurface> *choices = 0;
        switch (terrain) {
            case GRASS:
                choices = &grassObstacles;
                break;
            case DIRT:
                choices = &dirtObstacles;
                break;
            case SAND:
                choices = &sandObstacles;
                break;
            case WATER:
                choices = &waterObstacles;
                break;
            case SWAMP:
                choices = &swampObstacles;
                break;
            case SNOW:
            default:
                choices = &snowObstacles;
                break;
        }

        std::uniform_int_distribution<size_t> dist(0, choices->size() - 1);
        auto i = dist(randomGenerator());
        return (*choices)[i];
    }
}

RandomMap::RandomMap(Sint16 hWidth, Sint16 hHeight, const SDL_Rect &pDisplayArea)
    : mgrid_(hWidth, hHeight),
    pWidth_(pHexSize * 3 / 4 * hWidth + pHexSize / 4),
    pHeight_(pHexSize * hHeight + pHexSize / 2),
    numRegions_(18),
    regions_(mgrid_.size(), -1),
    centers_(),
    regionGraph_(numRegions_),
    regionGraphWalk_(numRegions_),
    tgrid_(hWidth + 2, hHeight + 2),
    terrain_(tgrid_.size()),
    tObst_(tgrid_.size(), 0),
    tObstImg_(tgrid_.size()),
    pDisplayArea_(pDisplayArea),
    mMaxX_(pWidth_ - pDisplayArea_.w),
    mMaxY_(pHeight_ - pDisplayArea_.h),
    px_(0),
    py_(0)
{
    assert(hWidth > 1);

    loadTiles();
    generateRegions();
    generateObstacles();
    makeWalkable();
    buildRegionGraph();
    assignTerrain();
    setObstacleImages();

    // XXX
    auto path = getRegionPath(0, 8);
    copy(std::begin(path), std::end(path), std::ostream_iterator<int>(std::cout, ", "));
    std::cout << '\n';
}

Sint16 RandomMap::pWidth() const
{
    return pWidth_;
}

Sint16 RandomMap::pHeight() const
{
    return pHeight_;
}

const SDL_Rect & RandomMap::getDisplayArea() const
{
    return pDisplayArea_;
}

Point RandomMap::maxPixel() const
{
    return {mMaxX_, mMaxY_};
}

void RandomMap::draw(Sint16 mpx, Sint16 mpy)
{
    assert(mpx >= 0 && mpx <= mMaxX_ && mpy >= 0 && mpy <= mMaxY_);

    px_ = mpx;
    py_ = mpy;

    auto nwHex = getHexAtS(rectCorner(pDisplayArea_, Dir::NW));
    auto seHex = getHexAtS(rectCorner(pDisplayArea_, Dir::SE));
    assert(nwHex != hInvalid);
    assert(seHex != hInvalid);

    // Overdraw enough to cover the edges of the screen.  Stay within the
    // terrain grid.
    nwHex.first = std::max(nwHex.first - 1, -1);
    nwHex.second = std::max(nwHex.second - 1, -1);
    seHex.first = std::min<Sint16>(seHex.first + 1, mgrid_.width());
    seHex.second = std::min<Sint16>(seHex.second + 1, mgrid_.height());

    // TODO: RAII this, kinda like ScopeGuard11.  I expect it to be common.
    SDL_Rect temp;
    SDL_GetClipRect(screen, &temp);
    SDL_SetClipRect(screen, &pDisplayArea_);

    // TODO: fix it so we can call loadTiles() here again.
    sdlClear(pDisplayArea_);
    for (Sint16 hx = nwHex.first; hx <= seHex.first; ++hx) {
        for (Sint16 hy = nwHex.second; hy <= seHex.second; ++hy) {
            drawTile(hx, hy);
        }
    }
    for (Sint16 hx = nwHex.first; hx <= seHex.first; ++hx) {
        for (Sint16 hy = nwHex.second; hy <= seHex.second; ++hy) {
            drawObstacle(hx, hy);
        }
    }
    SDL_SetClipRect(screen, &temp);
}

Point RandomMap::mDrawnAt() const
{
    return {px_, py_};
}

Point RandomMap::getHexAtS(Sint16 spx, Sint16 spy) const
{
    return getHexAtS({spx, spy});
}

Point RandomMap::getHexAtS(const Point &sp) const
{
    if (!insideRect(sp.first, sp.second, pDisplayArea_)) {
        return hInvalid;
    }

    return getHexAtM(mPixel(sp));
}

// source: Battle for Wesnoth, pixel_position_to_hex() in display.cpp.
Point RandomMap::getHexAtM(Sint16 mpx, Sint16 mpy) const
{
    assert(mpx >= 0 && mpx < pWidth_ && mpy >= 0 && mpy < pHeight_);

    // tilingWidth
    // |   |
    //  _     _
    // / \_    tilingHeight
    // \_/ \  _
    //   \_/
    const Sint16 tilingWidth = pHexSize * 3 / 2;
    const Sint16 tilingHeight = pHexSize;

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

Point RandomMap::getHexAtM(const Point &mp) const
{
    return getHexAtM(mp.first, mp.second);
}

Point RandomMap::sPixelFromHex(Sint16 hx, Sint16 hy) const
{
    Sint16 mpx = hx * pHexSize * 0.75;
    Sint16 mpy = (hy + 0.5 * abs(hx % 2)) * pHexSize;
    return sPixel(mpx, mpy);
}

Point RandomMap::sPixelFromHex(const Point &hex) const
{
    return sPixelFromHex(hex.first, hex.second);
}

int RandomMap::getTerrainAt(Sint16 mpx, Sint16 mpy) const
{
    Point mHex = getHexAtM(mpx, mpy);
    return terrain_[tIndex(mHex)];
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
            if (rNeighbor == reg) continue;

            // If an adjacent hex is in a different region and we haven't
            // already recorded that region as a neighbor, save it.
            if (!contains(regionGraph_[reg], rNeighbor)) {
                regionGraph_[reg].push_back(rNeighbor);
            }

            // If both this hex and an adjacent hex are clear of obstacles,
            // then there is a walkable path between the two regions.
            if (tObst_[tIndex(i)] == 0 && tObst_[tIndex(an)] == 0 &&
                !contains(regionGraphWalk_[reg], rNeighbor)) {
                regionGraphWalk_[reg].push_back(rNeighbor);
            }
        }
    }

    // XXX
    for (auto i = 0u; i < regionGraphWalk_.size(); ++i) {
        std::cout << i << ": ";
        copy(std::begin(regionGraphWalk_[i]), std::end(regionGraphWalk_[i]),
             std::ostream_iterator<int>(std::cout, ", "));
        std::cout << "\n";
    }
}

void RandomMap::generateObstacles()
{
    std::uniform_real_distribution<> dist(0, 1);
    std::vector<double> obstChance;

    // Assign random values to each hex.
    generate_n(std::back_inserter(obstChance), mgrid_.size(),
               [&] { return dist(randomGenerator()); });

    // Relaxation step - replace each hex with the average of its neighbors.
    for (auto i = 0u; i < obstChance.size(); ++i) {
        double sum = 0.0;
        auto neighbors = mgrid_.aryNeighbors(i);
        for (auto n : neighbors) {
            sum += obstChance[n];
        }

        // Any hex above the threshold gets an obstacle.
        if (sum / neighbors.size() > 0.58) {  // TODO: make this configurable?
            tObst_[tIndex(i)] = 1;
        }
    }
}

void RandomMap::assignTerrain()
{
    auto rTerrain = graphTerrain(regionGraph_);

    // Assign the terrain for the main grid.
    for (auto i = 0u; i < regions_.size(); ++i) {
        auto tIdx = tIndex(i);
        terrain_[tIdx] = rTerrain[regions_[i]];
    }

    // Corners of the terrain grid mirror those of the main grid.
    auto nw = tgrid_.aryCorner(Dir::NW);
    auto nwMirror = tIndex(mgrid_.aryCorner(Dir::NW));
    terrain_[nw] = terrain_[nwMirror];
    tObst_[nw] = tObst_[nwMirror];
    auto ne = tgrid_.aryCorner(Dir::NE);
    auto neMirror = tIndex(mgrid_.aryCorner(Dir::NE));
    terrain_[ne] = terrain_[neMirror];
    tObst_[ne] = tObst_[neMirror];
    auto se = tgrid_.aryCorner(Dir::SE);
    auto seMirror = tIndex(mgrid_.aryCorner(Dir::SE));
    terrain_[se] = terrain_[seMirror];
    tObst_[se] = tObst_[seMirror];
    auto sw = tgrid_.aryCorner(Dir::SW);
    auto swMirror = tIndex(mgrid_.aryCorner(Dir::SW));
    terrain_[sw] = terrain_[swMirror];
    tObst_[sw] = tObst_[swMirror];

    // Hexes along the top and bottom edges mirror those directly below and
    // above, respectively.
    for (Sint16 hx = 0; hx < mgrid_.width(); ++hx) {
        Point top = {hx, -1};
        auto topMirror = adjacent(top, Dir::S);
        auto topIdx = tIndex(top);
        terrain_[topIdx] = terrain_[tIndex(topMirror)];
        tObst_[topIdx] = tObst_[tIndex(topMirror)];

        Point bottom = {hx, mgrid_.height()};
        auto bottomMirror = adjacent(bottom, Dir::N);
        auto botIdx = tIndex(bottom);
        terrain_[botIdx] = terrain_[tIndex(bottomMirror)];
        tObst_[botIdx] = tObst_[tIndex(bottomMirror)];
    }
    // Hexes along the left and right edges mirror their NE and SW neighbors,
    // respectively.
    for (Sint16 hy = 0; hy < mgrid_.height(); ++hy) {
        Point left = {-1, hy};
        auto leftMirror = adjacent(left, Dir::NE);
        auto leftIdx = tIndex(left);
        terrain_[leftIdx] = terrain_[tIndex(leftMirror)];
        tObst_[leftIdx] = tObst_[tIndex(leftMirror)];

        Point right = {mgrid_.width(), hy};
        auto rightMirror = adjacent(right, Dir::SW);
        auto rightIdx = tIndex(right);
        terrain_[rightIdx] = terrain_[tIndex(rightMirror)];
        tObst_[rightIdx] = tObst_[tIndex(rightMirror)];
    }
}

void RandomMap::setObstacleImages()
{
    for (auto i = 0u; i < tObstImg_.size(); ++i) {
        if (tObst_[i] == 0) continue;

        Obstacle &o = tObstImg_[i];
        o.img = getObstacle(terrain_[i]);
        o.pxOffset = pHexSize - o.img->w;
        o.pyOffset = pHexSize - o.img->h;

        // Shift the graphics a tiny bit for a less gridded look.
        std::uniform_int_distribution<size_t> dist(-3, 3);
        o.pxOffset += dist(randomGenerator());
        o.pyOffset += dist(randomGenerator());
    }
}

void RandomMap::drawTile(Sint16 hx, Sint16 hy)
{
    Sint16 spx = 0;
    Sint16 spy = 0;
    std::tie(spx, spy) = sPixelFromHex(hx, hy);
    auto tIdx = tIndex(hx, hy);
    auto terrainType = terrain_[tIdx];

    sdlBlit(tiles[terrainType], spx, spy);

    // Draw edge transitions for each neighboring tile.
    for (auto dir : Dir()) {
        auto neighbor = adjacent({hx, hy}, dir);
        auto neighborIndex = tIndex(neighbor);
        if (neighborIndex == -1) continue;
        auto edgeType = getEdge(terrainType, terrain_[neighborIndex]);
        if (edgeType >= 0) {
            int e = edgeType * 6 + int(dir);
            sdlBlit(edges[e], spx, spy);
        }
    }
}

void RandomMap::drawObstacle(Sint16 hx, Sint16 hy)
{
    Sint16 spx = 0;
    Sint16 spy = 0;
    std::tie(spx, spy) = sPixelFromHex(hx, hy);
    auto tIdx = tIndex(hx, hy);

    if (tObst_[tIdx]) {
        sdlBlit(tObstImg_[tIdx].img, spx + tObstImg_[tIdx].pxOffset,
                spy + tObstImg_[tIdx].pyOffset);
    }
}

void RandomMap::makeWalkable()
{
    // TODO: this fails if there is only one walkable hex in a region.

    // Build a list of walkable hexes in each region.
    std::vector<std::vector<int>> walkByReg(numRegions_);
    for (auto i = 0u; i < regions_.size(); ++i) {
        if (walkable(i)) {
            auto r = regions_[i];
            walkByReg[r].push_back(i);
        }
    }

    // Keep track of all hexes we can reach through multiple function calls.
    std::vector<char> visited(regions_.size(), 0);
    
    for (auto i = 0; i < numRegions_; ++i) {
        if (!walkByReg[i].empty()) {
            makeRegionWalkable(walkByReg[i], visited);
        }
    }
}

void RandomMap::makeRegionWalkable(std::vector<int> &hexes,
                                   std::vector<char> &visited)
{
    // Breadth-first search from the first walkable hex in each region.  If
    // the regions are open, we should reach every hex this way.
    std::queue<int> q;
    q.push(hexes[0]);
    while (!q.empty()) {
        auto hex = q.front();
        visited[hex] = 1;
        for (auto n : aryNeighborsSameRegion(hex)) {
            if (walkable(n) && visited[n] == 0) {
                q.push(n);
            }
        }
        q.pop();
    }

    // Were any hexes not visited by the search?
    auto notFound = find_if(std::begin(hexes), std::end(hexes),
                            [&] (int hex) { return visited[hex] == 0; });
    if (notFound == std::end(hexes)) return;

    // Starting from a hex we couldn't reach, find a path to the nearest
    // walkable hex already visited in this region.
    Pathfinder pf;
    pf.setNeighbors([this] (int node) { return aryNeighborsSameRegion(node); });
    pf.setGoal([this, &visited] (int node) {
        return visited[node] == 1 && walkable(node);
    });
    auto path = pf.getPathFrom(*notFound);

    // Clear this path of obstacles.
    for (auto n : path) {
        tObst_[tIndex(n)] = 0;
        visited[n] = 1;
    }
    
    // Start over with the hex that wasn't found last time.
    iter_swap(std::begin(hexes), notFound);
    makeRegionWalkable(hexes, visited);
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
    return mPixel(sp.first, sp.second);
}

Point RandomMap::mPixel(Sint16 spx, Sint16 spy) const
{
    Sint16 mpx = px_ + spx - pDisplayArea_.x;
    Sint16 mpy = py_ + spy - pDisplayArea_.y;
    return {mpx, mpy};
}

Point RandomMap::sPixel(const Point &mp) const
{
    return sPixel(mp.first, mp.second);
}

Point RandomMap::sPixel(Sint16 mpx, Sint16 mpy) const
{
    Sint16 spx = mpx - px_ + pDisplayArea_.x;
    Sint16 spy = mpy - py_ + pDisplayArea_.y;
    return {spx, spy};
}

std::vector<int> RandomMap::aryNeighborsSameRegion(int aIndex) const
{
    assert(aIndex >= 0 && aIndex < mgrid_.size());

    auto myReg = regions_[aIndex];
    std::vector<int> nbrs;

    for (auto n : mgrid_.aryNeighbors(aIndex)) {
        if (regions_[n] == myReg) {
            nbrs.push_back(n);
        }
    }

    return nbrs;
}

bool RandomMap::walkable(int mIndex) const
{
    return tObst_[tIndex(mIndex)] == 0;
}

std::vector<int> RandomMap::getRegionPath(int rBegin, int rEnd) const
{
    Pathfinder pf;
    pf.setNeighbors([this] (int n) { return regionGraphWalk_[n]; });
    pf.setGoal(rEnd);
    return pf.getPathFrom(rBegin);
}
