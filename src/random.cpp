/*
    Copyright (C) 2012 by Michael Kristofik <kristo605@gmail.com>
    Part of the libsdl-demos project.
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    or at your option any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY.
 
    See the COPYING.txt file for more details.
*/
#include "SDL.h"
#include "SDL_image.h"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using SdlSurface = std::shared_ptr<SDL_Surface>;
using Point = std::pair<Sint16, Sint16>;

const Sint16 hexSize = 72;
const Sint16 mapWidth = 16;
const Sint16 mapHeight = 9;
const Sint16 mapSize = mapWidth * mapHeight;
const int numRegions = 18;
const int numTerrains = 6;
const Point hInvalid = {-1, -1};
const Sint16 maxDist = mapSize + 1;
// TODO: use h prefix for hex coordinates, p for pixel coordinates, a for array index

SDL_Surface *screen = nullptr;

SdlSurface make_surface(SDL_Surface *surf)
{
    return SdlSurface(surf, SDL_FreeSurface);
}

std::ostream & operator<<(std::ostream &os, const Point &p)
{
    os << '(' << p.first << ',' << p.second << ')';
    return os;
}

Point hexFromAry(int aIndex)
{
    return std::make_pair(aIndex % mapWidth, aIndex / mapWidth);
}

int aryFromHex(const Point &hex)
{
    return hex.second * mapWidth + hex.first;
}

Point hexRandom()
{
    static std::minstd_rand gen(static_cast<unsigned int>(std::time(nullptr)));
    static std::uniform_int_distribution<Sint16> dist(0, mapWidth * mapHeight - 1);
    Sint16 aRand = dist(gen);
    return hexFromAry(aRand);
}

// source: Battle for Wesnoth, distance_between() in map_location.cpp.
Sint16 hexDist(const Point &lhs, const Point &rhs)
{
    if (lhs == hInvalid || rhs == hInvalid) {
        return maxDist;
    }

    Sint16 dx = abs(lhs.first - rhs.first);
    Sint16 dy = abs(lhs.second - rhs.second);

    // Since the x-axis of the hex grid is staggered, we need to add a step in
    // certain cases.
    Sint16 vPenalty = 0;
    if ((lhs.second < rhs.second && lhs.first % 2 == 0 && rhs.first % 2 == 1) ||
        (lhs.second > rhs.second && lhs.first % 2 == 1 && rhs.first % 2 == 0)) {
        vPenalty = 1;
    }

    return std::max<Sint16>(dx, dy + vPenalty + dx / 2);
}

std::vector<Point> hexNeighbors(const Point &hex)
{
    std::vector<Point> hv;

    if (hex.second > 0) {
        // north
        hv.emplace_back(hex.first, hex.second - 1);
    }
    if (hex.second < mapHeight - 1) {
        // south
        hv.emplace_back(hex.first, hex.second + 1);
    }
    if (hex.first > 0) {
        if (hex.first % 2 == 0) {
            if (hex.second > 0) {
                // northwest, even column
                hv.emplace_back(hex.first - 1, hex.second - 1);
            }
            // southwest, even column
            hv.emplace_back(hex.first - 1, hex.second);
        }
        else {
            // northwest, odd column
            hv.emplace_back(hex.first - 1, hex.second);
            if (hex.second < mapHeight - 1) {
                // southwest, odd column
                hv.emplace_back(hex.first - 1, hex.second + 1);
            }
        }
    }
    if (hex.first < mapWidth - 1) {
        if (hex.first % 2 == 0) {
            if (hex.second > 0) {
                // northeast, even column
                hv.emplace_back(hex.first + 1, hex.second - 1);
            }
            // southeast, even column
            hv.emplace_back(hex.first + 1, hex.second);
        }
        else {
            // northeast, odd column
            hv.emplace_back(hex.first + 1, hex.second);
            if (hex.second < mapHeight - 1) {
                // southeast, odd column
                hv.emplace_back(hex.first + 1, hex.second + 1);
            }
        }
    }

    return hv;
}

// Same as hexNeighbors() but with array indexes instead of hex coordinates.
std::vector<int> aryNeighbors(int aIndex)
{
    std::vector<int> av;

    if (aIndex >= mapWidth) {  // below the top row
        // north
        av.push_back(aIndex - mapWidth);
    }
    if (aIndex < mapSize - mapWidth) {  // above the bottom row
        // south
        av.push_back(aIndex + mapWidth);
    }
    if (aIndex % mapWidth > 0) {  // not in the left column
        if (aIndex % 2 == 0) {
            if (aIndex >= mapWidth) {
                // northwest, even column
                av.push_back(aIndex - mapWidth - 1);
            }
            // southwest, even column
            av.push_back(aIndex - 1);
        }
        else {
            // northwest, odd column
            av.push_back(aIndex - 1);
            if (aIndex < mapSize - mapWidth) {
                // southwest, odd column
                av.push_back(aIndex + mapWidth - 1);
            }
        }
    }
    if (aIndex % mapWidth < mapWidth - 1) {  // not in the right column
        if (aIndex % 2 == 0) {
            if (aIndex >= mapWidth) {
                // northeast, even column
                av.push_back(aIndex - mapWidth + 1);
            }
            // southeast, even column
            av.push_back(aIndex + 1);
        }
        else {
            // northeast, odd column
            av.push_back(aIndex + 1);
            if (aIndex < mapSize - mapWidth) {
                // southeast, odd column
                av.push_back(aIndex + mapWidth + 1);
            }
        }
    }

    return av;
}

// Return the index of the closest center point to the given hex (x,y).
int findClosest(Sint16 x, Sint16 y, const std::vector<Point> &centers)
{
    int closest = -1;
    Sint16 bestSoFar = maxDist;

    for (int i = 0; i < numRegions; ++i) {
        Sint16 dist = hexDist(Point(x, y), centers[i]);
        if (dist < bestSoFar) {
            closest = i;
            bestSoFar = dist;
        }
    }

    return closest;
}

// Compute the centers of mass of each region.
std::vector<Point> getCenters(const std::vector<int> &regions)
{
    std::vector<Point> positionSums(numRegions);
    std::vector<int> numHexes(numRegions);
    std::vector<Point> centers(numRegions, hInvalid);

    for (Sint16 x = 0; x < mapWidth; ++x) {
        for (Sint16 y = 0; y < mapHeight; ++y) {
            int region = regions[y * mapWidth + x];
            assert(region >= 0 && region < numRegions);

            auto &p = positionSums[region];
            p.first += x;
            p.second += y;
            ++numHexes[region];
        }
    }

    for (int i = 0; i < numRegions; ++i) {
        // The voronoi algorithm sometimes leads to regions being "absorbed" by
        // their neighbors, leaving no hexes left.  Leave the default (invalid)
        // center hex in place for such a region.
        if (numHexes[i] > 0) {
            auto &c = centers[i];
            auto &p = positionSums[i];
            c.first = p.first / numHexes[i];
            c.second = p.second / numHexes[i];
        }
    }

    return centers;
}

std::vector<int> voronoi()
{
    // Start with a set of random center points.  Don't worry if there are
    // duplicates.
    std::vector<Point> centers;
    for (int i = 0; i < numRegions; ++i) {
        centers.push_back(hexRandom());
    }

    std::vector<int> regions(mapSize);
    for (int i = 0; i < 4; ++i) {
        // Find the closest center to each point on the map, number those
        // regions.
        for (Sint16 x = 0; x < mapWidth; ++x) {
            for (Sint16 y = 0; y < mapHeight; ++y) {
                regions[y * mapWidth + x] = findClosest(x, y, centers);
            }
        }

        centers = getCenters(regions);
        // Repeat this process to make more regular-looking regions.
    }

    // Assign each hex to its final region.
    for (Sint16 x = 0; x < mapWidth; ++x) {
        for (Sint16 y = 0; y < mapHeight; ++y) {
            regions[y * mapWidth + x] = findClosest(x, y, centers);
        }
    }

    return regions;
}

// Construct an adjacency list for each region.
std::vector<std::vector<int>> regionNeighbors(const std::vector<int> &regions)
{
    assert(regions.size() == unsigned(mapSize));

    std::vector<std::vector<int>> ret(numRegions);
    for (int i = 0; i < mapSize; ++i) {
        int reg = regions[i];
        assert(reg >= 0 && reg < numRegions);

        for (const auto &an : aryNeighbors(i)) {
            int neighborReg = regions[an];
            // If an adjacent hex is in a different region and we haven't
            // already recorded that region as a neighbor, save it.
            if (neighborReg != reg && find(std::begin(ret[reg]),
                                           std::end(ret[reg]),
                                           neighborReg) == std::end(ret[reg])) {
                ret[reg].push_back(neighborReg);
            }
        }
    }

    return ret;
}

// Assign a terrain type to each region using the given adjacency list.
std::vector<int> assignTerrain(const std::vector<std::vector<int>> &adj)
{
    std::vector<int> terrain(numRegions, -1);

    // Greedy coloring.  Each region gets a different terrain from its
    // neighbors, using the lowest number available.
    for (int i = 0; i < numRegions; ++i) {
        // For each neighboring region, save which terrains have been assigned.
        std::bitset<numTerrains> assignedTerrains;
        for (auto n : adj[i]) {
            assert(n >= 0 && n < numRegions);
            if (terrain[n] > -1) {
                assignedTerrains[terrain[n]] = true;
            }
        }
        if (!assignedTerrains.all()) {
            for (int j = 0; j < numTerrains; ++j) {
                if (!assignedTerrains[j]) {
                    terrain[i] = j;
                    break;
                }
            }
        }
        else {
            terrain[i] = 0;
        }
    }

    return terrain;
}

void sdlBlit(const SdlSurface &surf, Sint16 x, Sint16 y)
{
    assert(screen != nullptr);
    SDL_Rect dest = {x, y, 0, 0};
    if (SDL_BlitSurface(surf.get(), nullptr, screen, &dest) < 0) {
        std::cerr << "Warning: error drawing to screen: " << SDL_GetError();
    }
}

SdlSurface sdlLoadImage(const char *filename)
{
    auto temp = make_surface(IMG_Load(filename));
    if (!temp) {
        std::cerr << "Error loading image " << filename
            << "\n    " << IMG_GetError() << '\n';
        return temp;
    }
    auto surf = make_surface(SDL_DisplayFormatAlpha(temp.get()));
    if (!surf) {
        std::cerr << "Error converting to display format: "
            << "\n    " << IMG_GetError() << '\n';
    }

    return surf;
}

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        std::cerr << "Error initializing SDL: " << SDL_GetError();
        return EXIT_FAILURE;
    }
    atexit(SDL_Quit);

    if (IMG_Init(IMG_INIT_PNG) < 0) {
        std::cerr << "Error initializing SDL_image: " << IMG_GetError();
        return EXIT_FAILURE;
    }
    atexit(IMG_Quit);

    // Have to do this prior to SetVideoMode.
    auto icon = make_surface(IMG_Load("../img/icon.png"));
    if (icon != nullptr) {
        SDL_WM_SetIcon(icon.get(), nullptr);
    }
    else {
        std::cerr << "Warning: error loading icon file: " << IMG_GetError();
    }

    screen = SDL_SetVideoMode(882, 684, 0, SDL_SWSURFACE);
    if (screen == nullptr) {
        std::cerr << "Error setting video mode: " << SDL_GetError();
        return EXIT_FAILURE;    
    }
    SDL_WM_SetCaption("Random Map Test", "");

    std::vector<SdlSurface> tiles;
    tiles.push_back(sdlLoadImage("../img/grass.png"));
    tiles.push_back(sdlLoadImage("../img/dirt.png"));
    tiles.push_back(sdlLoadImage("../img/desert.png"));
    tiles.push_back(sdlLoadImage("../img/water.png"));
    tiles.push_back(sdlLoadImage("../img/swamp.png"));
    tiles.push_back(sdlLoadImage("../img/snow.png"));

    auto regions = voronoi();
    auto adjacencyList = regionNeighbors(regions);
    auto terrain = assignTerrain(adjacencyList);

    // Display even-numbered columns.
    for (Sint16 x = 0; x < mapWidth; x += 2) {
        for (Sint16 y = 0; y < mapHeight; ++y) {
            auto terrainIndex = terrain[regions[y * mapWidth + x]];
            sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, y * hexSize);
        }
    }

    // Display odd-numbered columns.
    for (Sint16 x = 1; x < mapWidth; x += 2) {
        for (Sint16 y = 0; y < mapHeight; ++y) {
            auto terrainIndex = terrain[regions[y * mapWidth + x]];
            sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, (y+0.5) * hexSize);
        }
    }

    // Overdraw so we don't get jagged edges.
    for (Sint16 y = -1; y < mapHeight; ++y) {  // left edge, x = -1
        int terrainX = 0;
        int terrainY = std::min(y + 1, mapHeight - 1);
        auto terrainIndex = terrain[regions[terrainY * mapWidth + terrainX]];
        sdlBlit(tiles[terrainIndex], -0.75 * hexSize, (y+0.5) * hexSize);
    }
    for (Sint16 x = 1; x < mapWidth; x += 2) {  // top edge, y = -1
        int terrainX = x;
        int terrainY = 0;
        auto terrainIndex = terrain[regions[terrainY * mapWidth + terrainX]];
        sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, -0.5 * hexSize);
    }
    for (Sint16 y = 0; y < mapHeight + 1; ++y) {  // right edge, x = mapWidth
        int terrainX = mapWidth - 1;
        int terrainY = std::min<int>(y, mapHeight - 1);
        auto terrainIndex = terrain[regions[terrainY * mapWidth + terrainX]];
        sdlBlit(tiles[terrainIndex], mapWidth * hexSize * 0.75, y * hexSize);
    }
    for (Sint16 x = 0; x < mapWidth; x += 2) {  // bottom edge, y = mapHeight
        int terrainX = x;
        int terrainY = mapHeight - 1;
        auto terrainIndex = terrain[regions[terrainY * mapWidth + terrainX]];
        sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, mapHeight * hexSize);
    }

    SDL_UpdateRect(screen, 0, 0, 0, 0);

    bool isDone = false;
    SDL_Event event;
    while (!isDone) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isDone = true;
            }
        }
        SDL_Delay(1);
    }

    // TODO: unit tests?
    assert(hexDist({1, 1}, {2, 2}) == 1);
    assert(hexDist({4, 4}, {3, 3}) == 1);
    assert(hexDist({1, 1}, {3, 3}) == 3);
    assert(hexDist({7, 7}, {5, 5}) == 3);

    for (int i = 0; i < 2; ++i) {
        auto hex = hexRandom();
        auto aryN = aryNeighbors(aryFromHex(hex));
        auto hexN = hexNeighbors(hex);
        assert(aryN.size() == hexN.size());
        std::cout << hex << " neighbors are ";
        for (unsigned int i = 0; i < hexN.size(); ++i) {
            std::cout << hexN[i] << ',';
            assert(aryFromHex(hexN[i]) == aryN[i]);
        }
        std::cout << '\n';
    }

    int count = 0;
    for (const auto &neighbors : adjacencyList) {
        std::cout << count << ": ";
        for_each(std::begin(neighbors), std::end(neighbors), [] (int n) { std::cout << n << ','; });
        std::cout << '\n';
        ++count;
    }

    return 0;
}
