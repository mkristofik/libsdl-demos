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
#include "RandomMap.h"
#include "sdl_helper.h"

namespace {
    // Load the tiles in the same order as Terrain enum.
    std::vector<SdlSurface> tiles = {sdlLoadImage("../img/grass.png"),
                                     sdlLoadImage("../img/dirt.png"),
                                     sdlLoadImage("../img/desert.png"),
                                     sdlLoadImage("../img/water.png"),
                                     sdlLoadImage("../img/swamp.png"),
                                     sdlLoadImage("../img/snow.png")};

    // Load edge transitions in same order, 6 per terrain type.
    std::vector<SdlSurface> edges = {sdlLoadImage("../img/grass-n.png"),
                                     sdlLoadImage("../img/grass-ne.png"),
                                     sdlLoadImage("../img/grass-se.png"),
                                     sdlLoadImage("../img/grass-s.png"),
                                     sdlLoadImage("../img/grass-sw.png"),
                                     sdlLoadImage("../img/grass-nw.png"),
                                     sdlLoadImage("../img/dirt-n.png"),
                                     sdlLoadImage("../img/dirt-ne.png"),
                                     sdlLoadImage("../img/dirt-se.png"),
                                     sdlLoadImage("../img/dirt-s.png"),
                                     sdlLoadImage("../img/dirt-sw.png"),
                                     sdlLoadImage("../img/dirt-nw.png"),
                                     sdlLoadImage("../img/beach-n.png"),
                                     sdlLoadImage("../img/beach-ne.png"),
                                     sdlLoadImage("../img/beach-se.png"),
                                     sdlLoadImage("../img/beach-s.png"),
                                     sdlLoadImage("../img/beach-sw.png"),
                                     sdlLoadImage("../img/beach-nw.png")};
}

RandomMap::RandomMap(Sint16 hWidth, Sint16 hHeight)
    : hgrid_(hWidth, hHeight),
    numRegions_(18),
    regions_(),
    terrain_(),
    regionGraph_()
{
    generateRegions();
}

void RandomMap::draw()
{
}

void RandomMap::generateRegions()
{
}

std::vector<Point> RandomMap::getHexCenters() const
{
    return {};
}

void RandomMap::buildRegionGraph()
{
}
