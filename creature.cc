// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <cassert>
#include <functional>
#include <sstream>

#include "creature.h"
#include "dice.h"
#include "hero.h"
#include "item.h"
#include "map.h"
#include "monster.h"
#include "textutils.h"
#include "world.h"


//============================================================================
// Creature
//============================================================================

Creature::~Creature()
{
}



CreatureH
Creature::create(Creature::Type t, Species::Type species)
{
    switch(t)
    {
    case Creature::Hero:
        assert(!"illegally creating Hero in Creature::create()");

    case Creature::Monster:
        return Monster::createMonster(species);


    case Creature::ClassedMonster:
    case Creature::NPC:
        assert(!"Created Monster");

    default:
        assert(!"Dunno what I created");
    }
	
	return CreatureH();

}



Creature::Creature()
:   Actor()
{
}



Classes::ClassLevels 
Creature::getApparentClasses(CreatureH /*viewer*/) const
{
    Classes::ClassLevels cl(getClassLevels());
    return cl;
}


CreatureH
Creature::getCreatureHandle()
{
    return boost::dynamic_pointer_cast<Creature>(shared_from_this());
}


//============================================================================
// Free Functions
//============================================================================

namespace
{
    struct DamSort : public std::binary_function<Damage, Damage, bool>
    {
        bool operator() (Damage const & lh, Damage const & rh) const
        { return (lh.num == rh.num) ? lh.type > rh.type : lh.num > rh.num; }
    };

    std::string GetPredominantDamageType(std::vector<Damage> & dam)
    {
        if (dam.empty())
            return "lightly $tap";

        std::nth_element(dam.begin(), dam.begin(), dam.end(), DamSort());
        switch (dam.front().type)
        {
        case Damage::Acid :
            return "$burn";
        case Damage::Bludgeoning :
            return "$crush";
        case Damage::Cold :
            return "$freeze";
        case Damage::Fire :
            return "$burn";
        case Damage::Lightning :
            return "$shock";
        case Damage::Magic :
            return "$blast";
        case Damage::Piercing :
            return "$pierce";
        case Damage::Poison :
            return "$poison";
        case Damage::Slashing :
            return "$slash";
        default :
            break;
        }
        return "$bash";
    }
}
 


CombatResult
Fight(CreatureH attacker, CreatureH defender)
{
    int base_attack = attacker->getAttackBonus(defender);
    int base_defend = defender->getDefenseBonus(attacker);
    int base_delta  = base_attack - base_defend;

    // it's easier to attack than to defend
    int attack_roll = Dice::Random0(50);
    int defend_roll = Dice::Random0(40);

    // critical hits are scored by the lucky (2% chance) or by those vastly more skilled than defenders
    bool critical_hit = (attack_roll == 49) || (base_delta > Dice::Random0(base_defend));

    // critical misses are only scored by the unlucky and inept
    bool critical_miss = (!critical_hit && attack_roll == 0 && Dice::Random0(base_attack) < 1);

    if (critical_hit || base_attack + attack_roll > base_defend + defend_roll)
    {
        // Hit!
        std::vector<Damage> damage = attacker->getCombatDamage(defender, critical_hit);
        Apply(damage.begin(), damage.end(), *defender, &Creature::applyDamage);

        std::string msg("$1n ");
        msg.append(GetPredominantDamageType(damage));
        msg.append(" $2a.");
        HERO->printImmediateMessage(TextUtils::FormatMessage(msg, HERO, attacker, defender));
        return DefenderDamaged;
    }
    else if (critical_miss)
    {
        // TODO: Critical miss
        return AttackerDamaged;
    }
    else
    {
        HERO->printImmediateMessage(TextUtils::FormatMessage("$1n miss $2a.", HERO, attacker, defender));
    }
    return DefenderDamaged;
}

