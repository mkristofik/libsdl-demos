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
#include "Terrain.h"
#include <bitset>
#include <cassert>

int getEdge(int terrainFrom, int terrainTo)
{
    if ((terrainFrom == WATER && terrainTo != WATER) ||
        (terrainFrom != WATER && terrainTo == WATER) ||
        (terrainFrom == SAND && terrainTo != SAND) ||
        (terrainFrom != SAND && terrainTo == SAND)) {
        return SAND;
    }
    else if ((terrainFrom == DIRT && terrainTo == GRASS) ||
             (terrainFrom == GRASS && terrainTo == DIRT)) {
        return GRASS;
    }
    else if (terrainFrom != terrainTo) {
        return DIRT;
    }

    return -1;
}

std::vector<int> graphTerrain(const AdjacencyList &graph)
{
    auto size = graph.size();
    std::vector<int> terrain(size, -1);

    // Greedy coloring.  Each node tries to get a different terrain from its
    // neighbors.
    for (auto node = 0u; node < size; ++node) {
        // For each neighbor, save which terrains have been assigned.
        std::bitset<NUM_TERRAINS> assignedTerrains;
        for (auto neighbor : graph[node]) {
            assert(neighbor >= 0 && unsigned(neighbor) < size);
            if (terrain[neighbor] > -1) {
                assignedTerrains[terrain[neighbor]] = true;
            }
        }
        if (!assignedTerrains.all()) {
            // Use the lowest numbered terrain available.
            for (int t = 0; t < NUM_TERRAINS; ++t) {
                if (!assignedTerrains[t]) {
                    terrain[node] = t;
                    break;
                }
            }
        }
        else {
            terrain[node] = 0;
        }
    }

    return terrain;
}
