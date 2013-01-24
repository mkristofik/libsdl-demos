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
#ifndef RANDOM_MAP_H
#define RANDOM_MAP_H

#include "HexGrid.h"
#include "SDL_types.h"
#include "Terrain.h"
#include "hex_utils.h"
#include <vector>

class RandomMap
{
public:
    RandomMap(Sint16 hWidth, Sint16 hHeight);
    void draw();

private:
    // Use a Voronoi diagram to generate a random set of regions.
    void generateRegions();

    // Compute the "center of mass" for each region.
    std::vector<Point> getHexCenters() const;

    // Construct an adjacency list for each region.
    void buildRegionGraph();

    HexGrid hgrid_;
    int numRegions_;
    std::vector<int> regions_;
    std::vector<int> terrain_;
    AdjacencyList regionGraph_;
};

#endif
