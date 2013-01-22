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
#define BOOST_TEST_MODULE Hex_Utils_Test
#include <boost/test/unit_test.hpp>

#include "hex_utils.h"

BOOST_AUTO_TEST_CASE(Distance)
{
    BOOST_CHECK_EQUAL(hexDist({1, 1}, {2, 2}), 1);
    BOOST_CHECK_EQUAL(hexDist({4, 4}, {3, 3}), 1);
    BOOST_CHECK_EQUAL(hexDist({1, 1}, {3, 3}), 3);
    BOOST_CHECK_EQUAL(hexDist({7, 7}, {5, 5}), 3);
}

BOOST_AUTO_TEST_CASE(Array_Index_To_Hex)
{
    HexGrid grid(16, 9);
    
    // Generate some random hexes and convert back and forth between the array
    // index and 2D representations.
    for (int i = 0; i < 10; ++i) {
        auto hex = grid.hexRandom();
        auto a = grid.aryFromHex(hex);
        auto aryN = grid.aryNeighbors(a);
        auto hexN = grid.hexNeighbors(hex);
        BOOST_CHECK_EQUAL(aryN.size(), hexN.size());

        for (unsigned int i = 0; i < hexN.size(); ++i) {
            BOOST_CHECK_EQUAL(grid.aryFromHex(hexN[i]), aryN[i]);
        }
    }
}

template <class Container, class T>
bool contains(const Container& c, const T& elem)
{
    return find(std::begin(c), std::end(c), elem) != std::end(c);
}

BOOST_AUTO_TEST_CASE(Neighbors)
{
    HexGrid grid(16, 9);

    auto hn = grid.hexNeighbors({0, 0});
    BOOST_CHECK(contains(hn, Point{1, 0}));
    BOOST_CHECK(contains(hn, Point{0, 1}));

    hn = grid.hexNeighbors({2, 4});
    BOOST_CHECK(contains(hn, Point{2, 3}));
    BOOST_CHECK(contains(hn, Point{2, 5}));
    BOOST_CHECK(contains(hn, Point{1, 3}));
    BOOST_CHECK(contains(hn, Point{1, 4}));
    BOOST_CHECK(contains(hn, Point{3, 3}));
    BOOST_CHECK(contains(hn, Point{3, 4}));

    hn = grid.hexNeighbors({7, 5});
    BOOST_CHECK(contains(hn, Point{7, 4}));
    BOOST_CHECK(contains(hn, Point{7, 6}));
    BOOST_CHECK(contains(hn, Point{6, 5}));
    BOOST_CHECK(contains(hn, Point{6, 6}));
    BOOST_CHECK(contains(hn, Point{8, 5}));
    BOOST_CHECK(contains(hn, Point{8, 6}));

    hn = grid.hexNeighbors({15, 8});
    BOOST_CHECK(contains(hn, Point{15, 7}));
    BOOST_CHECK(contains(hn, Point{14, 8}));
}

BOOST_AUTO_TEST_CASE(Hex_Get_Neighbor)
{
    HexGrid grid(16, 9);

    Point h1{4, 5};
    int a1 = grid.aryFromHex(h1);
    for (auto d : Dir()) {
        BOOST_CHECK_EQUAL(str(grid.hexGetNeighbor(h1, d)),
                          str(grid.hexFromAry(grid.aryGetNeighbor(a1, d))));
    }

    Point h2{7, 6};
    int a2 = grid.aryFromHex(h2);
    for (auto d : Dir()) {
        BOOST_CHECK_EQUAL(str(grid.hexGetNeighbor(h2, d)),
                          str(grid.hexFromAry(grid.aryGetNeighbor(a2, d))));
    }
}
