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
#include "hex_utils.h"
#include "sdl_helper.h"
#include <tuple>

namespace
{
    const Sint16 width = 5;
    const Sint16 height = 5;
    const Sint16 pWidth = pHexSize * 3 / 4 * width + pHexSize / 4;
    const Sint16 pHeight = pHexSize * height;
    SDL_Rect window = {0, 0, pWidth, pHeight};
}

Point pixelFromHex(Sint16 hx, Sint16 hy)
{
    Sint16 px = hx * pHexSize * 3 / 4;
    Sint16 py = (hy + 0.5 * abs(hx % 2)) * pHexSize;
    return {px, py};
}

// Draw a 5-hex wide hexagonal grid.
void drawHexGrid(const SdlSurface &tile)
{
    sdlClear(window);
    for (int x = 0; x < 5; ++x) {
        for (int y = 1; y < 4; ++y) {
            sdlBlit(tile, pixelFromHex(x, y));
        }
    }
    sdlBlit(tile, pixelFromHex(1, 0));
    sdlBlit(tile, pixelFromHex(2, 0));
    sdlBlit(tile, pixelFromHex(3, 0));
    sdlBlit(tile, pixelFromHex(2, 4));
}

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (!sdlInit(window.w, window.h, "../img/icon.png", "Animation Test")) {
        return EXIT_FAILURE;
    }

    SdlSurface tile = sdlLoadImage("../img/hex-grid.png");
    drawHexGrid(tile);
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

    return EXIT_SUCCESS;
}
