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
#include "terrain.h"
#include <cassert>
#include <iostream>
#include <tuple>

Minimap::Minimap(const RandomMap &map, const SDL_Rect &displayArea)
    : map_(map),
    displayArea_(displayArea),
    width_(displayArea_.w),
    height_(displayArea_.h),
    hScale_(map_.pWidth() / static_cast<double>(width_)),
    vScale_(map_.pHeight() / static_cast<double>(height_)),
    surface_()
{
}

void Minimap::draw()
{
    if (!surface_) {
        generate();
    }
    if (surface_) {
        sdlClear(displayArea_);
        sdlBlit(surface_, displayArea_.x, displayArea_.y);
    }
}

SDL_Rect Minimap::drawBoundingBox()
{
    Sint16 sx = displayArea_.x;
    Sint16 sy = displayArea_.y;
    Sint16 mapX = 0;
    Sint16 mapY = 0;
    std::tie(mapX, mapY) = map_.mDrawnAt();
    const auto &visibleArea = map_.getDisplayArea();

    // Scale the visible area of the map to the size of the minimap.
    Sint16 box_nw_x = sx + static_cast<double>(mapX) / map_.pWidth() * width_;
    Sint16 box_nw_y = sy + static_cast<double>(mapY) / map_.pHeight() * height_;
    Sint16 box_se_x = sx + static_cast<double>(mapX + visibleArea.w - 1) /
        map_.pWidth() * width_;
    Sint16 box_se_y = sy + static_cast<double>(mapY + visibleArea.h - 1) /
        map_.pHeight() * height_;
    Uint16 box_width = box_se_x - box_nw_x;
    Uint16 box_height = box_se_y - box_nw_y;

    auto white = SDL_MapRGB(screen->format, 255, 255, 255);
    sdlDashedLineH(box_nw_x, box_nw_y, box_width, white);
    sdlDashedLineV(box_nw_x, box_nw_y, box_height, white);
    sdlDashedLineH(box_nw_x, box_se_y, box_width, white);
    sdlDashedLineV(box_se_x, box_nw_y, box_height, white);

    return {box_nw_x, box_nw_y, box_width, box_height};
}

void Minimap::generate()
{
    auto surf = sdlCreateSurface(width_, height_);
    Uint32 terrainColors[] = {SDL_MapRGB(surf->format, 16, 96, 16),  // grass
                              SDL_MapRGB(surf->format, 112, 112, 64),  // dirt
                              SDL_MapRGB(surf->format, 208, 192, 128),  // sand
                              SDL_MapRGB(surf->format, 0, 64, 144),  // water
                              SDL_MapRGB(surf->format, 48, 48, 48),  // swamp
                              SDL_MapRGB(surf->format, 240, 240, 240)};  // snow

    // For each pixel of minimap,
    //     Get corresponding set of pixels on the main map
    //     Find most common terrain type in that set
    //     Draw that color for the pixel
    //
    // note: this will have to be fixed if BitsPerPixel is ever not 32.
    SdlLock(surf.get(), [&] {
        for (Sint16 x = 0; x < width_; ++x) {
            for (Sint16 y = 0; y < height_; ++y) {
                int mostCommon = GRASS;
                int count = 0;
                for (Sint16 i = 0; i < hScale_; ++i) {
                    for (Sint16 j = 0; j < vScale_; ++j) {
                        int terrain = map_.getTerrainAt(x * hScale_ + i,
                                                        y * vScale_ + j);
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
                auto pixel = static_cast<Uint32 *>(surf->pixels) + y*width_ + x;
                *pixel = terrainColors[mostCommon];
            }
        }
    });

    surface_ = sdlDisplayFormat(surf);
}
