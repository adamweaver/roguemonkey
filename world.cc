// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <cassert>
#include <limits>

#include "boost/scoped_ptr.hpp"

#include "display.h"
#include "dungeonmaster.h"
#include "hero.h"
#include "inputdef.h"
#include "map.h"
#include "textutils.h"
#include "world.h"

namespace
{
    boost::scoped_ptr<World> worldsingleton(0);
}


//=========================================================================
// World
//=========================================================================
World &
World::TheWorld()
{
    assert(worldsingleton.get() && "TheWorld() requested before initialisation");
    return *worldsingleton;
}


void
World::Init()
{
    assert(!worldsingleton.get() && "Attempted to initialise World twice");
    worldsingleton.reset(new World);



}


World::World() :
    m_dms(),
    m_maps_in_play()
{
    DungeonMasterH overworld = createDM("cave", "cave");
    addMapToCurrentList(overworld->getOrCreateMap(0));
}



DungeonMasterH
World::createDM(std::string const & type, std::string const & name)
{
    DungeonMasterH dm = DungeonMaster::theFactory().create(type, name);
    m_dms[name] = dm;
    return dm;
}



void
World::mainLoop(HeroH hero)
{
    getDMByName("cave")->getOrCreateMap(0)->addCreature(Map::Default, hero);

    // Now start the main game logic
    while(!m_maps_in_play.empty())
    {
        // Process new turn. Find next Actor by high watermark
        unsigned int next_action = std::numeric_limits<unsigned int>::max();
        ActorH next_actor;

        for(MapsInPlay::iterator it = m_maps_in_play.begin(); it != m_maps_in_play.end(); ++it)
        {
            if ((*it)->getNextActor()->getTurn() < next_action)
            {
                next_actor = (*it)->getNextActor();
                next_action = next_actor->getTurn();
            }
        }
        next_action = next_actor->act();
        assert(next_actor->getCoords().M().get() && "Invalid Map in Coords for Actor!");
        next_actor->getCoords().M()->updateActor(next_actor, next_action);
    }
    return;
}


DungeonMasterH
World::getDMByName(std::string const & name) const
{
    DMs::const_iterator it = m_dms.find(name);
    if (it != m_dms.end())
    {
        return it->second;
    }
    assert(!"getDMByName failed - does not exist");
    return DungeonMasterH();
}


void
World::addMapToCurrentList(MapH mp)
{
    m_maps_in_play.insert(mp);
}



void
World::removeMapFromCurrentList(MapH mp)
{
    m_maps_in_play.erase(mp);
}





