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
#include <vector>

SDL_Surface *screen = nullptr;

namespace
{
    using DashSize = std::pair<Sint16, Uint16>;  // line-relative pos, width
    std::vector<DashSize> dashedLine(Uint16 lineLen)
    {
        const Uint16 spaceSize = 6;
        const Uint16 dashSize = spaceSize * 3 / 2;

        std::vector<DashSize> dashes;
        Sint16 pos = 0;
        while (pos < lineLen) {
            if (pos + dashSize < lineLen) {
                dashes.emplace_back(pos, dashSize);
                pos += dashSize + spaceSize;
            }
            else {
                dashes.emplace_back(pos, lineLen - pos);
                break;
            }
        }

        return dashes;
    }
}

bool sdlInit(Sint16 winWidth, Sint16 winHeight, const char *iconPath,
             const char *caption)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) {
        std::cerr << "Error initializing SDL: " << SDL_GetError();
        return false;
    }
    atexit(SDL_Quit);

    if (IMG_Init(IMG_INIT_PNG) < 0) {
        std::cerr << "Error initializing SDL_image: " << IMG_GetError();
        return false;
    }
    atexit(IMG_Quit);

    if (TTF_Init() < 0) {
        std::cerr << "Error initializing SDL_ttf: " << TTF_GetError();
        return false;
    }
    atexit(TTF_Quit);

    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) < 0) {
        std::cerr << "Warning: error initializing SDL_mixer: " << Mix_GetError();
        // not a fatal error
    }
    atexit(Mix_Quit);

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        std::cerr << "Warning: error opening SDL_mixer: " << Mix_GetError();
        // not a fatal error
    }
    atexit(Mix_CloseAudio);

    // Have to do this prior to SetVideoMode.
    auto icon = make_surface(IMG_Load(iconPath));
    if (icon != nullptr) {
        SDL_WM_SetIcon(icon.get(), nullptr);
    }
    else {
        std::cerr << "Warning: error loading icon file: " << IMG_GetError();
    }

    screen = SDL_SetVideoMode(winWidth, winHeight, 0, SDL_SWSURFACE);
    if (screen == nullptr) {
        std::cerr << "Error setting video mode: " << SDL_GetError();
        return false;
    }

    SDL_WM_SetCaption(caption, "");
    return true;
}

SdlSurface make_surface(SDL_Surface *surf)
{
    return SdlSurface(surf, SDL_FreeSurface);
}

// source: SDL_CreateRGBSurface documentation.
SdlSurface sdlCreateSurface(Sint16 width, Sint16 height)
{
    // This can only be called after SDL_SetVideoMode()
    assert(screen != nullptr);

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
        std::cerr << "Warning: error drawing to screen: " << SDL_GetError()
            << '\n';
    }
}

void sdlClear(SDL_Rect region)
{
    assert(screen != nullptr);
    auto black = SDL_MapRGB(screen->format, 0, 0, 0);
    if (SDL_FillRect(screen, &region, black) < 0) {
        std::cerr << "Error clearing screen region: " << SDL_GetError() << '\n';
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

SdlFont sdlLoadFont(const char *filename, int ptSize)
{
    SdlFont font(TTF_OpenFont(filename, ptSize), TTF_CloseFont);
    if (!font) {
        std::cerr << "Error loading font " << filename << " size " << ptSize
            << "\n    " << TTF_GetError() << '\n';
    }
    return font;
}

SdlMusic sdlLoadMusic(const char *filename)
{
    SdlMusic music(Mix_LoadMUS(filename), Mix_FreeMusic);
    if (!music) {
        std::cerr << "Error loading music " << filename << "\n    "
            << Mix_GetError() << '\n';
    }
    return music;
}

void sdlDashedLineH(Sint16 px, Sint16 py, Uint16 len, Uint32 color)
{
    assert(screen != nullptr);
    const Uint16 lineWidth = 1;
    for (const auto &dash : dashedLine(len)) {
        SDL_Rect r = {Sint16(px + dash.first), py, dash.second, lineWidth};
        if (SDL_FillRect(screen, &r, color) < 0) {
            std::cerr << "Error drawing horizontal dashed line: "
                << SDL_GetError() << '\n';
            return;
        }
        // TODO: this could be a unit test.
        //std::cout << 'H' << r.x << ',' << r.y << 'x' << r.w << '\n';
    }
}

void sdlDashedLineV(Sint16 px, Sint16 py, Uint16 len, Uint32 color)
{
    assert(screen != nullptr);
    const Uint16 lineWidth = 1;
    for (const auto &dash : dashedLine(len)) {
        SDL_Rect r = {px, Sint16(py + dash.first), lineWidth, dash.second};
        if (SDL_FillRect(screen, &r, color) < 0) {
            std::cerr << "Error drawing horizontal dashed line: "
                << SDL_GetError() << '\n';
            return;
        }
        // TODO: this could be a unit test.
        //std::cout << 'V' << r.x << ',' << r.y << 'x' << r.h << '\n';
    }
}

bool insideRect(Sint16 x, Sint16 y, const SDL_Rect &rect)
{
    return x >= rect.x &&
           y >= rect.y &&
           x < rect.x + rect.w &&
           y < rect.y + rect.h;
}

std::pair<double, double> rectPct(Sint16 x, Sint16 y, const SDL_Rect &rect)
{
    return {(static_cast<double>(x) - rect.x) / (rect.w - 1),
            (static_cast<double>(y) - rect.y) / (rect.h - 1)};
}

Dir8 nearEdge(Sint16 x, Sint16 y, const SDL_Rect &rect)
{
    if (!insideRect(x, y, rect)) {
        return Dir8::None;
    }

    const Sint16 edgeDist = 10;

    if (x - rect.x < edgeDist) {  // left edge
        if (y - rect.y < edgeDist) {
            return Dir8::NW;
        }
        else if (rect.y + rect.h - 1 - y < edgeDist) {
            return Dir8::SW;
        }
        else {
            return Dir8::W;
        }
    }
    else if (rect.x + rect.w - 1 - x < edgeDist) {  // right edge
        if (y - rect.y < edgeDist) {
            return Dir8::NE;
        }
        else if (rect.y + rect.h - 1 - y < edgeDist) {
            return Dir8::SE;
        }
        else {
            return Dir8::E;
        }
    }
    else if (y - rect.y < edgeDist) {  // top edge
        return Dir8::N;
    }
    else if (rect.y + rect.h - 1 - y < edgeDist) {
        return Dir8::S;
    }

    return Dir8::None;
}

SDL_Rect sdlGetBounds(const SdlSurface &surf, Sint16 x, Sint16 y)
{
    return {x, y, static_cast<Uint16>(surf->w), static_cast<Uint16>(surf->h)};
}

void sdlDrawText(const SdlFont &font, const char *txt, Sint16 x, Sint16 y,
                 const SDL_Color &color, Justify j)
{
    SdlSurface textImg = make_surface(TTF_RenderText_Blended(font.get(), txt,
                                                             color));
    if (textImg == nullptr) {
        std::cerr << "Warning: error rendering blended text: " << TTF_GetError();
        return;
    }

    if (j == Justify::CENTER) {
        x -= textImg->w / 2;
    }
    else if (j == Justify::RIGHT) {
        x -= textImg->w;
    }

    sdlBlit(textImg, x, y);
}
