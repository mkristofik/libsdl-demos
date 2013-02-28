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
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

namespace
{
    SDL_Rect mapArea = {10, 10, 882, 684};  // sized to hold 16x9 hexes
    SDL_Rect minimapArea = {902, 10, 200, 167};

    std::unique_ptr<RandomMap> rmap;
    std::unique_ptr<Minimap> mini;
    SDL_Rect miniBox;  // screen area of bounding box inside minimap

    bool minimapHasFocus = false;
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

    rmap->draw(tgtMapX, tgtMapY);
    mini->draw();
    mini->drawBoundingBox();
}

// If the user clicks inside the minimap, try to center the bounding box on the
// mouse cursor.
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
}

// If the user has clicked inside the minimap, moving the mouse will drag the
// bounding box.
void handleMouseMotion(const SDL_MouseMotionEvent &event)
{
    if (!minimapHasFocus) {
        return;
    }

    if (event.state & SDL_BUTTON(1)) {
        moveMiniBoxCenter(event.x, event.y);
    }
}

// After releasing the mouse button, the minimap bounding box no longer moves.
void handleMouseUp(const SDL_MouseButtonEvent &event)
{
    minimapHasFocus = false;
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

    screen = SDL_SetVideoMode(1112, 704, 0, SDL_SWSURFACE);
    if (screen == nullptr) {
        std::cerr << "Error setting video mode: " << SDL_GetError();
        return EXIT_FAILURE;    
    }
    SDL_WM_SetCaption("Random Map Test", "");

    rmap = make_unique<RandomMap>(32, 18, mapArea);
    mini = make_unique<Minimap>(*rmap, minimapArea);

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

    bool isDone = false;
    SDL_Event event;
    while (!isDone) {
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
        SDL_UpdateRect(screen, 0, 0, 0, 0);
        SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}
