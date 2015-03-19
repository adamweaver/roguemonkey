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
// TownDM
//============================================================================
class TownDM : public DungeonMaster
{
public:
    TownDM(std::string const & name) :
        DungeonMaster(name)
    {
    }

    virtual ~TownDM()
    {
    }

    static DungeonMaster * create(std::string const & name)
    {
        return new TownDM(name);
    }

    std::string describe() const
    {
        return "Town DM";
    }

    std::string describe(CreatureH) const
    {
        return "Town DM";
    }

    std::string describeIndef(CreatureH) const
    {
        return "a Town DM";
    }

    std::string describeIndef() const
    {
        return "a Town DM";
    }

    Actor::Speed getSpeed() const
    {
        return Actor::Fast;
    }

    MapH createLevel(int /*lvl*/, int mapsz_x, int mapsz_y)
    {
        DMUtils::CharVec town_map(mapsz_x * mapsz_y, ' ');

        for (int i = 0; i < 50; ++i)
        {
            int w = Dice::Random0(8) + 2;
            int h = Dice::Random0(8) + 2;
            int x = Dice::Random0(mapsz_x - w);
            int y = Dice::Random0(mapsz_y - h);

            for (int yy = 0; yy < h; ++yy)
                for (int xx = 0; xx < w; ++xx)
                    town_map[(yy + y) * mapsz_x + xx + x] = '#';
        }

        Map::TerrainMapping tmap;
        tmap.insert(Map::TerrainMapping::value_type(' ', Map::Grass));
        tmap.insert(Map::TerrainMapping::value_type('#', Map::RockWall));
        tmap.insert(Map::TerrainMapping::value_type('T', Map::Tree));

        MapH tmp(new Map(mapsz_x, mapsz_y, &town_map[0], tmap));

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

        for (int i = 0; i < 25; ++i)
        {
            int xx = Dice::Random0(mapsz_x);
            int yy = Dice::Random0(mapsz_y);
            Coords coord(xx, yy);
            if (tmp->getTerrain(coord) == Map::Grass)
            {
                CreatureH cr = Creature::create(Creature::Monster, Species::Human);
                tmp->addCreature(Map::Default, cr);
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
    struct TownDM_Registrar
    {
        TownDM_Registrar()
        {
            DungeonMaster::DungeonFactory &fact =
                DungeonMaster::theFactory();
            fact.registerCreator("town", TownDM::create);
        }

        ~TownDM_Registrar()
        {
            DungeonMaster::DungeonFactory &fact =
                DungeonMaster::theFactory();
            fact.deregisterCreator("town");
        }

    } registrar;
}


