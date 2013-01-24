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
#define BOOST_TEST_MODULE Terrain_Test
#include <boost/test/unit_test.hpp>

#include "terrain.h"

// Check that we always draw an edge between two different terrains and that we
// never draw an edge between two terrains that are the same.
BOOST_AUTO_TEST_CASE(Edges)
{
    for (int i = 0; i < NUM_TERRAINS; ++i) {
        for (int j = 0; j < NUM_TERRAINS; ++j) {
            if (i == j) {
                BOOST_CHECK_EQUAL(getEdge(i, j), -1);
            }
            else {
                auto edge = getEdge(i, j);
                BOOST_CHECK_GE(edge, 0);
                BOOST_CHECK_LT(edge, NUM_TERRAINS);
            }
        }
    }
}
