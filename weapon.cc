// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <cassert>
#include <sstream>

#include "dice.h"
#include "dictionary.h"
#include "inputdef.h"
#include "option.h"
#include "skills.h"
#include "species.h"
#include "textutils.h"
#include "weapon.h"


//============================================================================
// Weapon statistics
//============================================================================
namespace 
{
    struct WeaponStats
    {
        typedef Dictionary<Species::Nationality, std::string> RacialName;

        Representation           representation;
        RacialName               racial_name;
        RacialName               unid_racial_name;
        int                      num_hands;
        Skills::Type             skill_used;

        WeaponStats(Representation rep, std::string const & default_name) :
            representation(rep),
            racial_name(),
            unid_racial_name(),
            num_hands(1),
            skill_used()
        {
            racial_name.insert(Species::Guilder, default_name);
            unid_racial_name.insert(Species::Guilder, default_name);
        }

        WeaponStats & Name(Species::Nationality r, std::string const & name)
        { racial_name.insert(r, name); return *this; }

        WeaponStats & UnIdName(Species::Nationality r, std::string const & name)
        { unid_racial_name.insert(r, name); return *this; }

        WeaponStats & Hands(int num)
        { num_hands = num; return *this; }

        WeaponStats & SkillUsed(Skills::Type t)
        { skill_used = t; return *this; }
    };

    Representation cy_paren('(', Colour::Aqua);
    Representation br_paren('(', Colour::Brown);
    Representation bl_paren('(', Colour::LBlue);
    Representation br_staff('|', Colour::Brown);
    
    WeaponStats wstats[] =
    {
        // easily concealable
        WeaponStats(Representation('(', Colour::LGrey), "#dagger").
        Name(Species::AlArqan, "#jambiya").
        Name(Species::Xian, "#kris").
        SkillUsed(Skills::ShortBlades),

        // higher damage than dagger, but not concealable. 
        WeaponStats(Representation('(', Colour::LGrey), "short #sword").
        Name(Species::AlArqan, "#akinakes").
        SkillUsed(Skills::ShortBlades),

        // can catch weapons
        WeaponStats(Representation('(', Colour::LGrey), "#pair of dragon teeth").
        Name(Species::Xian, "#sai").
        SkillUsed(Skills::ShortBlades).
        Hands(2),

        // 
        WeaponStats(Representation('(', Colour::LGrey), "#sabre").
        Name(Species::AlArqan, "#falchion").
        Name(Species::Xian, "#dao").
        SkillUsed(Skills::ShortBlades),

        // basic sword
        WeaponStats(Representation('(', Colour::LBlue), "#broadsword").
        Name(Species::Xian, "#nandao").
        SkillUsed(Skills::Swords),

        // better armour penetration
        WeaponStats(Representation('(', Colour::LBlue), "#longsword").
        Name(Species::Xian, "#jian").
        SkillUsed(Skills::Swords),

        // higher damage
        WeaponStats(Representation('(', Colour::LBlue), "#greatsword").
        Name(Species::AlArqan, "#tulwar").
        Name(Species::Xian, "#dadao").
        Hands(2).
        SkillUsed(Skills::Swords),

        // better at cutting, bad at bludgeoning
        WeaponStats(Representation('(', Colour::LBlue), "#scimitar").
        Name(Species::AlArqan, "#shamshir").
        Name(Species::Xian, "#piandao").
        SkillUsed(Skills::Swords),

        // cheapest weapon
        WeaponStats(Representation('|', Colour::LGreen), "#staff").
        Name(Species::Xian, "#nangun").
        Hands(2).
        SkillUsed(Skills::Polearms),

        // highest damage of polearms
        WeaponStats(Representation('|', Colour::LGreen), "#halberd").
        Name(Species::Xian, "moon #axe").
        Hands(2).
        SkillUsed(Skills::Polearms),

        // best defence
        WeaponStats(Representation('|', Colour::LGreen), "#glaive").
        Name(Species::Xian, "#kwandao").
        Hands(2).
        SkillUsed(Skills::Polearms),

        // basic weapon
        WeaponStats(Representation('|', Colour::LGrey), "#spear").
        Name(Species::Xian, "#qiang").
        Hands(3).
        SkillUsed(Skills::Polearms),

        WeaponStats(Representation('(', Colour::DGrey), "#mace").
        Name(Species::AlArqan, "#club").
        SkillUsed(Skills::Hafted),

        WeaponStats(Representation('(', Colour::DGrey), "spiked #flail").
        Name(Species::Xian, "#lianjia").
        Name(Species::AlArqan, "morning #star").
        SkillUsed(Skills::Hafted),

        WeaponStats(Representation('(', Colour::DGrey), "hand #axe").
        Name(Species::AlArqan, "#sagaris").
        SkillUsed(Skills::Hafted),

        WeaponStats(Representation('(', Colour::DGrey), "battle #axe").
        Hands(3).
        SkillUsed(Skills::Hafted),

        WeaponStats(Representation('(', Colour::Magenta), "chain #whip").
        SkillUsed(Skills::Exotics),

        // fastest weapon
        WeaponStats(Representation('(', Colour::Magenta), "meteor #hammer").
        Hands(2).
        SkillUsed(Skills::Exotics),

        WeaponStats(Representation('(', Colour::Magenta), "#kama").
        SkillUsed(Skills::Exotics),

        WeaponStats(Representation('(', Colour::Yellow), "long #bow").
        SkillUsed(Skills::Bows),

        WeaponStats(Representation('(', Colour::Yellow), "short #bow").
        SkillUsed(Skills::Bows),

        WeaponStats(Representation('(', Colour::Brown), "#crossbow").
        SkillUsed(Skills::Crossbows),

        WeaponStats(Representation('(', Colour::Brown), "heavy #crossbow").
        SkillUsed(Skills::Crossbows)

    };
    int const num_weapons = sizeof(wstats) / sizeof(WeaponStats);

}
//============================================================================
// Weapon
//============================================================================
void
Weapon::Init()
{
    assert(Weapon::EndWeapon == num_weapons);
}

Weapon::Weapon() :
    m_type(static_cast<Weapon::Type>(Dice::Random0(Weapon::EndWeapon))),
    m_plus(0)
{
}


WeaponH
Weapon::createWeapon()
{
    return WeaponH(new Weapon);
}


ItemH
Weapon::doClone() const
{
    WeaponH weapon(new Weapon);
    weapon->m_type = m_type;
    weapon->m_plus = m_plus;
    return weapon;
}



Item::ItemType
Weapon::getItemType() const
{
    return Item::Weapon;
}


Representation
Weapon::getRepresentation() const
{
    return wstats[m_type].representation;
}


int
Weapon::numHandsRequired(int /*str*/) const
{
    return wstats[m_type].num_hands;
}


std::string
Weapon::describe(CreatureH) const
{
    return describe();
}


std::string 
Weapon::describe() const
{
/*    WeaponStats::RacialName const & rn = (known_id & Ident::Basename) ? 
        wstats[m_type].racial_name : wstats[m_type].unid_racial_name;          
    WeaponStats::RacialName::const_iterator ci = rn.find(Species::Florin);
    if (ci == rn.end())
        ci = wstats[m_type].racial_name.find(Species::Florin);
    return ci->second;*/
    return TextUtils::PluraliseString(wstats[m_type].racial_name.begin()->second, getNumber());
}


std::string 
Weapon::describeIndef(CreatureH) const
{
    return describeIndef();
}


std::string 
Weapon::describeIndef() const
{
    int num = getNumber();
    std::string name(TextUtils::PluraliseString(wstats[m_type].racial_name.begin()->second, num));
    return num == 1 ? TextUtils::IndefiniteArticle(name) + " " + name : name;
}


std::string
Weapon::describeNum(CreatureH, int num) const
{
    return TextUtils::PluraliseString(wstats[m_type].racial_name.begin()->second, num);
}




Species::BodySlot::Type
Weapon::slotRequired() const
{
    switch (m_type)
    {
    case ShortBow:
    case LongBow:
    case LightCrossbow:
    case HeavyCrossbow:
        return Species::BodySlot::Launcher;
    default:
        break;
    }
    return Species::BodySlot::WieldedR;
}





bool
Weapon::willStack() const
{
    return false;
}


bool
Weapon::doLessThan(ItemH r) const
{
    assert(r->getItemType() == Item::Weapon);
    WeaponH rh = boost::dynamic_pointer_cast<Weapon>(r);
    assert(rh != 0 && "Invalid Weapon this ptr passed");

    if (m_type < rh->m_type)
        return true;
    if (m_type == rh->m_type && m_plus < rh->m_plus)
        return true;

    return false;
}


bool
Weapon::doEquivalent(ItemH r) const
{
    assert(r->getItemType() == Item::Weapon);
    WeaponH rh = boost::dynamic_pointer_cast<Weapon>(r);
    assert(rh && "Invalid Weapon this ptr passed");

    return m_type == rh->m_type && m_plus == rh->m_plus;
}


bool
Weapon::doEquals(ItemH r) const
{
    assert(r->getItemType() == Item::Weapon);
    WeaponH rh = boost::dynamic_pointer_cast<Weapon>(r);
    assert(rh && "Invalid Weapon this ptr passed");

    return m_type == rh->m_type && m_plus == rh->m_plus;
}



