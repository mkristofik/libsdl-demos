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
}

void handleMouseUp(const SDL_MouseButtonEvent &event, Mix_Music *music)
{
    if (event.button == SDL_BUTTON_LEFT &&
        insideRect(event.x, event.y, playButton))
    {
        if (!Mix_PlayingMusic()) {  // have we started playing music at all
            Mix_PlayMusic(music, 0);
            Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
            sdlBlit(pause, playButton.x, playButton.y);
        }
        else {
            if (Mix_PausedMusic()) {
                sdlBlit(pause, playButton.x, playButton.y);
                Mix_ResumeMusic();
            }
            else {
                sdlBlit(play, playButton.x, playButton.y);
                Mix_PauseMusic();
            }
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

    auto font = sdlLoadFont("../DejaVuSans.ttf", 12);

    SDL_UpdateRect(screen, 0, 0, 0, 0);

    SdlMusic music = sdlLoadMusic("../music/wesnoth.ogg");
    // note: can't allocate this at global scope because it has to be freed
    // before closing down SDL_Mixer.

    bool isDone = false;
    SDL_Event event;
    while (!isDone) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONUP) {
                handleMouseUp(event.button, music.get());
            }
            else if (event.type == SDL_QUIT) {
                Mix_HaltMusic();
                isDone = true;
            }
        }

        SDL_UpdateRect(screen, 0, 0, 0, 0);
        SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}
