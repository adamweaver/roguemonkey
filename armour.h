#ifndef H_ARMOUR_
#define H_ARMOUR_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <set>
#include <string>

#include "handles.h"
#include "item.h"


class Armour : public Item
{
public:
    enum Type
    {
        Hat, Helm, Gorget, 
        Gauntlets, Vambrace, Boots, Greaves, Belt, 
        Robe, Tunic, Maille, Scale, Plate, Cloak,
        Buckler, TargetShield, KiteShield, TowerShield,

        EndArmour
    };

    virtual ~Armour();

    static void Init();

    static ArmourH createArmour();

    virtual std::string describe(CreatureH cr) const;
    virtual std::string describe() const;
    virtual std::string describeIndef(CreatureH cr) const;
    virtual std::string describeIndef() const;
    virtual std::string describeNum(CreatureH cr, int num) const;
    virtual Gender getGender() const { return Neuter; }

    virtual ItemH doClone() const;

    /*
     * Get type of item
     * @return type of item
     */
    virtual ItemType getItemType() const;

    /**
     * Get character representation for an Item (ie '(' and Cyan for weapons)
     * @return character to be displayed
     */
    virtual Representation getRepresentation() const;

    /**
     * Will this item stack with itself?
     * @return true for scrolls, potions, missile weapons, etc
     */
    virtual bool willStack() const;

    /**
     * Which body slot is used by item
     * @return Slot type
     */
    virtual Species::BodySlot::Type slotRequired() const;


private:
    Armour();

    virtual bool doLessThan(ItemH r) const;
    virtual bool doEquivalent(ItemH r) const;
    virtual bool doEquals(ItemH r) const;

    Type      m_type;
    int       m_plus;
};

#endif
