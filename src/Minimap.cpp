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

Minimap::Minimap(Sint16 width, const RandomMap &map)
    : map_(map),
    scale_(static_cast<double>(width) / map.pWidth()),
    width_(width),
    height_(map.pHeight() * scale_)
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

void Minimap::draw(Sint16 x, Sint16 y)
{
}
