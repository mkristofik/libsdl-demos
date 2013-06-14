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
#include "sdl_helper.h"
#include <iostream>
#include <tuple>

// Who is animating right now?
enum class Animating { NONE, BOWMAN };

namespace
{
    const Sint16 width = 5;
    const Sint16 height = 5;
    const Sint16 pWidth = pHexSize * 3 / 4 * width + pHexSize / 4;
    const Sint16 pHeight = pHexSize * height;
    SDL_Rect window = {0, 0, pWidth, pHeight};

    SdlSurface tile;
    SdlSurface bowman;
    SdlSurface bowmanAttack;

    Uint32 animStart_ms = 0;
    auto subject = Animating::NONE;
}

void loadImages()
{
    tile = sdlLoadImage("../img/hex-grid.png");
    bowman = sdlLoadImage("../img/bowman.png");
    bowmanAttack = sdlLoadImage("../img/bowman-attack-ranged.png");
}

Point pixelFromHex(Sint16 hx, Sint16 hy)
{
    Sint16 px = hx * pHexSize * 3 / 4;
    Sint16 py = (hy + 0.5 * abs(hx % 2)) * pHexSize;
    return {px, py};
}

void handleMouseUp(const SDL_MouseButtonEvent &event)
{
    // Ignore mouse clicks while animating.
    if (subject != Animating::NONE) {
        return;
    }

    // Left mouse button starts the bowman animation.
    if (event.button == SDL_BUTTON_LEFT) {
        subject = Animating::BOWMAN;
        animStart_ms = SDL_GetTicks();
    }
}

// Draw a 5-hex wide hexagonal grid.
void drawHexGrid()
{
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

void drawBowman()
{
    auto hex = pixelFromHex(1, 0);
    if (subject != Animating::BOWMAN) {
        sdlBlit(bowman, hex);
        return;
    }

    // TODO:
    // - missile fires 295 ms
    // - in wesnoth, missile hits 445 ms
    Uint32 frameSeq_ms[] = {65, 140, 215, 315, 445, 510};
    bool animDone = true;

    auto elapsed_ms = SDL_GetTicks() - animStart_ms;
    for (int i = 0; i < 6; ++i) {
        if (elapsed_ms < frameSeq_ms[i]) {
            sdlBlitFrame(bowmanAttack, i, 6, hex);
            animDone = false;
            break;
        }
    }
    if (animDone) {
        sdlBlit(bowman, hex);
        subject = Animating::NONE;
        // TODO: resetting this actually depends on the flight of the arrow and
        // the hit animation of the target.
    }
}

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (!sdlInit(window.w, window.h, "../img/icon.png", "Animation Test")) {
        return EXIT_FAILURE;
    }

    loadImages();

    drawHexGrid();
    drawBowman();
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    bool isDone = false;
    SDL_Event event;
    while (!isDone) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isDone = true;
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                handleMouseUp(event.button);
            }
        }

        if (subject != Animating::NONE) {
            sdlClear(window);
            drawHexGrid();
            drawBowman();
            SDL_UpdateRect(screen, 0, 0, 0, 0);
        }

        SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}
