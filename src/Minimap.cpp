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
#include "Minimap.h"

#include "RandomMap.h"
#include "sdl_helper.h"
#include "terrain.h"
#include <iostream>

Minimap::Minimap(Sint16 width, const RandomMap &map)
    : map_(map),
    scale_(map.pWidth() / static_cast<double>(width)),
    width_(width),
    height_(map.pHeight() / scale_)
{
}

Sint16 Minimap::width() const
{
    return width_;
}

Sint16 Minimap::height() const
{
    return height_;
}

void Minimap::draw(Sint16 sx, Sint16 sy)
{
    auto surf = sdlCreateSurface(width_, height_);
    Uint32 green = SDL_MapRGB(surf->format, 0, 255, 0);
    Uint32 blue = SDL_MapRGB(surf->format, 0, 0, 255);
    Uint32 black = SDL_MapRGB(surf->format, 0, 0, 0);

    if (SDL_MUSTLOCK(surf.get())) {
        if (SDL_LockSurface(surf.get()) < 0) {
            std::cerr << "Error locking surface: " << SDL_GetError() << '\n';
            return;
        }
    }
    /*
    For each pixel of minimap,
        Get corresponding set of pixels on the main map (use scale factor)
        Find most common terrain type in that set
        Draw that color for the pixel
    */
    // Note: this will have to be fixed if BitsPerPixel is ever not 32.
    Sint16 blockSize = scale_;
    for (Sint16 x = 0; x < width_; ++x) {
        for (Sint16 y = 0; y < height_; ++y) {
            int mostCommon = GRASS;
            int count = 0;
            for (Sint16 i = 0; i < blockSize; ++i) {
                for (Sint16 j = 0; j < blockSize; ++j) {
                    int terrain = map_.getTerrainAt(x * scale_ + i,
                                                    y * scale_ + j);
                    if (terrain == mostCommon) {
                        ++count;
                    }
                    else if (count == 0) {
                        mostCommon = terrain;
                        count = 1;
                    }
                    else {
                        --count;
                    }
                }
            }
            Uint32 *pixel = static_cast<Uint32 *>(surf->pixels) + y * width_ + x;
            std::cout << pixel << '\n';
            if (mostCommon == GRASS) {
                *pixel = green;
            }
            else if (mostCommon == WATER) {
                *pixel = blue;
            }
            else {
                *pixel = black;
            }
        }
    }
    if (SDL_MUSTLOCK(surf.get())) {
        SDL_UnlockSurface(surf.get());
    }
    auto surf2 = make_surface(SDL_DisplayFormatAlpha(surf.get()));
    if (!surf2) {
        std::cerr << "Error converting to display format: "
            << "\n    " << IMG_GetError() << '\n';
    }
    sdlBlit(surf2, sx, sy);
}
