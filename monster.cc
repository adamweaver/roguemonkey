// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <vector>

#include "dice.h"
#include "hero.h"
#include "map.h"
#include "monster.h"
#include "world.h"

//============================================================================
// Monster
//============================================================================

CreatureH
Monster::createMonster(Species::Type t)
{
    return CreatureH(new Monster(t));
}


Monster::Monster(Species::Type t)
:   Creature(),
    m_species(new Species(t)),
    m_inventory(new ItemPile(52)),
    m_target_cr(),
    m_target_pos(),
    m_path()
{
   setTargetPosition(Coords(0, 0));
}


Monster::~Monster()
{
}


Creature::Type
Monster::creatureType() const
{
    return Creature::Monster;
}


std::string
Monster::describe() const
{
    return "monster";
}


std::string
Monster::describe(CreatureH) const
{
    return "monster";
}


std::string 
Monster::describeIndef(CreatureH cr) const
{
    return "a monster";
}


std::string 
Monster::describeIndef() const
{
    return "a monster";
}


Representation
Monster::getRepresentation(CreatureH /*cr*/) const
{
    return Representation('o', Colour::Orange);
}



int
Monster::getSightRadius() const
{
    return 5;
}



unsigned int
Monster::act()
{  
//    if (m_path.empty())
//    {
        setTargetPosition(HERO->getCoords());
        updatePathToPosition();
//    }

    if (m_path.empty())
        return 1 * Actor::Slow;

    Coords here(getCoords());
    CreatureH me(getCreatureHandle());
    Coords there(m_path.front());

    if (here.M()->isPassable(there, me) && !here.M()->getCreature(there))
    {
        here.M()->moveCreature(there, me);
        m_path.pop_front();
    }
    return 1 * Actor::Slow;
}


Creature::StatPair
Monster::getStat(Creature::Stat /*st*/) const
{
    return StatPair(5, 5);
}



void
Monster::updateView()
{

}


int
Monster::heroGUID() const
{
    return 0;
}


ItemPileH
Monster::getInventory() const
{
    return m_inventory;
}


ItemH
Monster::getInvInSlot(Species::BodySlot::Type t) const
{
    return m_species->getInvInSlot(t);
}


ItemH
Monster::swapInvInSlot(Species::BodySlot::Type t, ItemH item)
{
    return m_species->swapInvInSlot(t, item);
}



void
Monster::levelUp()
{
}



Classes::ClassLevels 
Monster::getClassLevels() const
{
    return Classes::ClassLevels();
}


Actor::Speed
Monster::getSpeed() const
{
    return Actor::Fast;
}


Gender
Monster::getGender() const
{
    return Male;
}


int
Monster::getAttackBonus(CreatureH /*defender*/) const
{
    return 2;
}


int 
Monster::getDefenseBonus(CreatureH /*attacker*/) const
{
    return 0;
}


std::vector<Damage> 
Monster::getCombatDamage(CreatureH /*defender*/, bool critical) const
{
    return std::vector<Damage>(1, Damage(Damage::Magic, 4 + 4 * critical));
}  


void
Monster::applyDamage(Damage const & dam)
{
    return;
}


bool 
Monster::deceased() const
{
    return false;
}


bool 
Monster::hasSkill(Skills::Type, int lev) const
{
    return false;
}


Coords
Monster::getTargetPosition() const
{
    return m_target_pos;
}


void
Monster::setTargetPosition(Coords pos)
{
    m_target_pos = pos;
}


void
Monster::updatePathToPosition()
{
    Coords here(getCoords());
    std::vector<Coords> path(here.M()->pathFind(here, getTargetPosition(), getCreatureHandle()));
    m_path.assign(path.begin(), path.end());
}



