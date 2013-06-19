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

// Who is animating right now?
enum class Animating { NONE, BOWMAN, MARSHAL, GRUNT };

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
    SdlSurface marshalDefend;
    SdlSurface missile;
    SdlSurface archer;
    SdlSurface archerDefend;
    SdlSurface grunt;
    SdlSurface gruntDefend;
    SdlSurface gruntAttack;

    Uint32 animStart_ms = 0;
    auto subject = Animating::NONE;

    bool bowSoundPlayed = false;
    bool archerHitSoundPlayed = false;
    bool swordSoundPlayed = false;
    bool gruntHitSoundPlayed = false;
    bool retaliateSoundPlayed = false;
    bool marshalHitSoundPlayed = false;
}

void loadImages()
{
    tile = sdlLoadImage("../img/hex-grid.png");
    bowman = sdlLoadImage("../img/bowman.png");
    bowmanAttack = sdlLoadImage("../img/bowman-attack-ranged.png");
    marshal = sdlLoadImage("../img/marshal.png");
    marshalAttack = sdlLoadImage("../img/marshal-attack-melee.png");
    marshalDefend = sdlLoadImage("../img/marshal-defend.png");
    missile = sdlLoadImage("../img/missile.png");
    archer = sdlFlipH(sdlLoadImage("../img/orc-archer.png"));
    archerDefend = sdlFlipH(sdlLoadImage("../img/orc-archer-defend.png"));
    grunt = sdlFlipH(sdlLoadImage("../img/orc-grunt.png"));
    gruntDefend = sdlFlipH(sdlLoadImage("../img/orc-grunt-defend.png"));
    gruntAttack = sdlFlipSheetH(sdlLoadImage("../img/orc-grunt-attack-melee.png"), 7);
}

Point pixelFromHex(Sint16 hx, Sint16 hy)
{
    Sint16 px = hx * pHexSize * 3 / 4;
    Sint16 py = (hy + 0.5 * abs(hx % 2)) * pHexSize;
    return {px, py};
}

// attack animation lasts 600 ms
// first half, slide toward hex 2,3 (get halfway there)
// second half, slide back
Point slideToTarget(const Point &src, const Point &target, Uint32 elapsed_ms)
{
    auto delta = (target - src) / 2;
    Sint16 drawX = 0;
    Sint16 drawY = 0;
    if (elapsed_ms < 300) {
        drawX = elapsed_ms / 300.0 * delta.first + src.first;
        drawY = elapsed_ms / 300.0 * delta.second + src.second;
    }
    else {
        drawX = (600 - elapsed_ms) / 300.0 * delta.first + src.first;
        drawY = (600 - elapsed_ms) / 300.0 * delta.second + src.second;
    }

    return {drawX, drawY};
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
        bowSoundPlayed = false;
        archerHitSoundPlayed = false;
    }
    else if (event.button == SDL_BUTTON_RIGHT) {
        subject = Animating::MARSHAL;
        animStart_ms = SDL_GetTicks();
        swordSoundPlayed = false;
        gruntHitSoundPlayed = false;
        retaliateSoundPlayed = false;
        marshalHitSoundPlayed = false;
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

void drawBowman(const SdlSound &bowSound)
{
    auto hex = pixelFromHex(1, 0);
    if (subject != Animating::BOWMAN) {
        sdlBlit(bowman, hex);
        return;
    }

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

    if (!bowSoundPlayed && elapsed_ms > 140) {
        sdlPlaySound(bowSound);
        bowSoundPlayed = true;
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

void drawMarshal(const SdlSound &swordSound, const SdlSound &hitSound)
{
    auto hex = pixelFromHex(1, 2);
    auto elapsed_ms = SDL_GetTicks() - animStart_ms;

    if (subject == Animating::MARSHAL) {
        Uint32 frameSeq_ms[] = {50, 100, 200, 275, 375, 425, 500};
        if (elapsed_ms > 600) {
            sdlBlit(marshal, hex);

            // Start a retaliation after the attack finishes.
            subject = Animating::GRUNT;
            animStart_ms = SDL_GetTicks();
            return;
        }

        auto drawPos = slideToTarget(hex, pixelFromHex(2, 3), elapsed_ms);

        // Past the end of the animated frames, draw the base image.
        if (elapsed_ms > 500) {
            sdlBlit(marshal, drawPos);
            return;
        }

        for (int i = 0; i < 7; ++i) {
            if (elapsed_ms < frameSeq_ms[i]) {
                sdlBlitFrame(marshalAttack, i, 7, drawPos);
                break;
            }
        }

        if (!swordSoundPlayed && elapsed_ms > 100) {
            sdlPlaySound(swordSound);
            swordSoundPlayed = true;
        }
    }
    else if (subject == Animating::GRUNT) {
        if (elapsed_ms >= 300 && elapsed_ms < 550) {
            sdlBlit(marshalDefend, hex);
            if (!marshalHitSoundPlayed) {
                sdlPlaySound(hitSound);
                marshalHitSoundPlayed = true;
            }
        }
        else {
            sdlBlit(marshal, hex);
        }
    }
    else {
        sdlBlit(marshal, hex);
    }
}

void drawEnemy1(const SdlSound &hitSound)
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
        if (!archerHitSoundPlayed) {
            sdlPlaySound(hitSound);
            archerHitSoundPlayed = true;
        }
    }
    else {
        sdlBlit(archer, hex);
        subject = Animating::NONE;
    }
}

void drawEnemy2(const SdlSound &swordSound, const SdlSound &hitSound)
{
    auto hex = pixelFromHex(2, 3);
    auto elapsed_ms = SDL_GetTicks() - animStart_ms;

    if (subject == Animating::GRUNT) {
        Uint32 frameSeq_ms[] = {50, 100, 200, 275, 375, 425, 500};
        if (elapsed_ms > 600) {
            sdlBlit(grunt, hex);
            subject = Animating::NONE;
            return;
        }

        auto drawPos = slideToTarget(hex, pixelFromHex(1, 2), elapsed_ms);

        // Past the end of the animated frames, draw the base image.
        if (elapsed_ms > 500) {
            sdlBlit(grunt, drawPos);
            return;
        }

        for (int i = 0; i < 7; ++i) {
            if (elapsed_ms < frameSeq_ms[i]) {
                sdlBlitFrame(gruntAttack, i, 7, drawPos);
                break;
            }
        }

        if (!retaliateSoundPlayed && elapsed_ms > 100) {
            sdlPlaySound(swordSound);
            retaliateSoundPlayed = true;
        }
    }
    else if (subject == Animating::MARSHAL) {
        if (elapsed_ms >= 300 && elapsed_ms < 550) {
            sdlBlit(gruntDefend, hex);
            if (!gruntHitSoundPlayed) {
                sdlPlaySound(hitSound);
                gruntHitSoundPlayed = true;
            }
        }
        else {
            sdlBlit(grunt, hex);
        }
    }
    else {
        sdlBlit(grunt, hex);
    }
}

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (!sdlInit(window.w, window.h, "../img/icon.png", "Animation Test")) {
        return EXIT_FAILURE;
    }

    loadImages();

    // Load sounds (can't do this at file scope).
    auto bowFired = sdlLoadSound("../sounds/bow.ogg");
    auto marshalHit = sdlLoadSound("../sounds/human-hit.ogg");
    auto archerHit = sdlLoadSound("../sounds/orc-small-hit.ogg");
    auto gruntHit = sdlLoadSound("../sounds/orc-hit.ogg");
    auto swordSwing = sdlLoadSound("../sounds/sword.ogg");
    auto theme = sdlLoadMusic("../music/battle.ogg");

    drawHexGrid();
    drawBowman(bowFired);
    drawMarshal(swordSwing, marshalHit);
    drawEnemy1(archerHit);
    drawEnemy2(swordSwing, gruntHit);
    sdlPlayMusic(theme);
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    bool isDone = false;
    SDL_Event event;
    while (!isDone) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                Mix_HaltMusic();
                Mix_HaltChannel(-1);
                isDone = true;
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                handleMouseUp(event.button);
            }
        }

        if (subject != Animating::NONE) {
            sdlClear(window);
            drawHexGrid();
            drawBowman(bowFired);
            drawMissile();
            // order matters, we want the attacker drawn on top
            if (subject == Animating::MARSHAL) {
                drawEnemy2(swordSwing, gruntHit);
                drawMarshal(swordSwing, marshalHit);
            }
            else {
                drawMarshal(swordSwing, marshalHit);
                drawEnemy2(swordSwing, gruntHit);
            }
            drawEnemy1(archerHit);
            SDL_UpdateRect(screen, 0, 0, 0, 0);
        }

        SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}
