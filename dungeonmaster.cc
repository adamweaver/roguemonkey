// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <cassert>

#include "dungeonmaster.h"
#include "map.h"
#include "stllike.h"

//============================================================================
// DungeonMaster
//============================================================================
DungeonMaster::DungeonMaster(std::string name) :
    Actor(),
    m_name(name),
    m_maps()
{
}


DungeonMaster::~DungeonMaster()
{
}


std::string const &
DungeonMaster::getName() const
{
    return m_name;
}


MapH
DungeonMaster::getOrCreateMap(int mp)
{
    assert(mp >= 0 && "Illegal map request submitted to getMap()");

    if (mp >= static_cast<int>(m_maps.size()))
        m_maps.resize(mp + 1);
    if (m_maps[mp].get() == 0)
        m_maps[mp] = createLevel(mp, 80, 80);

    return m_maps[mp];
}





unsigned int
DungeonMaster::act()
{
    return Actor::VSlow * 10U;
}




DungeonMaster::DungeonFactory &
DungeonMaster::theFactory()
{
    static DungeonFactory fact;
    return fact;
}

