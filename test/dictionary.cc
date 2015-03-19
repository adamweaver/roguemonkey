// -*- Mode: C++ -*-
// RogueMonkey Copyringt 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>
#include <utility>

#include "boost/test/minimal.hpp"

#include "dictionary.h"


int test_main(int, char **)
{
    typedef Dictionary<int, int> SimpleDict;

    std::pair<int, int> load[] =
    {
        std::make_pair(1, 1), std::make_pair(10, 10), std::make_pair(5, 5),
        std::make_pair(4, 4), std::make_pair(3, 3), std::make_pair(2, 2),
        std::make_pair(12, 12)
    };

    SimpleDict loadtest(&load[0], &load[0] + 7);
    BOOST_CHECK(loadtest.size() == 7);

    int res = 0;
    for (SimpleDict::iterator i = loadtest.begin(); i != loadtest.end(); ++i)
    {
        BOOST_CHECK(i->first == i->second);
        BOOST_CHECK(i->second > res);
        res = i->second;
    }






    return 0;
}
