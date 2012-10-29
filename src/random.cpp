#include "SDL.h"
#include "SDL_image.h"

#include <iostream>
#include <memory>

typedef std::unique_ptr<SDL_Surface, void(*)(SDL_Surface *)> SdlSurface;

SdlSurface sdlLoadImage(const char *filename)
{
    SdlSurface temp(IMG_Load(filename), SDL_FreeSurface);
    if (!temp) {
        std::cerr << "Error loading image " << filename
            << "\n    " << IMG_GetError() << '\n';
        return temp;
    }
    SdlSurface surf(SDL_DisplayFormatAlpha(temp.get()), SDL_FreeSurface);
    if (!surf) {
        std::cerr << "Error converting to display format: "
            << "\n    " << IMG_GetError() << '\n';
    }

    return surf;
}

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        std::cerr << "Error initializing SDL: " << SDL_GetError();
        return EXIT_FAILURE;
    }
    atexit(SDL_Quit);

    if (IMG_Init(IMG_INIT_PNG) < 0) {
        std::cerr << "Error initializing SDL_image: " << IMG_GetError();
        return EXIT_FAILURE;
    }
    atexit(IMG_Quit);

    // Have to do this prior to SetVideoMode.
    SdlSurface icon(IMG_Load("../img/icon.png"), SDL_FreeSurface);
    if (icon != nullptr) {
        SDL_WM_SetIcon(icon.get(), nullptr);
    }
    else {
        std::cerr << "Warning: error loading icon file: " << IMG_GetError();
    }

    SDL_Surface *screen = SDL_SetVideoMode(882, 684, 0, SDL_SWSURFACE);
    if (screen == nullptr) {
        std::cerr << "Error setting video mode: " << SDL_GetError();
        return EXIT_FAILURE;    
    }
    SDL_WM_SetCaption("Random Map Test", "");

    const Sint16 hexSize = 72;
    auto grassTile = sdlLoadImage("../img/grass.png");
    SDL_Rect dest = {0, 0, 0, 0};

    // Display even-numbered columns.
    for (Sint16 x = 0; x <= 14; x += 2) {
        for (Sint16 y = 0; y <= 8; ++y) {
            dest.x = x * hexSize * 0.75;
            dest.y = y * hexSize;
            if (SDL_BlitSurface(grassTile.get(), nullptr, screen, &dest) < 0) {
                std::cerr << "Warning: error drawing grass hex to screen: " << SDL_GetError();
            }
        }
    }

    // Display odd-numbered columns.
    for (Sint16 x = 1; x <= 15; x += 2) {
        for (Sint16 y = 0; y <= 8; ++y) {
            dest.x = x * hexSize * 0.75;
            dest.y = (y * hexSize) + (hexSize * 0.5);
            if (SDL_BlitSurface(grassTile.get(), nullptr, screen, &dest) < 0) {
                std::cerr << "Warning: error drawing grass hex to screen: " << SDL_GetError();
            }
        }
    }

    SDL_UpdateRect(screen, 0, 0, 0, 0);

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

    return 0;
}
