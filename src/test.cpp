#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include <cstdlib>
#include <iostream>
#include <memory>

typedef std::unique_ptr<SDL_Surface, void(*)(SDL_Surface *)> SdlSurface;
typedef std::unique_ptr<TTF_Font, void(*)(TTF_Font *)> SdlFont;

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

SdlFont sdlLoadFont(const char *filename, int ptsize)
{
    SdlFont font(TTF_OpenFont(filename, ptsize), TTF_CloseFont);
    if (!font) {
        std::cerr << "Error loading font " << filename << " size " << ptsize
            << "\n    " << TTF_GetError() << '\n';
    }
    return font;
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

    if (TTF_Init() < 0) {
        std::cerr << "Error initializing SDL_ttf: " << TTF_GetError();
        return EXIT_FAILURE;
    }
    atexit(TTF_Quit);

    // Have to do this prior to SetVideoMode.
    SdlSurface icon(IMG_Load("../img/icon.png"), SDL_FreeSurface);
    if (icon != nullptr) {
        SDL_WM_SetIcon(icon.get(), nullptr);
    }
    else {
        std::cerr << "Warning: error loading icon file: " << IMG_GetError();
    }

    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
    if (screen == nullptr) {
        std::cerr << "Error setting video mode: " << SDL_GetError();
        return EXIT_FAILURE;    
    }
    SDL_WM_SetCaption("Title Bar", "Doesn't seem to matter on Win7");

    auto picture = sdlLoadImage("../img/avatar.png");
    SDL_Rect dest = {300, 200, 0, 0};
    if (SDL_BlitSurface(picture.get(), nullptr, screen, &dest) < 0) {
        std::cerr << "Warning: error drawing picture to screen: " << SDL_GetError();
    }

    auto font = sdlLoadFont("../DejaVuSans.ttf", 12);
    if (!font) {
        return EXIT_FAILURE;
    }
    int hint = TTF_GetFontHinting(font.get());
    std::cout << "Curious about font hinting, we have: " << hint << '\n';
    SDL_Surface *text = TTF_RenderText_Blended(font.get(), "Hello, world!", SDL_Color({255, 255, 255, 0}));
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
    text = TTF_RenderText_Shaded(font.get(), "Hello, world!", SDL_Color({255, 255, 255, 0}), SDL_Color({0, 0, 0, 0}));
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
    text = TTF_RenderText_Solid(font.get(), "Hello, world!", SDL_Color({255, 255, 255, 0}));
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

    SDL_FreeSurface(text);
    return EXIT_SUCCESS;
}
