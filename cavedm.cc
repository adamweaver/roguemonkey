// -*- Mode: C++ -*-
// RogueMonkey (c) 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>
#include <map>

#include "boost/shared_ptr.hpp"

#include "creature.h"
#include "dice.h"
#include "dmutils.h"
#include "dungeonmaster.h"
#include "item.h"
#include "map.h"


//============================================================================
// CaveDM
//============================================================================
namespace
{    
    enum CaveType
    {
        Goblin,

        CaveTypeEnd
    };

    struct RoomData
    {
        enum Type
        {
            Entry, Guard, Barracks, Mess, Treasury, Throne, EndType
        };

        enum Size
        {
            Small, Medium, Large, EndSize
        };

        int         likelihood; // one in likelihood (1 in 4)
        bool        allowmultiple;
        Type        depends;
        Size        size;
    };

    RoomData room_data[RoomData::EndType] = 
    {
        // Entry
        { 1, false, RoomData::EndType, RoomData::Small },

        // Guardroom
        { 2, true, RoomData::EndType, RoomData::Medium },

        // Barracks
        { 4, true, RoomData::Guard, RoomData::Medium },

        // Mess
        { 8, false, RoomData::EndType, RoomData::Large },

        // Treasury
        {10, true, RoomData::Guard, RoomData::Medium },

        // Throne
        {20, false, RoomData::Guard, RoomData::Medium },
    };



    struct CellularData
    {
        char terrain;
        int perc_fill;
        int blank_if_less;
        int fill_if_more;
        int iterations;
        bool borders_filled;
    };

    CellularData cellular[CaveTypeEnd] = 
    {
        // OpenLevel
        {'#', 35, 4, 4, 10, true},
    };
}
 
class CaveDM : public DungeonMaster
{
public:
    CaveDM(std::string const & name) :
        DungeonMaster(name)
    {
    }

    virtual ~CaveDM()
    {
    }

    static DungeonMaster * create(std::string const & name)
    {
        return new CaveDM(name);
    }

    std::string describe() const
    {
        return "Cave DM";
    }

    std::string describe(CreatureH) const
    {
        return "Cave DM";
    }

    std::string describeIndef(CreatureH) const
    {
        return "a Cave DM";
    }

    std::string describeIndef() const
    {
        return "a Cave DM";
    }

    Actor::Speed getSpeed() const
    {
        return Actor::VSlow;
    }

    MapH createLevel(int /*lvl*/, int mapsz_x, int mapsz_y)
    {
        int type = Dice::Random0(CaveTypeEnd);
        DMUtils::CharVec cave_map(DMUtils::CellularAutomata(mapsz_x, mapsz_y, cellular[type].terrain, 
                                                            cellular[type].perc_fill, cellular[type].blank_if_less, 
                                                            cellular[type].fill_if_more, cellular[type].iterations, 
                                                            cellular[type].borders_filled));
        Map::TerrainMapping tmap;
        tmap.insert(Map::TerrainMapping::value_type(' ', Map::Grass));
        tmap.insert(Map::TerrainMapping::value_type('#', Map::RockWall));
        tmap.insert(Map::TerrainMapping::value_type('T', Map::Tree));

        MapH tmp(new Map(mapsz_x, mapsz_y, &cave_map[0], tmap));
        
        for (int i = 0; i < 250; ++i)
        {
            int xx = Dice::Random0(mapsz_x);
            int yy = Dice::Random0(mapsz_y);
            Coords coord(xx, yy);
            if (tmp->getTerrain(coord) == Map::Grass)
            {
                tmp->addItem(coord, Item::createItem(Dice::Random0(100) > 49 ? Item::Weapon : Item::Weapon));
            }
        }
        return tmp;
    }

};



//==========================================================================
// unnamed Factory registrator
//============================================================================
namespace
{
    struct CaveDM_Registrar
    {
        CaveDM_Registrar()
        {
            DungeonMaster::DungeonFactory &fact =
                DungeonMaster::theFactory();
            fact.registerCreator("cave", CaveDM::create);
        }

        ~CaveDM_Registrar()
        {
            DungeonMaster::DungeonFactory &fact =
                DungeonMaster::theFactory();
            fact.deregisterCreator("cave");
        }

    } registrar;
}


