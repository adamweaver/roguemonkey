// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt


#include <cassert>
#include <sstream>

#include "armour.h"
#include "dice.h"
#include "dictionary.h"
#include "materials.h"
#include "species.h"
#include "textutils.h"


//============================================================================
// Armour statistics
//============================================================================
namespace
{
    struct ArmourStats
    {
        typedef Dictionary<Material::Type, std::string> MaterialName;
        typedef std::set<Species::BodySlot::Type>       SlotsTaken;

        Representation           representation;
        MaterialName             material_name;
        Species::BodySlot::Type  primary_slot;
        SlotsTaken               taken_slots;
        

        ArmourStats(Representation rep, Material::Type def_mat, 
                    std::string const & default_name) :
            representation(rep),
            material_name(),
            primary_slot(Species::BodySlot::Body),
            taken_slots()
        {
            material_name.insert(def_mat, default_name);
        }

        ArmourStats & Name(Material::Type m, std::string const & name)
        { material_name.insert(m, name); return *this; }

        ArmourStats & Slot(Species::BodySlot::Type slot)
        { primary_slot = slot; return *this; }

        ArmourStats & NewSlot(Species::BodySlot::Type slot)
        { taken_slots.insert(slot); return *this; }

    };

    Representation cy_brack('[', Colour::Aqua);
    Representation br_brack('[', Colour::Brown);

    ArmourStats astats[Armour::EndArmour] = 
    {
        ArmourStats(Representation('[', Colour::DGreen), Material::Leather, "#hat").
        Slot(Species::BodySlot::Helmet),

        ArmourStats(Representation('[', Colour::DGreen), Material::Steel, "#helm").
        Slot(Species::BodySlot::Helmet),

        ArmourStats(Representation('[', Colour::DGrey), Material::Steel, "#gorget").
        Slot(Species::BodySlot::Amulet),

        ArmourStats(Representation('[', Colour::LBlue), Material::Leather, "#pair of leather gloves").
        Slot(Species::BodySlot::Gloves).
        Name(Material::Steel, "#pair of gauntlets"),

        ArmourStats(Representation('[', Colour::DGrey), Material::Steel, "#pair of vambraces").
        Slot(Species::BodySlot::Vambraces),

        ArmourStats(Representation('[', Colour::DBlue), Material::Leather, "#pair of leather boots").
        Slot(Species::BodySlot::Boots),

        ArmourStats(Representation('[', Colour::DGrey), Material::Steel, "#pair of greaves").
        Slot(Species::BodySlot::Greaves),

        ArmourStats(Representation('[', Colour::DGrey), Material::Leather, "#belt").
        Slot(Species::BodySlot::Belt),

        ArmourStats(Representation('[', Colour::Aqua), Material::Cloth, "#robe"),

        ArmourStats(Representation('[', Colour::Aqua), Material::Leather, "#tunic").
        Name(Material::Steel, "maille #hauberk"),

        ArmourStats(Representation('[', Colour::Aqua), Material::Steel, "#coat of maille"),

        ArmourStats(Representation('[', Colour::Aqua), Material::Steel, "#coat of scales"),

        ArmourStats(Representation('[', Colour::Aqua), Material::Steel, "#plate").
        Name(Material::Leather, "#set of leathers"),

        ArmourStats(Representation('[', Colour::DGrey), Material::Cloth, "#cloak"),

        ArmourStats(Representation('[', Colour::LGreen), Material::Wood, "#buckler").
        Slot(Species::BodySlot::WieldedL),

        ArmourStats(Representation('[', Colour::LGreen), Material::Wood, "target #shield").
        Slot(Species::BodySlot::WieldedL),

        ArmourStats(Representation('[', Colour::LGreen), Material::Steel, "kite #shield").
        Slot(Species::BodySlot::WieldedL),

        ArmourStats(Representation('[', Colour::LGreen), Material::Steel, "tower #shield").
        Slot(Species::BodySlot::WieldedL)
    };

    int const num_armours = sizeof(astats) / sizeof(ArmourStats);

}

//============================================================================
// Armour
//============================================================================
void
Armour::Init()
{
    assert(num_armours == Armour::EndArmour);
}

Armour::Armour() : 
    m_type(static_cast<Armour::Type>(Dice::Random0(Armour::EndArmour))),
    m_plus(0)
{
}


Armour::~Armour()
{
}


ArmourH
Armour::createArmour()
{
    return ArmourH(new Armour);
}


ItemH
Armour::doClone() const
{
    ArmourH armour(new Armour);
    armour->m_type = m_type;
    armour->m_plus = m_plus;
    return armour;
}

Item::ItemType
Armour::getItemType() const
{
    return Item::Armour;
}


Representation
Armour::getRepresentation() const
{
    return astats[m_type].representation;
}


std::string 
Armour::describe() const
{
    return TextUtils::PluraliseString(astats[m_type].material_name.begin()->second, 1);
}


std::string
Armour::describe(CreatureH) const
{
    return describe();
}


std::string 
Armour::describeIndef(CreatureH) const
{
    return describeIndef();
}


std::string 
Armour::describeIndef() const
{
    int num = getNumber();
    std::string name(TextUtils::PluraliseString(astats[m_type].material_name.begin()->second, num));
    return num == 1 ? TextUtils::IndefiniteArticle(name) + " " + name : name;
}


std::string 
Armour::describeNum(CreatureH, int num) const
{
    return TextUtils::PluraliseString(astats[m_type].material_name.begin()->second, num);
}


Species::BodySlot::Type
Armour::slotRequired() const
{
    return astats[m_type].primary_slot;
}


bool
Armour::willStack() const
{
    return false;
}


bool
Armour::doLessThan(ItemH r) const
{
    assert(r->getItemType() == Item::Armour);
    ArmourH rh = boost::dynamic_pointer_cast<Armour>(r);
    assert(rh && "Invalid Armour this ptr passed");

    if (m_type < rh->m_type)
        return true;
    if (m_type == rh->m_type && m_plus < rh->m_plus)
        return true;

    return false;
}


bool
Armour::doEquivalent(ItemH r) const
{
    assert(r->getItemType() == Item::Armour);
    ArmourH rh = boost::dynamic_pointer_cast<Armour>(r);
    assert(rh && "Invalid Armour this ptr passed");

    return m_type == rh->m_type && m_plus == rh->m_plus;
}


bool
Armour::doEquals(ItemH r) const
{
    assert(r->getItemType() == Item::Armour);
    ArmourH rh = boost::dynamic_pointer_cast<Armour>(r);
    assert(rh && "Invalid Armour this ptr passed");

    return m_type == rh->m_type && m_plus == rh->m_plus;
}

