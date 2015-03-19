// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <set>
#include <stack>
#include <utility>
#include <vector>

#include "boost/multi_index_container.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index/identity.hpp"
#include "boost/multi_index/member.hpp"

#include "creature.h"
#include "dice.h"
#include "item.h"
#include "map.h"

//============================================================================
// Map::Terrain data
//============================================================================
namespace
{
    enum PassableOrNot
    {
        Passable, Impassable
    };

    struct TerrainInfo
    {
        std::string      name;
        Representation   representation;
        PassableOrNot    passable;
    };


    TerrainInfo TerrainI[Map::EndTerrain] =
    {
        // DirtFloor
        {"some dirt", Representation('.', Colour::Brown), Passable},

        // Grass
        {"some grass", Representation('.', Colour::DGreen), Passable},

        // RockWall
        {"a rock wall", Representation('#', Colour::Brown), Impassable},

        // Tree
        {"a tree", Representation('&', Colour::LGreen), Impassable},
        
        // ShallowWater
        {"some water", Representation('~', Colour::LBlue), Impassable},

        // DeepWater 
        {"some water", Representation('~', Colour::DBlue), Impassable},

        // Mountain
        {"a mountain", Representation('^', Colour::Brown), Impassable},

        // Desert
        {"desert", Representation('.', Colour::Yellow), Passable}

    };
}



//============================================================================
// Map
//============================================================================
Map::Map(int x, int y, Map::Terrain t) :
    m_actors(),
    m_grid(y * x, t),
    m_seengrid(y * x, Dark),
    m_heroseen(y * x, 0),
    m_creatures(),
    m_itempiles(),
    m_xsize(x),
    m_ysize(y)
{
}

Map::Map(int x, int y, char const *tmplt, Map::TerrainMapping const & tmk) :
    m_actors(),
    m_grid(y * x),
    m_seengrid(y * x, Dark),
    m_heroseen(y * x, 0),
    m_creatures(),
    m_itempiles(),
    m_xsize(x),
    m_ysize(y)
{
    std::vector<Terrain> terrain(96, Grass);
    for (TerrainMapping::const_iterator it = tmk.begin(); it != tmk.end(); ++it)
    {
        assert(it->first >= 32 && it->first < 127);
        terrain[it->first - 32] = it->second;
    };

    for (int i = 0; i < x*y; ++i)
        setTerrain(i % x, i / x, terrain[tmplt[i] - 32]);
}


ActorH
Map::getNextActor() const
{
    assert(!m_actors.empty() && "No Actor to fetch - getNextActor()");
    return m_actors.front();
}


void
Map::updateActor(ActorH act, unsigned int nt)
{
    assert(act->getCoords().M().get() == this && "updateActor() called for non-matching Map");
    // linear find() is ok, because Actor will usually be at the front
    Actors::iterator it = std::find(m_actors.begin(), m_actors.end(), act);
    if (it != m_actors.end())
        m_actors.erase(it);
    act->m_turn += nt;
    m_actors.push_back(act);
    m_actors.sort(Actor::ActorComp());
}


Map::Terrain
Map::getTerrain(Coords c) const
{
    assert(c.X() >= 0 && c.Y() >= 0 && c.X() < m_xsize && c.Y() < m_ysize);
    return m_grid[c.y * m_xsize + c.x];
}


std::string const &
Map::getTerrainName(Coords c) const
{
    return TerrainI[getTerrain(c)].name;
}



void
Map::setTerrain(Coords c, Map::Terrain t)
{
    assert(insideBoundaries(c));
    setTerrain(c.x, c.y, t);
}


void
Map::setTerrain(int x,  int y,  Map::Terrain t)
{
    m_grid.at(y * m_xsize + x) = t;
}


Representation
Map::getTerrainRep(int x, int y) const
{
    return TerrainI[m_grid[y * m_xsize + x]].representation;
}


CreatureH
Map::getCreature(Coords c) const
{
    assert(insideBoundaries(c));
    Creatures::const_iterator i = m_creatures.find(xyToHash(c));
    return i == m_creatures.end() ? CreatureH() : i->second;
}


void
Map::addCreature(Coords c, CreatureH creature)
{
    assert(insideBoundaries(c));
    Map *oldmap = creature->getCoords().M().get();
    if (oldmap && oldmap != this)
        oldmap->delCreature(creature->getCoords());
    addCreature(c.x, c.y, creature);
}


void
Map::addCreature(int x, int y, CreatureH creature)
{
    m_creatures.insert(Creatures::value_type(xyToHash(x, y), creature));
    m_actors.push_back(creature);
    m_actors.sort(Actor::ActorComp());
    creature->m_coords = Coords(x,  y);
    creature->m_coords.setMap(shared_from_this());
}


void
Map::addCreature(Position /*pos*/, CreatureH creature)
{
    // TODO: map-default positions
    for (bool ok = false; !ok; )
    {
        Coords coord(Dice::Random0(m_xsize), Dice::Random0(m_ysize));
        if (isPassable(coord, creature))
        {
            addCreature(coord, creature);
            ok = true;
        }
    }
}


CreatureH
Map::delCreature(Coords c)
{
    assert(c.M().get() == this && "delCreature() called for non-matching Map");
    assert(insideBoundaries(c));
    Creatures::iterator it = m_creatures.find(xyToHash(c));
    assert(it != m_creatures.end());
    CreatureH critter(it->second);
    m_creatures.erase(it);
    m_actors.remove(critter);
    return critter;
}


void
Map::moveCreature(Coords c, CreatureH cr)
{
    assert(c.M().get() == this && "moveCreature() called for non-matching Map");
    assert(insideBoundaries(c));
    Coords coords(cr->getCoords());
    m_creatures.erase(xyToHash(coords));
    m_creatures.insert(Creatures::value_type(xyToHash(c), cr));
    cr->m_coords.x = c.x;
    cr->m_coords.y = c.y;
}


bool
Map::insideBoundaries(Coords c) const
{
    return c.X() >= 0 && c.Y() >= 0 && c.X() < m_xsize && c.Y() < m_ysize;
}

Coords
Map::getSize() const
{
    return Coords(m_xsize, m_ysize);
}




ItemPileH
Map::getItemPile(Coords c) const
{
    assert(insideBoundaries(c));
    unsigned long hash = xyToHash(c);
    ItemPiles::const_iterator i = m_itempiles.find(hash);
    if (i != m_itempiles.end())
        return i->second;
    ItemPileH tmp(new ItemPile(52));
    return m_itempiles.insert(ItemPiles::value_type(hash, tmp)).first->second;
}



ItemPileH
Map::addItem(Coords c, ItemH item)
{
    assert(insideBoundaries(c));
    return addItem(c.x, c.y, item);
}


ItemPileH
Map::addItem(int x, int y, ItemH item)
{
    unsigned long hash = xyToHash(x, y);
    ItemPiles::iterator it = m_itempiles.find(hash);
    if (it == m_itempiles.end())
    {
        ItemPileH tmp(new ItemPile(52));
        tmp->addItemToPile(item);
        return m_itempiles.insert(
            ItemPiles::value_type(hash, tmp)).first->second;
    }
    it->second->addItemToPile(item);
    return it->second;
}


ItemPileH
Map::addItemPile(Coords c, ItemPileH itemp)
{
    assert(insideBoundaries(c));
    unsigned long hash = xyToHash(c);
    ItemPiles::iterator it = m_itempiles.find(hash);
    if (it == m_itempiles.end())
        return m_itempiles.insert(ItemPiles::value_type(hash, itemp)).first->second;
    TransferAllItems(itemp, it->second);
    return it->second;
}



ItemPileH
Map::delItem(Coords c, ItemH item)
{
    assert(insideBoundaries(c));
    unsigned long hash = xyToHash(c);
    ItemPiles::iterator it = m_itempiles.find(hash);
    if (it != m_itempiles.end())
        it->second->delItem(item);
    return it->second;
}


ItemPileH
Map::delItemPile(Coords c)
{
    assert(insideBoundaries(c));
    unsigned long hash = xyToHash(c);
    ItemPileH tmp = m_itempiles[hash];
    m_itempiles.erase(hash);
    return tmp;
}



bool
Map::isPassable(Coords c, CreatureH /*cr*/) const
{
    if (c.X() < 0 || c.X() >= m_xsize || c.Y() < 0 || c.Y() >= m_ysize)
        return false;
    return TerrainI[getTerrain(c)].passable == Passable;
}


bool
Map::blocksVision(Coords c, CreatureH /*cr*/) const
{
    assert(insideBoundaries(c));
    return !TerrainI[getTerrain(c)].passable;
}


Representation
Map::getRepresentation(Coords c, CreatureH cr)
{
    assert(insideBoundaries(c));
    Representation rep(' ', Colour::White);
    CreatureH critter = getCreature(c);
    ItemPileH ip = getItemPile(c);

    if (critter.get())
        rep = critter->getRepresentation(cr);
    else if (ip.get() && !ip->empty())
        rep = ip->front()->getRepresentation();
    else
        rep = TerrainI[getTerrain(c)].representation;
    return rep;

}


void
Map::clearSeenGrid()
{
    SeenGrid(m_xsize * m_ysize, Dark).swap(m_seengrid);
}


void
Map::setSeenGrid(int x, int y, Map::Lighting l)
{
    m_seengrid[y * m_xsize + x] = l;
}



Map::Lighting
Map::getSeenGrid(int x, int y)
{
    return m_seengrid[y * m_xsize + x];
}



void
Map::setHeroSeenChar(int x, int y)
{
    m_heroseen[y * m_xsize + x] = getTerrainRep(x, y).first;
}


char
Map::getHeroSeenChar(int x, int y) const
{
    return m_heroseen[y * m_xsize + x];
}


Map::HeroSeen &
Map::getHeroSeenMap()
{
    return m_heroseen;
}


unsigned long
Map::xyToHash(Coords c) const
{
    return (static_cast<unsigned long>(c.x) << 16) | c.y;
}


unsigned long
Map::xyToHash(int x,  int y) const
{
    return (static_cast<unsigned long>(x) << 16) | y;
}


Coords
Map::hashToXY(unsigned long hash) const
{
    return Coords(hash >> 16, hash & 0xFFFF);
}


int
Map::ManhattanDistance(Coords a, Coords b)
{
    return std::max(std::abs(a.x - b.x), std::abs(a.y- b.y));
}


int 
Map::RoguelikeDistance(Coords a, Coords b)
{
    return std::min(std::abs(a.x - b.x), std::abs(a.y - b.y));
}


//! Bresenham line algorithm
//  from http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
std::vector<Coords>
Map::TraceLineFromAtoB(Coords a, Coords b)
{
    std::vector<Coords> out;

    bool steep = std::abs(b.y - a.y) > std::abs(b.x - a.x);
    if (steep == true)
    {
        std::swap(a.x,  a.y);
        std::swap(b.x,  b.y);
    }
    int const deltax = std::abs(b.x - a.x);
    int const deltay = std::abs(b.y - a.y);
    int error = 0;
    int deltaerr = deltay;
    Coords curpos(a.x, a.y);
    int xstep = (a.x < b.x) ? 1 : -1;
    int ystep = (a.y < b.y) ? 1 : -1;

    out.push_back((steep) ? Coords(curpos.y,  curpos.x) : curpos);

    while (curpos.x != b.x)
    {
        curpos.x += xstep;
        error += deltaerr;
        if (error * 2 >= deltax)
        {
            curpos.y += ystep;
            error -= deltax;
        }
        out.push_back((steep) ? Coords(curpos.y,  curpos.x) : curpos);
    }
    return out;
}

//namespace
//{
    Coords Offsets[8] = { Coords(-1, -1), Coords(0, -1), Coords(1, -1),
                          Coords(-1, 0),                 Coords(1, 0),
                          Coords(-1, 1),  Coords(0, 1),  Coords(1, 1) }; 

    struct Node;
    typedef std::set<Node>         ClosedList;
    typedef ClosedList::iterator   ClosedIter;
    struct Node
    {
        Coords      coord;      // position
        ClosedIter  parent;     // parent of this position
        int         cost;       // cost to get here
        int         total_cost; // cost to get here plus estimated cost to target
                                // 
        Node(Coords here, ClosedIter p, int cost_here, int total) :
            coord(here), parent(p), cost(cost_here), total_cost(total) {}
        bool operator< (Node const & rh) const
        { return coord < rh.coord; }
    };

    using namespace boost::multi_index;
    typedef multi_index_container<
                                  Node, 
                                  indexed_by<
                                             ordered_unique<identity<Node> >, 
                                             ordered_non_unique<member<Node, int, &Node::total_cost> >
                                            >
                                 > OpenList;

    
    void AddOrUpdateOpenList(OpenList & open_list, Node & node)
    {
        // strictly the get<0>() isn't required as multi_index is derived from index<0>
        OpenList::iterator it = open_list.get<0>().find(node);
        if (it != open_list.get<0>().end())
        {
            if (it->cost > node.cost)
            {
                open_list.get<0>().replace(it, node);
            }
            return;
        }
        open_list.insert(node);

    }


    std::vector<Coords> const & CalculatePath(Coords final, ClosedIter parent, ClosedIter end, std::vector<Coords> & path)
    {
        std::stack<Coords> c_list;
        c_list.push(final);
        for ( ; parent != end && parent->parent != end; parent = parent->parent)
        {
            c_list.push(parent->coord);
        }
        while (!c_list.empty())
        {
            path.push_back(c_list.top());
            c_list.pop();
        }
        return path;
    }
//}


// Good description of A-Star from http://www.policyalmanac.org/games/aStarTutorial.htm
std::vector<Coords> 
Map::pathFind(Coords s, Coords e, CreatureH cr) const
{
    std::vector<Coords> path;
    if (s == e) return path;

    OpenList open_list;
    ClosedList closed_list;
    
    open_list.insert(Node(s, closed_list.end(), 0, ManhattanDistance(s, e)));

    while (!open_list.empty())
    {
        // Get the lowest total cost square on open list and put it on the closed list
        ClosedIter parent = closed_list.insert(*open_list.get<1>().begin()).first;
        open_list.erase(*open_list.get<1>().begin());

        // for each of the 8 squares adjacent
        for (int sq = 0; sq < 8; ++sq)
        {
            Coords square =  parent->coord + Offsets[sq];

            // is this the target?
            if (square.X() == e.X() && square.Y() == e.Y())
                return CalculatePath(square, parent, closed_list.end(), path);

            // if it is not walkable, ignore it
            if (!isPassable(square, cr) || getCreature(square)) continue;

            // record the cost to get here and estimated cost to get to target
            Node node(square, parent, parent->cost + 1, parent->cost + 1 + ManhattanDistance(square, e));

            // if it is already on the closed list, ignore it.
            if (closed_list.count(node)) continue;

            // add the node to the open list. update the node's parent if the cost is lower
            AddOrUpdateOpenList(open_list, node);
        }
    }

    // we've exhausted all nodes, there mustn't be a path to the target
    return path;
}

