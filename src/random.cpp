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
#include "Minimap.h"
#include "RandomMap.h"
#include "algo.h"
#include "hex_utils.h"
#include "sdl_helper.h"
#include "terrain.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>

namespace
{
    SDL_Rect mapArea = {10, 10, 882, 684};  // sized to hold 16x9 hexes
    SDL_Rect minimapArea = {902, 10, 200, 167};

    std::unique_ptr<RandomMap> rmap;
    std::unique_ptr<Minimap> mini;
    SDL_Rect miniBox;  // screen area of bounding box inside minimap
    bool minimapHasFocus = false;

    Uint32 elapsed_ms;  // time since last frame
    Uint32 timeNearEdge_ms;  // time mouse spent near map edges
    Dir8 mouseNearMapEdge;
    Point nextMapLoc;  // where to move the map next
    Point nextHex;  // where to move the selected hex
    Point pathToHex;  // highlight a path to here
    Point pathToHexPrev;
}

// Try to center the minimap's bounding box at the given screen coordinates,
// moving the main map accordingly.
void moveMiniBoxCenter(Sint16 px, Sint16 py)
{
    Sint16 tgtX = px - miniBox.w / 2;
    Sint16 tgtY = py - miniBox.h / 2;
    auto pct = rectPct(tgtX, tgtY, minimapArea);
    Sint16 tgtMapX = pct.first * rmap->pWidth();
    Sint16 tgtMapY = pct.second * rmap->pHeight();
    auto mapLimit = rmap->maxPixel();
    tgtMapX = bound(tgtMapX, 0, mapLimit.first);
    tgtMapY = bound(tgtMapY, 0, mapLimit.second);

    nextMapLoc = {tgtMapX, tgtMapY};
}

void scrollMap(Dir8 direction)
{
    const Sint16 mapScrollRate_pps = pHexSize * 4;  // pixels per second
    const Sint16 pScroll = std::max<Sint16>(1, mapScrollRate_pps * elapsed_ms /
                                               1000);
    const Sint16 pScrollDiag = std::max<Sint16>(1,
        mapScrollRate_pps * elapsed_ms / 1000 / std::sqrt(2));

    auto curPixel = rmap->mDrawnAt();
    Sint16 px = curPixel.first;
    Sint16 py = curPixel.second;
    auto maxPixel = rmap->maxPixel();
    Sint16 maxX = maxPixel.first;
    Sint16 maxY = maxPixel.second;

    switch (direction) {
        case Dir8::N:
            py = std::max<Sint16>(0, py - pScroll);
            break;
        case Dir8::NE:
            px = std::min<Sint16>(maxX, px + pScrollDiag);
            py = std::max<Sint16>(0, curPixel.second - pScrollDiag);
            break;
        case Dir8::E:
            px = std::min<Sint16>(maxX, px + pScroll);
            break;
        case Dir8::SE:
            px = std::min<Sint16>(maxX, px + pScrollDiag);
            py = std::min<Sint16>(maxY, py + pScrollDiag);
            break;
        case Dir8::S:
            py = std::min<Sint16>(maxY, py + pScroll);
            break;
        case Dir8::SW:
            px = std::max<Sint16>(0, px - pScrollDiag);
            py = std::min<Sint16>(maxY, py + pScroll);
            break;
        case Dir8::W:
            px = std::max<Sint16>(0, px - pScroll);
            break;
        case Dir8::NW:
            px = std::max<Sint16>(0, px - pScrollDiag);
            py = std::max<Sint16>(0, curPixel.second - pScrollDiag);
            break;
        default:
            break;
    }

    nextMapLoc = {px, py};
}

// If the user clicks inside the minimap, try to center the bounding box on the
// mouse cursor.  If the user clicks inside the main map, highlight that hex.
void handleMouseDown(const SDL_MouseButtonEvent &event)
{
    if (event.button == SDL_BUTTON_LEFT &&
        insideRect(event.x, event.y, minimapArea))
    {
        minimapHasFocus = true;
        moveMiniBoxCenter(event.x, event.y);
    }
    else {
        minimapHasFocus = false;
    }

    if (event.button == SDL_BUTTON_LEFT &&
        insideRect(event.x, event.y, mapArea)) {
        nextHex = rmap->getHexAtS(event.x, event.y);
    }
}

// If the user has clicked inside the minimap, moving the mouse will drag the
// bounding box.  Otherwise, track whether the mouse is near an edge of the
// main map (so we can scroll it).
void handleMouseMotion(const SDL_MouseMotionEvent &event)
{
    if (minimapHasFocus) {
        if (event.state & SDL_BUTTON(1)) {
            moveMiniBoxCenter(event.x, event.y);
        }
    }
    else {
        mouseNearMapEdge = nearEdge(event.x, event.y, mapArea);
        if (insideRect(event.x, event.y, mapArea) &&
            rmap->getSelectedHex() != hInvalid)
        {
            pathToHex = rmap->getHexAtS(event.x, event.y);
        }
        else {
            pathToHex = hInvalid;
        }
    }
}

// After releasing the mouse button, the minimap bounding box no longer moves.
void handleMouseUp(const SDL_MouseButtonEvent &event)
{
    minimapHasFocus = false;
}

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (!sdlInit(1112, 704, "../img/icon.png", "Random Map Test")) {
        return EXIT_FAILURE;
    }

    rmap = make_unique<RandomMap>(32, 18, mapArea);
    mini = make_unique<Minimap>(*rmap, minimapArea);
    timeNearEdge_ms = 0;
    mouseNearMapEdge = Dir8::None;
    pathToHex = hInvalid;
    pathToHexPrev = hInvalid;

    // TODO: unit tests for this would require an SDL main.  These assume the
    // map is drawn in the upper left corner of the screen.
    /*
    assert(str(m.getHexAtS(-1, -1)) == str(hInvalid));
    assert(str(m.getHexAtS(0, 0)) == str({-1, -1}));
    assert(str(m.getHexAtS(36, 36)) == str({0, 0}));
    assert(str(m.getHexAtS(36, 108)) == str({0, 1}));
    assert(str(m.getHexAtS(90, 144)) == str({1, 1}));
    */

    rmap->draw(0, 0);
    mini->draw();
    miniBox = mini->drawBoundingBox();
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    std::vector<Uint32> frames;
    bool isDone = false;
    auto prevFrameTime_ms = SDL_GetTicks();
    SDL_Event event;
    while (!isDone) {
        auto curTime_ms = SDL_GetTicks();
        elapsed_ms = curTime_ms - prevFrameTime_ms;
        prevFrameTime_ms = curTime_ms;
        if (!frames.empty() || elapsed_ms > 0) {  // skip zeroth frame
            frames.push_back(elapsed_ms);
        }

        nextMapLoc = rmap->mDrawnAt();
        nextHex = rmap->getSelectedHex();
        pathToHexPrev = pathToHex;

        // Scroll the map if the mouse hovers near a map edge for more than a
        // second.
        if (mouseNearMapEdge != Dir8::None) {
            timeNearEdge_ms += elapsed_ms;
        }
        else {
            timeNearEdge_ms = 0;
        }
        if (timeNearEdge_ms > 1000) {
            scrollMap(mouseNearMapEdge);
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                handleMouseDown(event.button);
            }
            else if (event.type == SDL_MOUSEMOTION) {
                handleMouseMotion(event.motion);
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                handleMouseUp(event.button);
            }
            else if (event.type == SDL_QUIT) {
                isDone = true;
            }
        }

        if (nextMapLoc != rmap->mDrawnAt() ||
            nextHex != rmap->getSelectedHex() ||
            pathToHex != pathToHexPrev)
        {
            rmap->selectHex(nextHex);
            rmap->highlightPath(rmap->getSelectedHex(), pathToHex);
            rmap->draw(nextMapLoc.first, nextMapLoc.second);
            mini->draw();
            mini->drawBoundingBox();
        }

        SDL_UpdateRect(screen, 0, 0, 0, 0);
        SDL_Delay(1);
    }

    std::cout << "Average frame time: " << std::accumulate(std::begin(frames), std::end(frames), 0) / static_cast<double>(frames.size()) << '\n';
    std::cout << "Minimum frame: " << *min_element(std::begin(frames), std::end(frames)) << '\n';
    std::cout << "Maximum frame: " << *max_element(std::begin(frames), std::end(frames)) << '\n';
    return EXIT_SUCCESS;
}
