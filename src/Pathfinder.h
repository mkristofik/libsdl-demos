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
#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <functional>
#include <vector>

// Generic implementation of the A* algorithm.  Suitable for any map or graph
// whose nodes can be represented by integers.
class Pathfinder
{
public:
    Pathfinder();

    // (REQUIRED) Define a function to return a list of nodes adjacent to a
    // given node.
    // std::vector<int> (int n) -> list of neighbors of n.
    void setNeighbors(std::function<std::vector<int> (int)> func);

    // (REQUIRED) Set the goal node, or describe the goal with a function.
    // bool (int n) -> return true if n is the goal.
    void setGoal(int targetNode);
    void setGoal(std::function<bool (int)> func);

    // (OPTIONAL) Define the cost of moving from one node to another.  Step
    // cost must not be negative.  If all moves are of equal cost, then you
    // don't need to provide this function.
    // int (int a, int b) -> step cost from node a to node b.
    void setStepCost(std::function<int (int, int)> func);

    // (OPTIONAL) Define a lower-bound estimate for the cost needed to reach
    // the goal from a given node.  Bad paths can result if the estimate is too
    // high.  This is a performance optimization for when you know where the
    // goal is.
    // int (int a) -> estimate shortest path from node a to goal.
    void setEstimate(std::function<int (int)> func);

    // Return the shortest path to the goal from the starting node.  Return an
    // empty list if the goal cannot be found.
    std::vector<int> getPathFrom(int start) const;

private:
    std::function<std::vector<int> (int)> neighbors_;
    std::function<bool (int)> goal_;
    std::function<int (int, int)> stepCost_;
    std::function<int (int)> estimate_;
};

#endif
