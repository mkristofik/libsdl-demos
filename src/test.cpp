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
#define BOOST_TEST_MODULE Test_Module_1
#include <boost/test/unit_test.hpp>

#include "hex_utils.h"

BOOST_AUTO_TEST_CASE(Hex_Distance)
{
    BOOST_CHECK_EQUAL(hexDist({1, 1}, {2, 2}), 1);
    BOOST_CHECK_EQUAL(hexDist({4, 4}, {3, 3}), 1);
    BOOST_CHECK_EQUAL(hexDist({1, 1}, {3, 3}), 3);
    BOOST_CHECK_EQUAL(hexDist({7, 7}, {5, 5}), 3);
}
