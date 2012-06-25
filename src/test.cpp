#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include <cstdlib>
#include <iostream>

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

    if (TTF_Init() < 0) {
        std::cerr << "Error initializing SDL_ttf: " << TTF_GetError();
        return EXIT_FAILURE;
    }
    atexit(TTF_Quit);

    // Have to do this prior to SetVideoMode.
    SDL_Surface *icon = IMG_Load("../icon.png");
    if (icon == nullptr) {
        std::cerr << "Warning: error loading icon file: " << IMG_GetError();
    }
    else {
        SDL_WM_SetIcon(icon, nullptr);
    }

    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
    if (screen == nullptr) {
        std::cerr << "Error setting video mode: " << SDL_GetError();
        return EXIT_FAILURE;    
    }
    SDL_WM_SetCaption("Title Bar", "Doesn't seem to matter on Win7");

    SDL_Surface *temp = IMG_Load("../avatar.png");
    if (temp == nullptr) {
        std::cerr << "Error loading image from disk: " << IMG_GetError();
        return EXIT_FAILURE;
    }
    SDL_Surface *picture = SDL_DisplayFormatAlpha(temp);
    SDL_FreeSurface(temp);
    if (picture == nullptr) {
        std::cerr << "Error converting to display format: " << IMG_GetError();
        return EXIT_FAILURE;
    }
    SDL_Rect dest = {300, 200, 0, 0};
    if (SDL_BlitSurface(picture, nullptr, screen, &dest) < 0) {
        std::cerr << "Warning: error drawing picture to screen: " << SDL_GetError();
    }

    TTF_Font *font = TTF_OpenFont("../DejaVuSans.ttf", 12);
    if (font == nullptr) {
        std::cerr << "Error loading font: " << TTF_GetError();
        SDL_FreeSurface(picture);
        return EXIT_FAILURE;
    }
    int hint = TTF_GetFontHinting(font);
    std::cout << "Curious about font hinting, we have: " << hint << '\n';
    SDL_Surface *text = TTF_RenderText_Blended(font, "Hello, world!", SDL_Color({255, 255, 255, 0}));
    if (text == nullptr) {
        std::cerr << "Warning: error rendering blended text: " << TTF_GetError();
    }
    else {
        dest = {285, 260, 0, 0};
        if (SDL_BlitSurface(text, nullptr, screen, &dest) < 0) {
            std::cerr << "Warning: error drawing blended text to screen: " << SDL_GetError();
        }
        SDL_FreeSurface(text);
    }
    text = TTF_RenderText_Shaded(font, "Hello, world!", SDL_Color({255, 255, 255, 0}), SDL_Color({0, 0, 0, 0}));
    if (text == nullptr) {
        std::cerr << "Warning: error rendering shaded text: " << TTF_GetError();
    }
    else {
        dest = {285, 280, 0, 0};
        if (SDL_BlitSurface(text, nullptr, screen, &dest) < 0) {
            std::cerr << "Warning: error drawing shaded text to screen: " << SDL_GetError();
        }
        SDL_FreeSurface(text);
    }
    text = TTF_RenderText_Solid(font, "Hello, world!", SDL_Color({255, 255, 255, 0}));
    if (text == nullptr) {
        std::cerr << "Warning: error rendering solid text: " << TTF_GetError();
    }
    else {
        dest = {285, 300, 0, 0};
        if (SDL_BlitSurface(text, nullptr, screen, &dest) < 0) {
            std::cerr << "Warning: error drawing solid text to screen: " << SDL_GetError();
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

    SDL_FreeSurface(picture);
    SDL_FreeSurface(text);
    TTF_CloseFont(font);
    return EXIT_SUCCESS;
}
