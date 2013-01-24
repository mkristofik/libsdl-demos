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
#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>

enum Terrain {GRASS, DIRT, SAND, WATER, SWAMP, SNOW, NUM_TERRAINS};
// note: I didn't use an enum class because doing math on terrain types is very
// convenient.

// Assign terrain to each node in a graph such that no adjacent nodes have the
// same terrain.  Graph nodes are represented by integers [0,n).
using AdjacencyList = std::vector<std::vector<int>>;
std::vector<int> graphTerrain(const AdjacencyList &graph);

// Return the edge transition to draw between two tiles.  Return -1 if no edge
// should be drawn.
int getEdge(int terrainFrom, int terrainTo);

#endif
