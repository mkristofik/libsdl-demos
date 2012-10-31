#include "SDL.h"
#include "SDL_image.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

typedef std::shared_ptr<SDL_Surface> SdlSurface;

const Sint16 hexSize = 72;
const Sint16 mapWidth = 16;
const Sint16 mapHeight = 9;
int terrain[] = {0,0,0,0,0,0,0,0,0,0,2,2,2,2,3,3,
                 4,0,0,0,0,0,0,0,0,0,2,2,2,2,3,3,
                 4,4,0,0,0,0,0,0,0,0,2,2,2,2,3,3,
                 4,4,4,0,0,0,0,0,0,1,1,2,2,2,3,3,
                 4,4,4,0,0,0,0,0,1,1,1,1,1,1,0,0,
                 4,4,4,0,0,0,0,1,1,1,1,1,1,1,0,0,
                 5,5,5,0,0,0,1,1,1,1,1,1,1,1,0,0,
                 5,5,5,5,1,1,1,1,1,1,1,1,1,1,0,0,
                 5,5,5,5,1,1,1,1,1,1,1,1,1,1,0,0};

SDL_Surface *screen = nullptr;

void sdlBlit(const SdlSurface &surf, Sint16 x, Sint16 y)
{
    assert(screen != nullptr);
    SDL_Rect dest = {x, y, 0, 0};
    if (SDL_BlitSurface(surf.get(), nullptr, screen, &dest) < 0) {
        std::cerr << "Warning: error drawing to screen: " << SDL_GetError();
    }
}

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

    screen = SDL_SetVideoMode(882, 684, 0, SDL_SWSURFACE);
    if (screen == nullptr) {
        std::cerr << "Error setting video mode: " << SDL_GetError();
        return EXIT_FAILURE;    
    }
    SDL_WM_SetCaption("Random Map Test", "");

    std::vector<SdlSurface> tiles;
    tiles.emplace_back(sdlLoadImage("../img/grass.png"));
    tiles.emplace_back(sdlLoadImage("../img/dirt.png"));
    tiles.emplace_back(sdlLoadImage("../img/swamp2.png"));
    tiles.emplace_back(sdlLoadImage("../img/snow.png"));
    tiles.emplace_back(sdlLoadImage("../img/desert.png"));
    tiles.emplace_back(sdlLoadImage("../img/water.png"));

    // Display even-numbered columns.
    for (Sint16 x = 0; x < mapWidth; x += 2) {
        for (Sint16 y = 0; y < mapHeight; ++y) {
            auto terrainIndex = terrain[y * mapWidth + x];
            sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, y * hexSize);
        }
    }

    // Display odd-numbered columns.
    for (Sint16 x = 1; x < mapWidth; x += 2) {
        for (Sint16 y = 0; y < mapHeight; ++y) {
            auto terrainIndex = terrain[y * mapWidth + x];
            sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, (y+0.5) * hexSize);
        }
    }

    // Overdraw so we don't get jagged edges.
    for (Sint16 y = -1; y < mapHeight; ++y) {  // left edge, x = -1
        int terrainX = 0;
        int terrainY = std::min(y + 1, mapHeight - 1);
        auto terrainIndex = terrain[terrainY * mapWidth + terrainX];
        sdlBlit(tiles[terrainIndex], -0.75 * hexSize, (y+0.5) * hexSize);
    }
    for (Sint16 x = 1; x < mapWidth; x += 2) {  // top edge, y = -1
        int terrainX = x;
        int terrainY = 0;
        auto terrainIndex = terrain[terrainY * mapWidth + terrainX];
        sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, -0.5 * hexSize);
    }
    for (Sint16 y = 0; y < mapHeight + 1; ++y) {  // right edge, x = mapWidth
        int terrainX = mapWidth - 1;
        int terrainY = std::min<int>(y, mapHeight - 1);
        auto terrainIndex = terrain[terrainY * mapWidth + terrainX];
        sdlBlit(tiles[terrainIndex], mapWidth * hexSize * 0.75, y * hexSize);
    }
    for (Sint16 x = 0; x < mapWidth; x += 2) {  // bottom edge, y = mapHeight
        int terrainX = x;
        int terrainY = mapHeight - 1;
        auto terrainIndex = terrain[terrainY * mapWidth + terrainX];
        sdlBlit(tiles[terrainIndex], x * hexSize * 0.75, mapHeight * hexSize);
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
