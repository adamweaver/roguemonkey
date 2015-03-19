// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include "actor.h"
#include "creature.h"
#include "dungeonmaster.h"

//============================================================================
// Actor
//============================================================================
Actor::Actor() :
    m_coords(-1, -1),
    m_turn(0)
{
}


Actor::~Actor()
{
}




Coords
Actor::getCoords() const
{
    return m_coords;
}



unsigned int 
Actor::getTurn() const
{
    return m_turn;
}


//============================================================================


