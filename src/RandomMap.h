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
#include "hex_utils.h"
#include "sdl_helper.h"
#include "terrain.h"
#include <vector>

class RandomMap
{
public:
    // Create a map and define the visible portion on the screen.  Minimum size
    // is 2x1.
    RandomMap(Sint16 hWidth, Sint16 hHeight, const SDL_Rect &pDisplayArea);

    // Size of the entire map in pixels.
    Sint16 pWidth() const;
    Sint16 pHeight() const;

    // Size of the visible map area only in pixels.
    const SDL_Rect & getDisplayArea() const;

    // Draw the map with the given map coordinates in the upper-left corner.
    // We can draw anywhere between (0,0) and maxPixel() and still keep the
    // display area filled.
    Point maxPixel() const;
    void draw(Sint16 mpx, Sint16 mpy);
    void redraw();  // use last draw position

    // Return the last draw() target.
    Point mDrawnAt() const;

    // Return the hex currently drawn at the given pixel.
    Point getHexAtS(Sint16 spx, Sint16 spy) const;
    Point getHexAtS(const Point &sp) const;
    Point getHexAtM(Sint16 mpx, Sint16 mpy) const;
    Point getHexAtM(const Point &mp) const;

    // Return the screen coordinates of the given hex.
    Point sPixelFromHex(Sint16 hx, Sint16 hy) const;
    Point sPixelFromHex(const Point &hex) const;

    // Get the terrain type at the given map coordinates.
    int getTerrainAt(Sint16 mpx, Sint16 mpy) const;

    // Highlight the given hex.
    void selectHex(const Point &hex);
    Point getSelectedHex() const;

    void highlightPath(const Point &hSrc, const Point &hDest);

    // Return true if the given hex doesn't have an obstacle.
    bool walkable(const Point &hex) const;

private:
    // Use a Voronoi diagram to generate a random set of regions.
    void generateRegions();
    void recalcHexCenters();

    // Construct an adjacency list for each region.
    void buildRegionGraph();

    void generateObstacles();
    void assignTerrain();
    void setObstacleImages();
    void drawTile(Sint16 hx, Sint16 hy);
    void drawObstacle(Sint16 hx, Sint16 hy);

    // Ensure all walkable hexes in each region are reachable from every other
    // walkable hex.
    void makeWalkable();
    void makeRegionWalkable(std::vector<int> &hexes, std::vector<char> &visited);

    // The terrain grid extends from (-1,-1) to (hWidth,hHeight) inclusive on
    // the main grid.  These conversions let us always refer to the map in main
    // grid coordinates.  Return -1 if the result is outside the terrain grid.
    int tIndex(int mIndex) const;
    int tIndex(const Point &mHex) const;
    int tIndex(Sint16 hx, Sint16 hy) const;

    // Convert between screen coordinates and map coordinates.
    Point mPixel(const Point &sp) const;
    Point mPixel(Sint16 spx, Sint16 spy) const;
    Point sPixel(const Point &mp) const;
    Point sPixel(Sint16 mpx, Sint16 mpy) const;
    Point sPixel(int mIndex) const;

    bool walkable(int mIndex) const;

    // Find shortest number of hops between regions.  Intended as a high-level
    // first pass at generating paths between distant hexes.
    std::vector<int> getRegionPath(int rBegin, int rEnd) const;

    HexGrid mgrid_;
    Sint16 pWidth_;
    Sint16 pHeight_;
    int numRegions_;
    std::vector<int> regions_;  // assign each tile to a region [0,numRegions)
    std::vector<Point> centers_;  // center hex of each region
    AdjacencyList regionGraph_;
    AdjacencyList regionGraphWalk_;  // walkable paths to adjacent regions

    // To help make the edges of the map look nice, we extend the grid by one
    // hex in every direction.
    HexGrid tgrid_;
    std::vector<int> terrain_;
    std::vector<char> tObst_;  // 1=obstacle present, 0=none

    struct Obstacle
    {
        Sint16 pxOffset;  // handle images not sized exactly to one hex
        Sint16 pyOffset;
        SdlSurface img;
        
        Obstacle() : pxOffset(0), pyOffset(0), img() {}
    };
    std::vector<Obstacle> tObstImg_;  // which obstacle graphics to use

    // Visible portion of the map.  Max pixel is defined so that the display
    // area is always filled.
    SDL_Rect pDisplayArea_;
    Sint16 mMaxX_;
    Sint16 mMaxY_;

    // Current upper-left pixel in map coordinates.
    Sint16 px_;
    Sint16 py_;

    Point selectedHex_;
    std::vector<int> selectedPath_;
};

#endif
