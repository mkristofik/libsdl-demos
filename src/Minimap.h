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
    Minimap(Sint16 width, const RandomMap &map);

    Sint16 width() const;
    Sint16 height() const;

    void draw(Sint16 x, Sint16 y);

private:
    void generate();

    const RandomMap &map_;
    double scale_;
    Sint16 width_;
    Sint16 height_;
    SdlSurface surface_;
};

#endif
