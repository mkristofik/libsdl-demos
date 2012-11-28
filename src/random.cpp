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
const int numRegions = 6;  // chose 6 because we have 6 types of terrain.
// TODO: use graph coloring algorithm to assign terrain types
// TODO: use h prefix for hex coordinates, p for pixel coordinates

SDL_Surface *screen = nullptr;

std::ostream & operator<<(std::ostream &os, const Point &p)
{
    os << '(' << p.first << ',' << p.second << ')';
    return os;
}

Point hexRandom()
{
    static std::minstd_rand gen(static_cast<unsigned int>(std::time(nullptr)));
    static std::uniform_int_distribution<Sint16> dist(0, mapWidth * mapHeight - 1);
    Sint16 p = dist(gen);
    return std::make_pair(p % mapWidth, p / mapWidth);
}

// source: http://playtechs.blogspot.com/2007/04/hex-grids.html
Sint16 hexDist(const Point &lhs, const Point &rhs)
{
    Sint16 dx = lhs.first - rhs.first;
    Sint16 dy = lhs.second - rhs.second;
    return (abs(dx) + abs(dy) + abs(dx + dy)) / 2;
}

std::vector<Point> hexNeighbors(const Point &hex)
{
    std::vector<Point> hv;

    if (hex.second > 0) {
        // north
        hv.emplace_back(std::make_pair(hex.first, hex.second - 1));
    }
    if (hex.second < mapHeight - 1) {
        // south
        hv.emplace_back(std::make_pair(hex.first, hex.second + 1));
    }
    if (hex.first > 0) {
        if (hex.first % 2 == 0) {
            if (hex.second > 0) {
                // northwest, even column
                hv.emplace_back(std::make_pair(hex.first - 1, hex.second - 1));
            }
            // southwest, even column
            hv.emplace_back(std::make_pair(hex.first - 1, hex.second));
        }
        else {
            // northwest, odd column
            hv.emplace_back(std::make_pair(hex.first - 1, hex.second));
            if (hex.second < mapHeight - 1) {
                // southwest, odd column
                hv.emplace_back(std::make_pair(hex.first - 1, hex.second + 1));
            }
        }
    }
    if (hex.first < mapWidth - 1) {
        if (hex.first % 2 == 0) {
            if (hex.second > 0) {
                // northeast, even column
                hv.emplace_back(std::make_pair(hex.first + 1, hex.second - 1));
            }
            // southeast, even column
            hv.emplace_back(std::make_pair(hex.first + 1, hex.second));
        }
        else {
            // northeast, odd column
            hv.emplace_back(std::make_pair(hex.first + 1, hex.second));
            if (hex.second < mapHeight - 1) {
                // southeast, odd column
                hv.emplace_back(std::make_pair(hex.first + 1, hex.second + 1));
            }
        }
    }

    return hv;
}

// Return the index of the closest center point to the given hex (x,y).
int findClosest(Sint16 x, Sint16 y, const std::vector<Point> &centers)
{
    int closest = -1;
    Sint16 bestSoFar = mapSize + 1;

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
    std::vector<Point> centers(numRegions);

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
        auto &c = centers[i];
        auto &p = positionSums[i];
        c.first = p.first / numHexes[i];
        c.second = p.second / numHexes[i];
    }

    return centers;
}

std::vector<int> voronoi()
{
    // Start with a set of random center points.
    std::vector<Point> centers;
    for (int i = 0; i < numRegions; ++i) {
        centers.emplace_back(hexRandom());
    }

    std::vector<int> regions(mapSize);
    for (int i = 0; i < 2; ++i) {
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
    SdlSurface temp(IMG_Load(filename), SDL_FreeSurface);
    if (!temp) {
        std::cerr << "Error loading image " << filename
            << "\n    " << IMG_GetError() << '\n';
        return temp;
    }
    SdlSurface surf(SDL_DisplayFormatAlpha(temp.get()), SDL_FreeSurface);
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
    SdlSurface icon(IMG_Load("../img/icon.png"), SDL_FreeSurface);
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
    tiles.emplace_back(sdlLoadImage("../img/grass.png"));
    tiles.emplace_back(sdlLoadImage("../img/dirt.png"));
    tiles.emplace_back(sdlLoadImage("../img/swamp.png"));
    tiles.emplace_back(sdlLoadImage("../img/snow.png"));
    tiles.emplace_back(sdlLoadImage("../img/desert.png"));
    tiles.emplace_back(sdlLoadImage("../img/water.png"));

    auto terrain = voronoi();

    // Display even-numbered columns.
    for (Sint16 x = 0; x < mapWidth; x += 2) {
        for (Sint16 y = 0; y < mapHeight; ++y) {
            auto terrainIndex = terrain[y * mapWidth + x];
            sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, y * hexSize);
        }
    }

    // Display odd-numbered columns.
    for (Sint16 x = 1; x < mapWidth; x += 2) {
        for (Sint16 y = 0; y < mapHeight; ++y) {
            auto terrainIndex = terrain[y * mapWidth + x];
            sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, (y+0.5) * hexSize);
        }
    }

    // Overdraw so we don't get jagged edges.
    for (Sint16 y = -1; y < mapHeight; ++y) {  // left edge, x = -1
        int terrainX = 0;
        int terrainY = std::min(y + 1, mapHeight - 1);
        auto terrainIndex = terrain[terrainY * mapWidth + terrainX];
        sdlBlit(tiles[terrainIndex], -0.75 * hexSize, (y+0.5) * hexSize);
    }
    for (Sint16 x = 1; x < mapWidth; x += 2) {  // top edge, y = -1
        int terrainX = x;
        int terrainY = 0;
        auto terrainIndex = terrain[terrainY * mapWidth + terrainX];
        sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, -0.5 * hexSize);
    }
    for (Sint16 y = 0; y < mapHeight + 1; ++y) {  // right edge, x = mapWidth
        int terrainX = mapWidth - 1;
        int terrainY = std::min<int>(y, mapHeight - 1);
        auto terrainIndex = terrain[terrainY * mapWidth + terrainX];
        sdlBlit(tiles[terrainIndex], mapWidth * hexSize * 0.75, y * hexSize);
    }
    for (Sint16 x = 0; x < mapWidth; x += 2) {  // bottom edge, y = mapHeight
        int terrainX = x;
        int terrainY = mapHeight - 1;
        auto terrainIndex = terrain[terrainY * mapWidth + terrainX];
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

    // TODO: unit test?
    for (int i = 0; i < 2; ++i) {
        auto hex = hexRandom();
        std::cout << hex << " neighbors are ";
        for (auto &h : hexNeighbors(hex)) {
            std::cout << h << ',';
        }
        std::cout << '\n';
    }

    return 0;
}
