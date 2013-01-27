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
#include "hex_utils.h"
#include "terrain.h"
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
    void recalcHexCenters();

    // Construct an adjacency list for each region.
    void buildRegionGraph();

    // Assign terrain types to each hex.
    void assignTerrain();

    HexGrid mgrid_;
    int numRegions_;
    std::vector<int> regions_;  // assign each tile to a region [0,numRegions)
    std::vector<Point> centers_;  // center hex of each region
    AdjacencyList regionGraph_;

    // To help make the edges of the map look nice, we extend the grid by one
    // hex in every direction.
    HexGrid tgrid_;
    std::vector<int> terrain_;
};

#endif
