// -*- Mode: C++ -*-
// RogueMonkey (c) 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>
#include <map>

#include "boost/shared_ptr.hpp"

#include "dice.h"
#include "dmutils.h"
#include "dungeonmaster.h"
#include "item.h"
#include "map.h"


//============================================================================
// TestDM
//============================================================================
class TestDM : public DungeonMaster
{
public:
    TestDM(std::string const & name) :
        DungeonMaster(name)
    {
    }

    virtual ~TestDM()
    {
    }

    static DungeonMaster * create(std::string const & name)
    {
        return new TestDM(name);
    }

    std::string describe() const
    {
        return "Test DM";
    }

    std::string describe(CreatureH) const
    {
        return "Test DM";
    }

    std::string describeIndef(CreatureH) const
    {
        return "a Test DM";
    }

    std::string describeIndef() const
    {
        return "a Test DM";
    }

    Actor::Speed getSpeed() const
    {
        return Actor::Fast;
    }

    MapH createLevel(int /*lvl*/, int /*x*/, int /*y*/)
    {
        int const mapsz_x = 125;
        int const mapsz_y = 125;

        DMUtils::CharVec test_map(DMUtils::CellularAutomata(mapsz_x, mapsz_y, '#', 40, 4, 4, 10, true));
        for (int y = 0; y < mapsz_y; ++y)
        {
            test_map[y * mapsz_x] = '#';
            test_map[(y + 1) * mapsz_x -1] = '#';
        }
        for (int x = 0; x < mapsz_x; ++x)
        {
            test_map[x] = '#';
            test_map[(mapsz_y - 1) * mapsz_x + x] = '#';
        }
        Map::TerrainMapping tmap;
        tmap.insert(Map::TerrainMapping::value_type(' ', Map::Grass));
        tmap.insert(Map::TerrainMapping::value_type('#', Map::RockWall));
        tmap.insert(Map::TerrainMapping::value_type('T', Map::Tree));

        MapH tmp(new Map(mapsz_x, mapsz_y, &test_map[0], tmap));

        for (int i = 0; i < 50; ++i)
        {
            int xx = Dice::Random0(mapsz_x);
            int yy = Dice::Random0(mapsz_y);
            Coords coord(xx, yy);
            if (tmp->getTerrain(coord) == Map::Grass)
            {
                for (int j = 0; j < 40; ++j)
                {
                    tmp->addItem(coord, Item::createItem(Dice::Random0(100) > 49 ?
                                                         Item::Weapon : Item::Armour));
                }
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
    struct TestDM_Registrar
    {
        TestDM_Registrar()
        {
            DungeonMaster::DungeonFactory &fact =
                DungeonMaster::theFactory();
            fact.registerCreator("test", TestDM::create);
        }

        ~TestDM_Registrar()
        {
            DungeonMaster::DungeonFactory &fact =
                DungeonMaster::theFactory();
            fact.deregisterCreator("test");
        }

    } registrar;
}


