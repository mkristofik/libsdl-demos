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
#include "gui.h"
#include "sdl_helper.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#include "boost/filesystem.hpp"

struct MusicInfo
{
    std::string path;
    SdlMusic music;
};

void handleMouseUp(const SDL_MouseButtonEvent &event,
                   std::vector<GuiButton *> buttons)
{
    if (event.button != SDL_BUTTON_LEFT) {
        return;
    }

    for (auto &b : buttons) {
        if (insideRect(event.x, event.y, b->getDisplayArea())) {
            b->click();
        }
    }
}

std::vector<MusicInfo> getMusicFiles(const char *dir)
{
    namespace bfs = boost::filesystem;

    bfs::path p(dir);
    if (!exists(p) || !is_directory(p)) {
        return {};
    }

    std::vector<MusicInfo> files;
    for (bfs::directory_iterator i(p); i != bfs::directory_iterator(); ++i) {
        bfs::path rawPath = i->path();
        std::string path = rawPath.make_preferred().string();
        files.emplace_back(MusicInfo{path, sdlLoadMusic(path)});
    }

    return files;
}

extern "C" int SDL_main(int, char **)  // 2-arg form is required by SDL
{
    if (!sdlInit(250, 140, "../img/icon.png", "Music Test")) {
        return EXIT_FAILURE;
    }

    SdlSurface play = sdlLoadImage("../img/button-play.png");
    SdlSurface pause = sdlLoadImage("../img/button-pause.png");
    SdlSurface next = sdlLoadImage("../img/button-next.png");
    SdlSurface prev = sdlLoadImage("../img/button-prev.png");

    // Define the control buttons.
    GuiButton playButton{105, 90, play};
    GuiButton nextTrack{155, 90, next};
    GuiButton prevTrack{55, 90, prev};
    std::vector<GuiButton *> buttons = {&playButton, &nextTrack, &prevTrack};

    auto font = sdlLoadFont("../DejaVuSans.ttf", 12);
    auto white = SDL_Color{255, 255, 255, 0};
    sdlDrawText(font, "Now playing:", SDL_Rect{10, 10, 230, 20}, white);

    auto trackTitle = SDL_Rect{10, 30, 230, 20};
    sdlDrawText(font, "Nothing", trackTitle, white);

    SDL_UpdateRect(screen, 0, 0, 0, 0);

    auto musicFiles = getMusicFiles("../music");
    assert(!musicFiles.empty());

    int trackNum = 0;

    playButton.onClick([&] {
        if (!Mix_PlayingMusic()) {  // have we started playing music at all
            auto &track = musicFiles[trackNum];
            sdlPlayMusic(track.music);
            playButton.setImage(pause);
            sdlDrawText(font, track.path, trackTitle, white);
        }
        else {
            if (Mix_PausedMusic()) {
                playButton.setImage(pause);
                Mix_ResumeMusic();
            }
            else {
                playButton.setImage(play);
                Mix_PauseMusic();
            }
        }
    });

    nextTrack.onClick([&] {
        if (!Mix_PlayingMusic()) return;

        trackNum = (trackNum + 1) % musicFiles.size();
        auto &track = musicFiles[trackNum];
        sdlDrawText(font, track.path, trackTitle, white);
        if (Mix_PausedMusic()) {
            sdlPlayMusic(track.music);
            Mix_PauseMusic();
        }
        else {
            sdlPlayMusic(track.music);
        }
    });

    prevTrack.onClick([&] {
        if (!Mix_PlayingMusic()) return;

        trackNum = (trackNum - 1) % musicFiles.size();
        auto &track = musicFiles[trackNum];
        sdlDrawText(font, track.path, trackTitle, white);
        if (Mix_PausedMusic()) {
            sdlPlayMusic(track.music);
            Mix_PauseMusic();
        }
        else {
            sdlPlayMusic(track.music);
        }
    });

    bool isDone = false;
    SDL_Event event;
    while (!isDone) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONUP) {
                handleMouseUp(event.button, buttons);
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
