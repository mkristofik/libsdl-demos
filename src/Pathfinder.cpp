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
#include "Pathfinder.h"
#include <algorithm>
#include <memory>
#include <unordered_map>

#include <iostream> // XXX

struct PathNode
{
    int prev;
    int costSoFar;
    int estTotalCost;
    bool visited;
};

typedef std::shared_ptr<PathNode> PathNodePtr;

namespace {
    PathNodePtr make_node(int prev, int costSoFar, int estTotalCost)
    {
        bool visited = false;
        return std::make_shared<PathNode>(PathNode{prev, costSoFar,
                                                   estTotalCost, visited});
    }
}

Pathfinder::Pathfinder()
    : neighbors_([] (int) { return std::vector<int>(); }),
    goal_([] (int) { return false; }),
    stepCost_([] (int, int) { return 1; }),
    estimate_([] (int) { return 0; })
{
}

void Pathfinder::setNeighbors(std::function<std::vector<int> (int)> func)
{
    neighbors_ = func;
}

void Pathfinder::setGoal(int targetNode)
{
    goal_ = [=] (int node) { return node == targetNode; };
}

void Pathfinder::setGoal(std::function<bool (int)> func)
{
    goal_ = func;
}

void Pathfinder::setStepCost(std::function<int (int, int)> func)
{
    stepCost_ = func;
}

void Pathfinder::setEstimate(std::function<int (int)> func)
{
    estimate_ = func;
}

std::vector<int> Pathfinder::getPathFrom(int start) const
{
    if (goal_(start)) return {start};

    // Record shortest path costs for every node we examine.
    std::unordered_map<int, PathNodePtr> nodes;
    // Maintain a heap of nodes to consider.
    std::vector<int> open;
    int goalLoc = -1;
    PathNodePtr goalNode;

    // The heap functions confusingly use operator< to build a heap with the
    // *largest* element on top.  We want to get the node with the *least* cost,
    // so we have to order nodes in the opposite way.
    auto orderByCost = [&] (int lhs, int rhs)
    {
        return nodes[lhs]->estTotalCost > nodes[rhs]->estTotalCost;
    };

    nodes.emplace(start, make_node(-1, 0, 0));
    open.push_back(start);

    // A* algorithm.  Decays to Dijkstra's if estimate function is always 0.
    while (!open.empty()) {
        auto loc = open.front();
        pop_heap(std::begin(open), std::end(open), orderByCost);
        open.pop_back();
        std::cout << "Considering node " << loc << " cost " << nodes[loc]->estTotalCost << std::endl; // XXX
        if (goal_(loc)) {
            goalLoc = loc;
            goalNode = nodes[loc];
            std::cout << "Path found!" << std::endl;  // XXX
            break;
        }

        auto &curNode = nodes[loc];
        curNode->visited = true;
        for (auto n : neighbors_(loc)) {
            std::cout << "Looking at neighbor " << n << "..."; // XXX
            auto nIter = nodes.find(n);
            auto step = stepCost_(loc, n);

            if (nIter != nodes.end()) {
                auto &nNode = nIter->second;
                if (nNode->visited) {
                    std::cout << "Was visited, ignoring" << std::endl;  // XXX
                    continue;
                }

                // Are we on a shorter path to the neighbor node than what
                // we've already seen?  If so, update the neighbor's node data.
                if (curNode->costSoFar + step < nNode->costSoFar) {
                    nNode->prev = loc;
                    nNode->costSoFar = curNode->costSoFar + step;
                    nNode->estTotalCost = nNode->costSoFar + estimate_(n);
                    make_heap(std::begin(open), std::end(open), orderByCost);
                    std::cout << "Got better path cost so far " << nNode->costSoFar << std::endl;  // XXX
                }
                else {
                    std::cout << "Worse path, do nothing" << std::endl;  // XXX
                }
            }
            else {
                // We haven't seen this node before.  Add it to the open list.
                nodes.emplace(n, make_node(loc, curNode->costSoFar + step, 
                    curNode->costSoFar + step + estimate_(n)));
                open.push_back(n);
                push_heap(std::begin(open), std::end(open), orderByCost);
                std::cout << "New node cost " << nodes[n]->estTotalCost << std::endl;  // XXX
            }
        }
    }

    if (!goalNode) {
        std::cout << "Failed!" << std::endl;
        return {};
    }

    // Build the path from the chain of nodes leading to the goal.
    std::vector<int> path = {goalLoc};
    auto n = goalNode;
    while (n->prev != -1) {
        path.push_back(n->prev);
        n = nodes[n->prev];
    }
    reverse(std::begin(path), std::end(path));
    return path;
}
