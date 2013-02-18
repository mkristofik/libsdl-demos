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
#include <cassert>
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
    Uint32 terrainColors[] = {SDL_MapRGB(surf->format, 16, 96, 16),  // grass
                              SDL_MapRGB(surf->format, 112, 112, 64),  // dirt
                              SDL_MapRGB(surf->format, 208, 192, 128),  // sand
                              SDL_MapRGB(surf->format, 0, 64, 144),  // water
                              SDL_MapRGB(surf->format, 48, 48, 48),  // swamp
                              SDL_MapRGB(surf->format, 240, 240, 240)};  // snow

    // TODO: RAII this and the unlock.
    if (SDL_MUSTLOCK(surf.get())) {
        if (SDL_LockSurface(surf.get()) < 0) {
            std::cerr << "Error locking surface: " << SDL_GetError() << '\n';
            return;
        }
    }

    // For each pixel of minimap,
    //     Get corresponding set of pixels on the main map
    //     Find most common terrain type in that set
    //     Draw that color for the pixel
    //
    // note: this will have to be fixed if BitsPerPixel is ever not 32.
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
            assert(mostCommon >= 0 && mostCommon < NUM_TERRAINS);
            Uint32 *pixel = static_cast<Uint32 *>(surf->pixels) + y * width_ + x;
            *pixel = terrainColors[mostCommon];
        }
    }
    if (SDL_MUSTLOCK(surf.get())) {
        SDL_UnlockSurface(surf.get());
    }
    auto surf2 = sdlDisplayFormat(surf);
    if (surf2) {
        sdlBlit(surf2, sx, sy);
    }
}
