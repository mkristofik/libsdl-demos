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
#include "sdl_helper.h"
#include <cassert>
#include <iostream>
#include <vector>

SDL_Surface *screen = nullptr;

namespace
{
    using DashSize = std::pair<Sint16, Uint16>;  // line-relative pos, width
    std::vector<DashSize> dashedLine(Uint16 lineLen)
    {
        const Uint16 spaceSize = 6;
        const Uint16 dashSize = spaceSize * 3 / 2;

        std::vector<DashSize> dashes;
        Sint16 pos = 0;
        while (pos < lineLen) {
            if (pos + dashSize < lineLen) {
                dashes.emplace_back(pos, dashSize);
                pos += dashSize + spaceSize;
            }
            else {
                dashes.emplace_back(pos, lineLen - pos);
                break;
            }
        }

        return dashes;
    }
}

SdlSurface make_surface(SDL_Surface *surf)
{
    return SdlSurface(surf, SDL_FreeSurface);
}

// source: SDL_CreateRGBSurface documentation.
SdlSurface sdlCreateSurface(Sint16 width, Sint16 height)
{
    // This can only be called after SDL_SetVideoMode()
    assert(screen != nullptr);

    SDL_Surface *surf;
    Uint32 rmask, gmask, bmask, amask;

    // SDL interprets each pixel as a 32-bit number, so our masks must depend
    // on the endianness (byte order) of the machine.
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                                rmask, gmask, bmask, amask);
    if (surf == nullptr) {
        std::cerr << "Error creating new surface: " << SDL_GetError() << '\n';
    }
    return make_surface(surf);
}

SdlSurface sdlDisplayFormat(const SdlSurface &src)
{
    auto surf = make_surface(SDL_DisplayFormatAlpha(src.get()));
    if (!surf) {
        std::cerr << "Error converting to display format: " << SDL_GetError()
            << '\n';
    }
    return surf;
}

void sdlBlit(const SdlSurface &surf, Sint16 px, Sint16 py)
{
    assert(screen != nullptr);
    SDL_Rect dest = {px, py, 0, 0};
    if (SDL_BlitSurface(surf.get(), nullptr, screen, &dest) < 0) {
        std::cerr << "Warning: error drawing to screen: " << SDL_GetError()
            << '\n';
    }
}

void sdlClear(SDL_Rect region)
{
    assert(screen != nullptr);
    auto black = SDL_MapRGB(screen->format, 0, 0, 0);
    if (SDL_FillRect(screen, &region, black) < 0) {
        std::cerr << "Error clearing screen region: " << SDL_GetError() << '\n';
    }
}

SdlSurface sdlLoadImage(const char *filename)
{
    auto img = make_surface(IMG_Load(filename));
    if (!img) {
        std::cerr << "Error loading image " << filename
            << "\n    " << IMG_GetError() << '\n';
        return img;
    }
    return sdlDisplayFormat(img);
}

void sdlDashedLineH(Sint16 px, Sint16 py, Uint16 len, Uint32 color)
{
    assert(screen != nullptr);
    const Uint16 lineWidth = 1;
    for (const auto &dash : dashedLine(len)) {
        SDL_Rect r = {Sint16(px + dash.first), py, dash.second, lineWidth};
        if (SDL_FillRect(screen, &r, color) < 0) {
            std::cerr << "Error drawing horizontal dashed line: "
                << SDL_GetError() << '\n';
            return;
        }
        // TODO: this could be a unit test.
        //std::cout << 'H' << r.x << ',' << r.y << 'x' << r.w << '\n';
    }
}

void sdlDashedLineV(Sint16 px, Sint16 py, Uint16 len, Uint32 color)
{
    assert(screen != nullptr);
    const Uint16 lineWidth = 1;
    for (const auto &dash : dashedLine(len)) {
        SDL_Rect r = {px, Sint16(py + dash.first), lineWidth, dash.second};
        if (SDL_FillRect(screen, &r, color) < 0) {
            std::cerr << "Error drawing horizontal dashed line: "
                << SDL_GetError() << '\n';
            return;
        }
        // TODO: this could be a unit test.
        //std::cout << 'V' << r.x << ',' << r.y << 'x' << r.h << '\n';
    }
}

