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

GuiButton::GuiButton(Sint16 x, Sint16 y, SdlSurface img)
    : image_{img},
    displayArea_(sdlGetBounds(image_, x, y)),
    action_{[] {}}
{
    sdlBlit(image_, x, y);
}

const SDL_Rect & GuiButton::getDisplayArea() const
{
    return displayArea_;
}

void GuiButton::setImage(SdlSurface img)
{
    image_ = img;
    sdlClear(displayArea_);
    sdlBlit(image_, displayArea_.x, displayArea_.y);
}

void GuiButton::onClick(std::function<void ()> func)
{
    action_ = func;
}

void GuiButton::click()
{
    action_();
}
