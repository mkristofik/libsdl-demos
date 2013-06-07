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
#include <iostream>

namespace
{
    SdlSurface play;
    SdlSurface pause;
    SdlSurface next;
    SdlSurface prev;
    SDL_Rect playButton;
    SDL_Rect nextTrack;
    SDL_Rect prevTrack;
    bool isPlaying = false;
}

void handleMouseUp(const SDL_MouseButtonEvent &event)
{
    if (event.button == SDL_BUTTON_LEFT &&
        insideRect(event.x, event.y, playButton))
    {
        if (isPlaying) {
            sdlBlit(play, playButton.x, playButton.y);
            isPlaying = false;
        }
        else {
            sdlBlit(pause, playButton.x, playButton.y);
            isPlaying = true;
        }

    }
}

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (!sdlInit(320, 180, "../img/icon.png", "Music Test")) {
        return EXIT_FAILURE;
    }

    play = sdlLoadImage("../img/button-play.png");
    pause = sdlLoadImage("../img/button-pause.png");
    next = sdlLoadImage("../img/button-next.png");
    prev = sdlLoadImage("../img/button-prev.png");

    // Define the control buttons.
    playButton = sdlGetBounds(play, 140, 130);
    sdlBlit(play, playButton.x, playButton.y);
    nextTrack = sdlGetBounds(next, 190, 130);
    sdlBlit(next, nextTrack.x, nextTrack.y);
    prevTrack = sdlGetBounds(prev, 90, 130);
    sdlBlit(prev, prevTrack.x, prevTrack.y);

    // Draw the slider bar and current track position.
    auto gray = SDL_MapRGB(screen->format, 195, 195, 195);
    auto trackbar = SDL_Rect{15, 113, 290, 3};
    SDL_FillRect(screen, &trackbar, gray);
    auto slider = sdlLoadImage("../img/button-slider.png");
    sdlBlit(slider, 10, 108);

    auto font = sdlLoadFont("../DejaVuSans.ttf", 12);
    auto white = SDL_Color{255, 255, 255};
    sdlDrawText(font, "0:00 / 0:00", 310, 130, white, Justify::RIGHT);

    SDL_UpdateRect(screen, 0, 0, 0, 0);

    bool isDone = false;
    SDL_Event event;
    while (!isDone) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONUP) {
                handleMouseUp(event.button);
            }
            else if (event.type == SDL_QUIT) {
                isDone = true;
            }
        }

        SDL_UpdateRect(screen, 0, 0, 0, 0);
        SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}
