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

SDL_Surface *screen = nullptr;

SdlSurface make_surface(SDL_Surface *surf)
{
    return SdlSurface(surf, SDL_FreeSurface);
}

// source: SDL_CreateRGBSurface documentation.
SdlSurface sdlCreateSurface(Sint16 width, Sint16 height)
{
    assert(screen);  // this can only be called after SDL_SetVideoMode()
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
        std::cerr << "Warning: error drawing to screen: " << SDL_GetError();
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
