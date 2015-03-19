// -*- Mode: C++ -*-
// RogueMonkey copyright 2075 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#ifndef H_MAP_
#define H_MAP_ 1

#include <cassert>
#include <list>
#include <map>
#include <vector>

#include "boost/enable_shared_from_this.hpp"
#include "boost/noncopyable.hpp"

#include "handles.h"
#include "inputdef.h"



class Map : public boost::enable_shared_from_this<Map>,
            private boost::noncopyable
{
public:
    enum Terrain
    {
        DirtFloor, Grass, RockWall, Tree, 

        ShallowWater, DeepWater, Mountain, Desert,

        EndTerrain
    };

    enum Position
    {
        Default, Random,
        StairUp1, StairUp2, StairUp3, StairUp4,
        StairDown1, Stairdown2, StairDown3, StairDown4
    };

    enum Lighting
    {
        Dark, Lit
    };

    typedef std::vector<char> HeroSeen;
    typedef std::vector<char> TerrainTemplate;
    typedef std::map<char, Terrain> TerrainMapping;

    /**
     * Creates a blank default map
     *
     * @param x      Size of map in x-coordinate
     * @param y      Size of map in y-coordinate
     * @param t      Default Terrain::Type to fill
     */
    Map(int x, int y, Terrain t);

    /**
     *
     * Creates a new map using templates
     *
     * @param x      width of map
     * @param y      height of map
     * @param tmplt  template to copy
     * @param tmk    Terrain key (i.e. '.' => Grass)
     */
    Map(int x, int y, char const *tmplt, TerrainMapping const & tmk);

    /**
     * Get Terrain at coordinate
     *
     * @param c      Coords struct of coordinates
     * @return       Terrain type at position
     */
    Terrain getTerrain(Coords c) const;

    /**
     * Get the name of the Terrain at coordinate
     *
     * @param c      Coords struct of coordinates
     * @return       name of terrain at position
     */
    std::string const & getTerrainName(Coords c) const;

    /**
     * Set the terrain at coordinate
     *
     * @param c      coordinates
     * @param t      type of terrain
     */
    void setTerrain(Coords c, Terrain t);

    /**
     * Gets the creature inhabiting position
     *
     * @param c      coordinates
     * @return       Creature or CreatureH() if none exists
     */
    CreatureH getCreature(Coords c) const;

    /**
     * Get ItemPile at location
     *
     * @param  c     coordinates
     * @return       ItemPile or ItemPileH() if empty
     */
    ItemPileH getItemPile(Coords c) const;

    /**
     * Add an Item to pile at location
     *
     * @param  c     coordinates
     * @param  item  item to add
     * @return       ItemPile containing item
     */
    ItemPileH addItem(Coords c, ItemH item);

    /**
     * Add a whole ItemPile to location
     *
     * @param  c     coordinates
     * @param  itemp ItemPile to add
     * @return       ImtemPile containing put pile
     */
    ItemPileH addItemPile(Coords c, ItemPileH itemp);

    /**
     * Remove an Item from pile
     *
     * @param  c     coordinates
     * @param  item  item to remove
     * @return       ItemPile remaining
     */
    ItemPileH delItem(Coords c, ItemH item);

    /**
     * Remove the whole pile
     * @param  c     coordinates
     * @return       ItemPile removed
     */
    ItemPileH delItemPile(Coords c);

    /**
     * Get size of map
     *
     * @return       coords representing size of map
     */
    Coords getSize() const;

    /**
     * Add a Creature to map
     *
     * @param  c    coordinates
     * @param  cr   Creature to add
     */
    void addCreature(Coords c, CreatureH cr);

    /**
     * Add a Creature to a pre-determined position
     *
     * @param pos   Position to add to (or near-enough)
     * @param cr    Creature to add
     */
    void addCreature(Position pos, CreatureH cr);

    /**
     * Remove a creature from map
     *
     * @param  c    coordinates
     * @return      Creature removed
     */
    CreatureH delCreature(Coords c);

    /**
     * Move a Creature from current location to new
     *
     * @param  c    new coordinates
     * @param  cr   creature to move
     */
    void moveCreature(Coords c, CreatureH cr);

    /**
     * Can a creature pass through space
     *
     * @param  c    coordinates
     * @param  cr   Creature to try
     * @return      true if can pass through
     */
    bool isPassable(Coords c, CreatureH cr) const;

    /**
     * Can a creature see through space?
     *
     * @param  c    coordinates
     * @param  cr   creature to try
     * @return      true if can see through
     */
    bool blocksVision(Coords c, CreatureH cr) const;

    /**
     * Get display Representation of space
     *
     * @param c     coordinates
     * @param cr    creature to try
     * @return      display Representation of space
     */
    Representation getRepresentation(Coords c, CreatureH cr);

    /**
     * Returns Manhattan Distance (square blocks) from a to b
     *
     * @param  a    first coordinates
     * @param  b    second coordinates
     * @return      square-block distance
     */
    static int ManhattanDistance(Coords a, Coords b);

    /**
     * Returns typical roguelike distance (diagonal and orthagonal are equivalent)
     * @param  a   first coordinate
     * @param  b   second coordinate
     * @return     distance
     */
    static int RoguelikeDistance(Coords a, Coords b);

    /**
     * Returns vector of coordinates from A to B in a straight line
     *
     * @param  a    first coordinates
     * @param  b    second coordinates
     * @return      vector of coordinates (including origin)
     */
    static std::vector<Coords> TraceLineFromAtoB(Coords a, Coords b);

    /**
     * Get the Actor next allowed to act()
     *
     * @return      next actor in line
     */
    ActorH getNextActor() const;

    /**
     * Update actor time
     *
     * @param act    actor to update
     * @param nt     number of turns to update by. 0 is remove
     */
    void updateActor(ActorH act, unsigned int nt);

    /**
     * Find a path from start to end using creature mobility
     * @param s      beginning
     * @param e      end position
     * @param cr     creature to pathfind
     * @return       vector of coordinates to follow
     */
    std::vector<Coords> pathFind(Coords s, Coords e, CreatureH cr) const;
    

    void clearSeenGrid();

    void setSeenGrid(int x, int y, Lighting lit);

    Lighting getSeenGrid(int x, int y);

    HeroSeen & getHeroSeenMap();
    void setHeroSeenChar(int x, int y);
    char getHeroSeenChar(int x, int y) const;

private:
    bool insideBoundaries(Coords c) const;
    unsigned long xyToHash(Coords c) const;
    unsigned long xyToHash(int x, int y) const;
    Coords hashToXY(unsigned long hash) const;
    void setTerrain(int x, int y, Terrain t);
    ItemPileH addItem(int x, int y,  ItemH item);
    void addCreature(int x, int y, CreatureH cr);
    Representation getTerrainRep(int x, int y) const;

    typedef std::vector<Lighting> SeenGrid;
    typedef std::vector<Map::Terrain> Grid;
    typedef std::map<unsigned long, CreatureH> Creatures;
    typedef std::map<unsigned long, ItemPileH> ItemPiles;
    typedef std::list<ActorH> Actors;

    Actors m_actors;
    Grid m_grid;
    SeenGrid m_seengrid;
    HeroSeen m_heroseen;
    Creatures m_creatures;
    mutable ItemPiles m_itempiles;

    int m_xsize;
    int m_ysize;

};




#endif

