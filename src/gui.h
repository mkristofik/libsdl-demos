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
#ifndef GUI_H
#define GUI_H

#include "sdl_helper.h"
#include <functional>

// Set of classes for GUI objects.

class GuiButton
{
public:
    GuiButton(Sint16 x, Sint16 y, SdlSurface img);

    const SDL_Rect & getDisplayArea() const;
    void setImage(SdlSurface img);
    void onClick(std::function<void ()> func);

    void click();

private:
    SdlSurface image_;
    SDL_Rect displayArea_;
    std::function<void ()> action_;
};

#endif
