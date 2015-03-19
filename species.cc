// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <cassert>
#include <iostream>
#include <vector>

#include "item.h"
#include "option.h"
#include "species.h"


//============================================================================
// Species statics
//============================================================================
//
char const *Species::BodySlot::SlotNames[Species::BodySlot::EndSlots] =
{
    "Helmet", "Amulet", "Body", "Cloak", "Belt", "Bracers", "Gloves",
    "", "", "Ring", "", "Greaves", "Boots", "Launcher", "Quiver"
};

char const *Species::BodySlot::TypeNames[Species::BodySlot::EndSlots] =
{
    "Helm", "Neck", "Body", "Cloak", "Belt", "Bracers", "Gloves",
    "Right Hand", "Left Hand", "Right Ring", "Left Ring", "Greaves", "Boots",
    "Launcher", "Quiver"
};



namespace
{
    char const *req_fields[] =
    {
        "Gender", "Heads", "Arms", "Legs"
    };

    int const num_req_fields = sizeof(req_fields) / sizeof(req_fields[0]);

    struct SpeciesDBase
    {
        private:

            static SpeciesDBase & getSDBase()
            {
                static SpeciesDBase sdb;
                return sdb;
            }

        public:

        struct SpeciesData
        {
            Gender gender;
            int index;
        };

        std::vector<SpeciesData> SDB;

        SpeciesDBase()
        : SDB(Species::EndSpecies)
        {
            OptionH df = Option::Create("data/species.ini");
/*            df->dieIfStructureNotOK("data/species.ini", Species::TypeNames,
                                    Species::EndSpecies, req_fields, num_req_fields);

            for (int i = 0; i < Species::EndSpecies; ++i)
            {
                OptionH tree = df->getSub(Species::TypeNames[i]);
                SDB[i].index = i;
                SDB[i].gender = Species::Gender::Type(0);
                std::string gender(df->getAsString("Gender"));
                if (gender.find('M'))
                    SDB[i].gender |= Species::Gender::Male;
                if (gender.find('F'))
                    SDB[i].gender |= Species::Gender::Female;
                if (SDB[i].gender == 0)
                    SDB[i].gender = Species::Gender::Neuter;
            }
*/
        }

        static SpeciesData & getSData(Species::Type which)
        {
            return getSDBase().SDB[which];
        }

    };
}



//============================================================================
// Species
//============================================================================
char const *
Species::TypeNames[EndSpecies] =
{
    "Human"
};




Species::Species(Species::Type t)
:   m_equipment(),
    m_species(t)
{

}



ItemH
Species::getInvInSlot(Species::BodySlot::Type t) const
{
    return m_equipment[t];
}



bool
Species::hasInvInSlot(Species::BodySlot::Type t) const
{
    return m_equipment[t];
}


ItemH
Species::swapInvInSlot(Species::BodySlot::Type t, ItemH item)
{
    Species::BodySlot::Type const DUAL = Species::BodySlot::PseudoDualWielded;
    Species::BodySlot::Type const LEFT = Species::BodySlot::WieldedL;
    Species::BodySlot::Type const RIGHT= Species::BodySlot::WieldedR;

    if (item && item->numHandsRequired(0) == 2)
        t = DUAL;

    if (t == DUAL)
    {
        ItemH old_l = m_equipment[LEFT];
        ItemH old_r = m_equipment[RIGHT];

        if (old_l.get())
            old_l->delEffect(ItemEffect::Equipped);

        if (old_r.get())
            old_r->delEffect(ItemEffect::Equipped);

        m_equipment[LEFT] = item;
        m_equipment[RIGHT] = item;

        if (item.get())
            item->addEffect(ItemEffect::Equipped, ItemEffect(int(DUAL)));

        return old_r;
    }

    if (m_equipment[t] && m_equipment[t]->getEffect(ItemEffect::Equipped).edata1 == DUAL)
    {
        ItemH old_l = m_equipment[LEFT];
        ItemH old_r = m_equipment[RIGHT];

        if (old_l.get())
            old_l->delEffect(ItemEffect::Equipped);

        if (old_r.get())
            old_r->delEffect(ItemEffect::Equipped);

        m_equipment[LEFT] = m_equipment[RIGHT] = ItemH();

        ItemH old = (t == LEFT ? old_l : old_r);
        m_equipment[t] = item;

        if (item)
            item->addEffect(ItemEffect::Equipped, ItemEffect(int(t)));

        return old;
    }

    ItemH old =  m_equipment[t];
    m_equipment[t] = item;

    if (old)
        old->delEffect(ItemEffect::Equipped);
    if (item)
        item->addEffect(ItemEffect::Equipped, ItemEffect(int(t)));
    return old;
}

