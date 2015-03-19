// -*- Mode: C++ -*-
// RogueMonkey (c) 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <cmath>
#include <string>

#include "dice.h"
#include "dmutils.h"
#include "dungeonmaster.h"
#include "item.h"
#include "map.h"


//============================================================================
// Overworld
//============================================================================

class Overworld : public DungeonMaster
{
public:
    typedef std::vector<short> Vertices;

    static int const mapsz_x = 256;
    static int const mapsz_y = 256;
    static int const roughness = 3;
    static int const begin_rand = 256;

    static int const deep_water = -20;
    static int const shallow_water = 0;
    static int const plain = 20;
    static int const forest = 30;
              

    Overworld(std::string const & name) :
        DungeonMaster(name)
    {
    }

    virtual ~Overworld()
    {
    }

    static DungeonMaster * create(std::string const & name)
    {
        return new Overworld(name);
    }

    std::string describe() const
    {
        return "Overworld";
    }

    std::string describe(CreatureH) const
    {
        return "Overworld";
    }

    std::string describeIndef(CreatureH) const
    {
        return "an Overworld";
    }

    std::string describeIndef() const
    {
        return "an Overworld";
    }

    Actor::Speed getSpeed() const
    {
        return Actor::Fast;
    }

    MapH createLevel(int /*lvl*/, int /*x*/, int /*y*/)
    {
        Vertices heightmap((mapsz_x + 1) * (mapsz_y + 1), 0);
        int rnd = begin_rand;

        for (int step = mapsz_x; step > 1; step = (step + 1) / 2 - 1)
        {
            for (int y = 0; y < mapsz_y - 2; y += step)
            {
                for (int x = 0; x < mapsz_x - 2; x += step)
                {
                    GenMidpoints(heightmap, x, y, step, rnd);
                }
            }
            rnd = static_cast<int>(rnd * std::pow(2.0, -roughness));
        }

        std::vector<char> cmap(mapsz_x * mapsz_y, ' ');
        for (int y = 1; y < mapsz_y - 1; ++y)
        {
            for (int x = 1; x < mapsz_x - 1; ++x)
            {
                int avg = Avg4(heightmap, x, y, 1);
                cmap[XY(x - 1, y - 1)] = 
                    (avg < deep_water) ? '~' : 
                    (avg < shallow_water) ? '`' :
                    (avg < plain) ? ' ' :
                    (avg < forest) ? '&' :  
                    '^';
            }
        }
                 

        Map::TerrainMapping tmap;
        tmap.insert(Map::TerrainMapping::value_type(' ', Map::Grass));
        tmap.insert(Map::TerrainMapping::value_type('~', Map::DeepWater));
        tmap.insert(Map::TerrainMapping::value_type('`', Map::ShallowWater));
        tmap.insert(Map::TerrainMapping::value_type('&', Map::Tree));
        tmap.insert(Map::TerrainMapping::value_type('^', Map::Mountain));

        MapH tmp(new Map(mapsz_x, mapsz_y, &cmap[0], tmap));
        return tmp;
    }

    /*
     *   A--B--C--D--E       *--B--*--D--*       
     *   |  |  |  |  |       |  |  |  |  |
     *   F--G--H--I--J       F--G--H--I--J
     *   |  |  |  |  |       |  |  |  |  |
     *   K--L--M--N--O       *--L--*--M--*
     *   |  |  |  |  |       |  |  |  |  |
     *   P--Q--R--S--T       O--P--Q--R--S
     *   |  |  |  |  |       |  |  |  |  |
     *   U--V--W--X--Y       *--V--*--X--*
     */

    // calculate M using (A + E + U + Y) / 4 + random(-rnd, rnd)
    // calculate C using (A + E) / 2
    static void GenMidpoints(Vertices & v, int tlx, int tly, int step, int rnd)
    {
        // M, C, W, K, O
        v[XY(tlx + step/2, tly + step/2)] = Avg4(v, tlx, tly, step) + Dice::Random0(2 * rnd) - rnd;
        v[XY(tlx + step/2, tly)] = (v[XY(tlx, tly)] + v[XY(tlx + step, tly)]) / 2;
        v[XY(tlx + step/2, tly + step)] = (v[XY(tlx, tly + step)] + v[XY(tlx + step, tly + step)]) / 2;
        v[XY(tlx, tly + step/2)] = (v[XY(tlx, tly)] + v[XY(tlx, tly + step)]) / 2;
        v[XY(tlx + step, tly + step/2)] = (v[XY(tlx + step, tly)] + v[XY(tlx + step, tly + step)]) / 2;
    }

    static int XY(int x, int y)
    {
        return y * mapsz_x + x;
    }

    static int Avg4(Vertices & v, int tlx, int tly, int step)
    {
        return (v[XY(tlx, tly)] + v[XY(tlx + step, tly)] + 
                v[XY(tlx, tly + step)] + v[XY(tlx + step, tly + step)]) / 4;
    }

};



//==========================================================================
// unnamed Factory registrator
//============================================================================
namespace
{
    struct Overworld_Registrar
    {
        Overworld_Registrar()
        {
            DungeonMaster::DungeonFactory &fact =
                DungeonMaster::theFactory();
            fact.registerCreator("overworld", Overworld::create);
        }

        ~Overworld_Registrar()
        {
            DungeonMaster::DungeonFactory &fact =
                DungeonMaster::theFactory();
            fact.deregisterCreator("overworld");
        }

    } registrar;
}


