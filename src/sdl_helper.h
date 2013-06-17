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
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include "hex_utils.h"
#include <iostream>
#include <memory>
#include <string>
#include <utility>

using SdlSurface = std::shared_ptr<SDL_Surface>;
using SdlFont = std::unique_ptr<TTF_Font, void(*)(TTF_Font *)>;
using SdlMusic = std::shared_ptr<Mix_Music>;

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

// Flip a surface or sprite sheet.  Creates a new surface.
SdlSurface sdlFlipH(const SdlSurface &src);

// Draw the full surface to the screen using (px,py) as the upper-left corner.
// Use the raw SDL_BlitSurface if you need something more specific.
void sdlBlit(const SdlSurface &surf, Sint16 px, Sint16 py);
void sdlBlit(const SdlSurface &surf, const Point &pos);

// Draw a portion of a sprite sheet to the screen.  Assumes each frame is the
// same size.
void sdlBlitFrame(const SdlSurface &surf, int frame, int numFrames,
                  Sint16 px, Sint16 py);
void sdlBlitFrame(const SdlSurface &surf, int frame, int numFrames,
                  const Point &pos);

// Clear the given region of the screen.
void sdlClear(SDL_Rect region);

// Set the clipping region for the duration of a lambda or other function call.
template <typename Func>
void sdlSetClipRect(const SDL_Rect &rect, const Func &f)
{
    SDL_Rect temp;
    SDL_GetClipRect(screen, &temp);
    SDL_SetClipRect(screen, &rect);
    f();
    SDL_SetClipRect(screen, &temp);
    // TODO: this might be better as a struct so that we can restore the clip
    // rectangle if f() throws an exception.
}

// Lock an image before accessing the underlying pixels.
struct SdlLock
{
    SDL_Surface *surface_;

    template <typename Func>
    SdlLock(SDL_Surface *surf, const Func &f) : surface_{surf}
    {
        if (SDL_MUSTLOCK(surface_)) {
            if (SDL_LockSurface(surface_) == 0) {
                f();
            }
            else {
                std::cerr << "Error locking surface: " << SDL_GetError()
                    << '\n';
            }
        }
        else {
            f();
        }
    }

    ~SdlLock()
    {
        if (SDL_MUSTLOCK(surface_)) {
            SDL_UnlockSurface(surface_);
        }
    }
};

// Load a resource from disk.  Returns null on failure.
SdlSurface sdlLoadImage(const char *filename);
SdlFont sdlLoadFont(const char *filename, int ptSize);
SdlMusic sdlLoadMusic(const char *filename);
SdlMusic sdlLoadMusic(const std::string &filename);
// note: don't try to allocate these at global scope.  They need sdlInit()
// before they will work, and the objects must be freed before SDL teardown
// happens.

// Draw a dashed line to the screen starting at (px,py).
void sdlDashedLineH(Sint16 px, Sint16 py, Uint16 len, Uint32 color);
void sdlDashedLineV(Sint16 px, Sint16 py, Uint16 len, Uint32 color);

// Rectangle functions.
bool insideRect(Sint16 x, Sint16 y, const SDL_Rect &rect);
std::pair<double, double> rectPct(Sint16 x, Sint16 y, const SDL_Rect &rect);
enum class Dir8 {None = -1, N, NE, E, SE, S, SW, W, NW};
Dir8 nearEdge(Sint16 x, Sint16 y, const SDL_Rect &rect);

// Return the bounding box for the given image.
SDL_Rect sdlGetBounds(const SdlSurface &surf, Sint16 x, Sint16 y);

// Draw text to the screen.
void sdlDrawText(const SdlFont &font, const char *txt, SDL_Rect pos,
                 const SDL_Color &color);
void sdlDrawText(const SdlFont &font, const std::string &txt, SDL_Rect pos,
                 const SDL_Color &color);

// Play a music file at a reasonable volume.
void sdlPlayMusic(SdlMusic &music);

#endif
