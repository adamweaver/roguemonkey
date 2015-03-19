#ifndef H_SPECIES_
#define H_SPECIES_

// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <map>
#include <string>


#include "handles.h"

struct Species
{
    enum Type
    {
        Human,

        EndSpecies
    };

    enum Nationality
    {
        Guilder, AlArqan, Xian,

        EndRaces
    };


    struct BodySlot
    {
        enum Type
        {
            Helmet,
            Amulet,
            Body,
            Cloak,
            Belt,
            Vambraces,
            Gloves,
            WieldedR,
            WieldedL,
            RingsR,
            RingsL,
            Greaves,
            Boots,
            Launcher,
            Quiver,
            EndSlots,
            PseudoDualWielded
        };

        static char const *TypeNames[EndSlots];
        static char const *SlotNames[EndSlots];
    };

    static char const *TypeNames[EndSpecies];

    /**
     * Constructor for a Species
     *
     * @param t      Type to create
     */
    Species(Type t);


    /**
     * Get equipped item in that body slot
     *
     * @param t      Slot type
     *
     * @return Item euipped, or ItemH() if empty
     */
    ItemH getInvInSlot(BodySlot::Type t) const;

    /**
     * Is an item equipped in that slot?
     *
     * @param t      Slot type
     *
     * @return true if item equipped
     */
    bool hasInvInSlot(BodySlot::Type t) const;

    /**
     * Swap item contained in slot
     *
     * @param t      Which body slot
     * @param it     Item to swap in
     *
     * @return previous ItemH or ItemH() held
     */
    ItemH swapInvInSlot(Species::BodySlot::Type t, ItemH it);


private:
    typedef std::map<BodySlot::Type, ItemH> Equipment;

    mutable Equipment        m_equipment;
    Species::Type            m_species;
};



#endif

