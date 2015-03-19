#ifndef H_WEAPON_
#define H_WEAPON_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>

#include "item.h"
#include "handles.h"


class Weapon : public Item
{
public:
    Weapon();
    Weapon & addData();
    enum Type
    {
        // short blades
        Dagger, /*alarq Jambiya, xian Kris*/
        ShortSword, /*alarq Makhaira, xian */
        Sai, 
        Sabre, /*alarq Falchion, xian Dao*/

        // long blades
        BroadSword,  /*alarq  , xian Nandao*/
        LongSword, /*alarq , xian Jian*/
        GreatSword, /*alarq Tulwar, xian Dadao*/
        Scimitar, /*alarq Shamshir, xian Piandao*/

        // Polearms
        Staff, 
        Halberd,
        Glaive, /*alarq , xian KwanDao*/
        Spear, /*alarq , xian Qiang */

        // Hafted
        Mace, 
        SpikedFlail,
        HandAxe,
        BattleAxe,     

        // Exotics
        ChainWhip, 
        MeteorHammer, 
        Kama,

        // Bows
        LongBow, 
        ShortBow, 

        // Crossbows & Gunnes
        LightCrossbow, 
        HeavyCrossbow,
       
        EndWeapon
    };

    static void Init();


    static WeaponH createWeapon();

    virtual ItemH doClone() const;

    virtual std::string describe(CreatureH cr) const;
    virtual std::string describe() const;
    virtual std::string describeIndef(CreatureH cr) const;
    virtual std::string describeIndef() const;
    virtual Gender getGender() const { return Neuter; }
    virtual std::string describeNum(CreatureH cr, int num) const;

    // Item
    virtual ItemType getItemType() const;
    virtual Representation getRepresentation() const;

    /**
     * Will this item stack with itself?
     *
     * @return true for scrolls, potions, missile weapons, etc
     */
    virtual bool willStack() const;

    /**
     * Number of hands required to wield
     *
     * @param str    wielder strength
     * @return       1 for 1, 2 for 2 hands, 3 for 1.5 hands
     */
    virtual int numHandsRequired(int str) const;

    /**
     * Which body slot is used by item
     *
     * @return Slot type
     */
    virtual Species::BodySlot::Type slotRequired() const;

private:
    virtual bool doLessThan(ItemH r) const;
    virtual bool doEquivalent(ItemH r) const;
    virtual bool doEquals(ItemH r) const;

    Type m_type;
    int m_plus;
};







#endif

