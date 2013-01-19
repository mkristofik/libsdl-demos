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
#ifndef HEX_UTILS_H
#define HEX_UTILS_H

#include "SDL_types.h"
#include <iosfwd>
#include <utility>

using Point = std::pair<Sint16, Sint16>;
const Point hInvalid = {-1, -1};

std::ostream & operator<<(std::ostream &os, const Point &p);
Sint16 hexDist(const Point &h1, const Point &h2);

#endif
