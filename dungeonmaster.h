#ifndef H_DUNGEONMASTER_
#define H_DUNGEONMASTER_ 1

// -*- Mode: C++ -*-
// RogueMonkey Copyringt 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "actor.h"
#include "error.h"
#include "factory.h"
#include "handles.h"


/**
  * DungeonMaster represents each dungeon or area. Each DM may own
  * multiple levels or maps. A DM is also an actor (to create monsters,
  * treasure, direct overall strategy, etc) at regular intervals.
  */
class DungeonMaster : public Actor
{
public:
    virtual ~DungeonMaster() = 0;

    virtual Gender getGender() const { return Neuter; }

    /**
     * Get the Map for this DM or create if does not exist
     *
     * @param lvl          level of DM to return
     * @return             appropriate map
     */
    MapH getOrCreateMap(int lvl);

    /**
     * Get the DungeonMaster's name
     *
     * @return             name of dm type
     */
    std::string const & getName() const;



    // From Actor
    virtual unsigned int act();
    virtual int heroGUID() const { return 0; }
    //virtual void notifyEvent(ActorH acted, Events::Type event);


    //===============================================================
    typedef DungeonMaster * (*DC) (std::string const & name);
    typedef Factory<DungeonMaster *,  std::string,  DC,  DungeonMasterH>
        DungeonFactory;

    /**
     * Factory method for pluggable DMs
     *
     * @return Factory method
     */
    static DungeonFactory & theFactory();


protected:
    DungeonMaster(std::string name);

private:
    /**
     * Create a map level
     *
     * @param lvl        level of dungeon (sic) to create
     * @param x          width of level
     * @param y          height of level
     */
    virtual MapH createLevel(int lvl, int x, int y) = 0;




    typedef std::vector<MapH> MapList;

    std::string    m_name;
    MapList        m_maps;
};


inline bool operator<(DungeonMasterH l, DungeonMasterH r)
{
    return l->getName() < r->getName();
}


#endif

