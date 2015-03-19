#ifndef H_WORLD_
#define H_WORLD_  1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <map>
#include <set>
#include <utility>

#include "boost/shared_ptr.hpp"
#include "boost/noncopyable.hpp"

#include "dungeonmaster.h"
#include "handles.h"



/**
 * World owns all DungeonMasters, and maintains lists of current Actors.
 * Takes responsibility for making all Actors act()
 */
class World : private boost::noncopyable
{
public:    
    /**
     * Singleton of the World structure
     *
     * @return        world singleton
     */
    static World & TheWorld();

    /**
     * Initialise the World structure
     */
    static void Init();

    /**
     * Commence game
     */
    void mainLoop(HeroH hero);

    /**
     * Return the DungeonMaster by name
     *
     * @param name     name of relevant DM
     * @return         DungeonMaster
     */
    DungeonMasterH getDMByName(std::string const & name) const;

    /**
     * Add a map to the currently used rotation
     *
     * @param mp       map to add
     */
    void addMapToCurrentList(MapH mp);

    /**
     * Remove a map from the currently used rotation
     *
     * @param mp       map to remove
     */
    void removeMapFromCurrentList(MapH mp);

private:
    typedef std::map<std::string, DungeonMasterH> DMs;
    typedef std::set<MapH>                        MapsInPlay;
    typedef std::set<HeroH>                       HeroesInPlay;

    World();
    DungeonMasterH createDM(std::string const & type, std::string const & name);

    DMs                   m_dms;
    MapsInPlay            m_maps_in_play;
};





#endif

