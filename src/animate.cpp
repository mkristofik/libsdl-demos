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
enum class Animating { NONE, BOWMAN, MARSHAL };

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
    SdlSurface marshal;
    SdlSurface marshalAttack;
    SdlSurface missile;
    SdlSurface archer;
    SdlSurface archerDefend;

    Uint32 animStart_ms = 0;
    auto subject = Animating::NONE;
}

void loadImages()
{
    tile = sdlLoadImage("../img/hex-grid.png");
    bowman = sdlLoadImage("../img/bowman.png");
    bowmanAttack = sdlLoadImage("../img/bowman-attack-ranged.png");
    marshal = sdlLoadImage("../img/marshal.png");
    marshalAttack = sdlLoadImage("../img/marshal-attack-melee.png");
    missile = sdlLoadImage("../img/missile.png");
    archer = sdlLoadImage("../img/orc-archer.png");
    archerDefend = sdlLoadImage("../img/orc-archer-defend.png");
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

    if (event.button == SDL_BUTTON_LEFT) {
        subject = Animating::BOWMAN;
        animStart_ms = SDL_GetTicks();
    }
    else if (event.button == SDL_BUTTON_RIGHT) {
        subject = Animating::MARSHAL;
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
    auto elapsed_ms = SDL_GetTicks() - animStart_ms;
    if (elapsed_ms > 510) {
        sdlBlit(bowman, hex);
        return;
    }

    for (int i = 0; i < 6; ++i) {
        if (elapsed_ms < frameSeq_ms[i]) {
            sdlBlitFrame(bowmanAttack, i, 6, hex);
            break;
        }
    }
}

void drawMissile()
{
    if (subject != Animating::BOWMAN) {
        return;
    }

    auto elapsed_ms = SDL_GetTicks() - animStart_ms;
    if (elapsed_ms < 295 || elapsed_ms > 745) {
        return;
    }

    auto start = pixelFromHex(1, 0);
    auto delta = pixelFromHex(4, 2) - start;

    // Arrow flies 1 hex in 150 ms.  Total distance is 3 hexes, so time of
    // flight is 450 ms.
    auto dt = (elapsed_ms - 295) / 450.0;
    Sint16 px = start.first + dt * delta.first;
    Sint16 py = start.second + dt * delta.second;

    // If we did this for real, we'd have to compute angle.  In this case we
    // know arrow flies southeast.
    auto direction = static_cast<int>(Dir::SE);
    sdlBlitFrame(missile, direction, 6, px, py);
}

void drawMarshal()
{
    auto hex = pixelFromHex(1, 2);
    if (subject != Animating::MARSHAL) {
        sdlBlit(marshal, hex);
        return;
    }

    Uint32 frameSeq_ms[] = {50, 100, 200, 275, 375, 425, 500};
    auto elapsed_ms = SDL_GetTicks() - animStart_ms;
    if (elapsed_ms > 600) {
        sdlBlit(marshal, hex);
        subject = Animating::NONE;
        // TODO: resetting this actually depends on the defender's hit animation
        return;
    }

    // attack animation lasts 600 ms
    // first half, slide toward hex 2,3 (get halfway there)
    // second half, slide back
    auto target = pixelFromHex(2, 3);
    auto delta = (target - hex) / 2;
    Sint16 drawX = 0;
    Sint16 drawY = 0;
    if (elapsed_ms < 300) {
        drawX = elapsed_ms / 300.0 * delta.first + hex.first;
        drawY = elapsed_ms / 300.0 * delta.second + hex.second;
    }
    else {
        drawX = (600 - elapsed_ms) / 300.0 * delta.first + hex.first;
        drawY = (600 - elapsed_ms) / 300.0 * delta.second + hex.second;
    }

    // Past the end of the animated frames, draw the base image.
    if (elapsed_ms > 500) {
        sdlBlit(marshal, drawX, drawY);
        return;
    }

    for (int i = 0; i < 7; ++i) {
        if (elapsed_ms < frameSeq_ms[i]) {
            sdlBlitFrame(marshalAttack, i, 7, drawX, drawY);
            break;
        }
    }
}

void drawEnemy1()
{
    auto hex = pixelFromHex(4, 2);
    if (subject != Animating::BOWMAN) {
        sdlBlit(archer, hex);
        return;
    }

    auto elapsed_ms = SDL_GetTicks() - animStart_ms;
    if (elapsed_ms < 745) {  // shooter animation plus time of flight
        sdlBlit(archer, hex);
    }
    else if (elapsed_ms < 995) {
        sdlBlit(archerDefend, hex);
    }
    else {
        sdlBlit(archer, hex);
        subject = Animating::NONE;
    }
}

// enemy 2 at hex 2,3 hit by sword

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (!sdlInit(window.w, window.h, "../img/icon.png", "Animation Test")) {
        return EXIT_FAILURE;
    }

    loadImages();

    drawHexGrid();
    drawBowman();
    drawMarshal();
    drawEnemy1();
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
            drawMissile();
            drawMarshal();
            drawEnemy1();
            SDL_UpdateRect(screen, 0, 0, 0, 0);
        }

        SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}
