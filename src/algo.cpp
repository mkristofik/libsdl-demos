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
#include "algo.h"
#include <ctime>

std::minstd_rand & randomGenerator()
{
    static std::minstd_rand gen(static_cast<unsigned int>(std::time(nullptr)));
    return gen;
}
