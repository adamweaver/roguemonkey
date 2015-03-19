// -*- Mode: C++ -*-
// RogueMonkey Copyringt 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>

#include "boost/test/minimal.hpp"

#include "netstring.h"

int test_main(int argc, char **argv)
{
    std::string s1 = "", s2 = "";
    int i1 = 0;
    double d1 = 0.0;

    s1 = "First netstring";
    NetString ns;
    ns << s1;
    ns >> s2;
    BOOST_CHECK(s1 == s2 && "First NetString");

    ns.clear();
    s1 = "Embedded, ,, Commas,";
    ns << s1;
    ns >> s2;
    BOOST_CHECK(s1 == s2 && "Embedded Commas");

    ns.clear();
    s1 = "String then Int";
    ns << s1 << 213;
    ns >> s2 >> i1;
    BOOST_CHECK(s1 == s2 && i1 == 213 && "String then Int");

    ns.clear();
    s1 = "String, Int, Double";
    ns << s1 << 32767 << 1.125;
    ns >> s2 >> i1 >> d1;
    BOOST_CHECK(s1 == s2 && i1 == 32767 && d1 == 1.125 && "String, Int, Double");

    // Check for null string
    ns.clear();
    s1 = "";
    ns << s1;
    ns >> s2;
    BOOST_CHECK(s1 == s2 && "Empty String");

    if (argc == 1)
        return 0;

    ns.clear();
    for (int i = 0; i < argc; ++i)
    {
        ns << argv[i];
    }
    for (int i = 0; i < argc; ++i)
    {
        ns >> s1;
        BOOST_CHECK(s1 == argv[i] && "Argv[i]");
    }

    return 0;
}
