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
#ifndef MINIMAP_H
#define MINIMAP_H

#include "sdl_helper.h"
class RandomMap;

class Minimap
{
public:
    Minimap(const RandomMap &map, const SDL_Rect &displayArea);

    void draw();

    // Draw a dotted rectangle representing the current visible map area.
    // Return the screen coordinates of that rectangle.
    SDL_Rect drawBoundingBox();

private:
    void generate();

    const RandomMap &map_;
    SDL_Rect displayArea_;
    Sint16 width_;
    Sint16 height_;
    double hScale_;
    double vScale_;
    SdlSurface surface_;
};

#endif
