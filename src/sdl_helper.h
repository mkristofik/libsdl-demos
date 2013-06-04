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
#ifndef SDL_HELPER_H
#define SDL_HELPER_H

#include "SDL.h"
#include "SDL_image.h"
#include <memory>
#include <utility>

using SdlSurface = std::shared_ptr<SDL_Surface>;

extern SDL_Surface *screen;

// Must call this before any other SDL functions will work.  There is no
// recovery if this returns false (you should exit the program).
bool sdlInit(Sint16 winWidth, Sint16 winHeight, const char *iconPath,
             const char *caption);

// Like std::make_shared, but with SDL_Surface.
SdlSurface make_surface(SDL_Surface *surf);

// Create a new surface of the given width and height.  Return a null surface
// on failure.
SdlSurface sdlCreateSurface(Sint16 width, Sint16 height);

// Convert the given surface to the screen format.  Return a null surface on
// failure.
SdlSurface sdlDisplayFormat(const SdlSurface &src);

// Draw the full surface to the screen using (px,py) as the upper-left corner.
// Use the raw SDL_BlitSurface if you need something more specific.
void sdlBlit(const SdlSurface &surf, Sint16 px, Sint16 py);

// Clear the given region of the screen.
void sdlClear(SDL_Rect region);

// Load an image from disk.  Returns a null surface on failure.
SdlSurface sdlLoadImage(const char *filename);

// Draw a dashed line to the screen starting at (px,py).
void sdlDashedLineH(Sint16 px, Sint16 py, Uint16 len, Uint32 color);
void sdlDashedLineV(Sint16 px, Sint16 py, Uint16 len, Uint32 color);

// Rectangle functions.
bool insideRect(Sint16 x, Sint16 y, const SDL_Rect &rect);
std::pair<double, double> rectPct(Sint16 x, Sint16 y, const SDL_Rect &rect);
enum class Dir8 {None = -1, N, NE, E, SE, S, SW, W, NW};
Dir8 nearEdge(Sint16 x, Sint16 y, const SDL_Rect &rect);

#endif
