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

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (!sdlInit(320, 180, "../img/icon.png", "Music Test")) {
        return EXIT_FAILURE;
    }

    bool isDone = false;
    SDL_Event event;
    while (!isDone) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isDone = true;
            }
        }

        SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}
