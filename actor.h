#ifndef H_ACTOR_
#define H_ACTOR_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <functional>

#include "handles.h"
#include "inputdef.h"
#include "map.h"


/**
 * Actor is a base class for all creatures, spell effects, and events
 * which need to occur at specified times.
 */
class Actor : public Describable
{   
public:
    /**
     * Speed of Actor in regular cycle. A cycle has 12 ticks: 
     * VSlow acts on tick 6 (every 12)
     * Slow acts on ticks 3 and 9 (every 6)
     * Normal acts on ticks 2, 6, and 10 (every 4)
     * Quick acts on ticks 3, 6, 9, and 12 (every 3)
     * Fast acts on ticks 1, 3, 5, 7, 9, and 11 (every 2)
     */
    enum Speed
    {
        VSlow = 12, Slow = 6, Normal = 4, Quick = 3, Fast = 2
    };

    virtual ~Actor();

    /**
     * Perform next action.
     *
     * @return     number of ticks taken to perform action. = 0 means death.
     */
    virtual unsigned int act() = 0;

    /**
     * Get Actor's default speed
     *
     * @return       Speed
     */
    virtual Speed getSpeed() const = 0;

    /**
     * Get the coordinates for actor
     *
     * @return       Coords location
     */
    Coords getCoords() const;

    /**
     * Get the next-acting turn count
     *
     * @return      turn count next allowed to act()
     */
    unsigned int getTurn() const;

    /**
     * Is the Actor a Hero?
     *
     * @return      non-zero for hero GUID
     */
    virtual int heroGUID() const = 0;

protected:
    Actor();    

private:
    friend class Map;
    friend struct ActorComp;

    struct ActorComp : std::binary_function<ActorH, ActorH, bool>
    {
        bool operator() (ActorH l, ActorH r) const
        {
            return l->m_turn < r->m_turn;
        }
    };

    Coords          m_coords;
    unsigned int    m_turn;
};



#endif

