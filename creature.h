#ifndef H_CREATURE_
#define H_CREATURE_ 1

// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>
#include <utility>
#include <vector>

#include "actor.h"
#include "handles.h"
#include "inputdef.h"
#include "skills.h"
#include "species.h"


class Creature : public Actor
{
public:
    enum Type
    {
        Hero, Monster, ClassedMonster, NPC
    };

    typedef std::pair<int, int> StatPair;

    enum Stat
    {
        Health, Mana, Might, Agility, Intellect, Charisma, Experience, EndStats
    };

    /**
     * Creates a new Creature of a certain species
     * @param t        Whether Hero, Monster or ClassedMonster
     * @param species  Genus of creature to create
     * @return         New creature
     */
    static CreatureH create(Type t, Species::Type species);
    virtual ~Creature() = 0;

    // From Actor
    /**
     * Perform AI action
     * @return         Number of ticks before next action
     */
    virtual unsigned int act() = 0;

    // Creature
    /**
     * Update creature's virtual FOV
     */
    virtual void updateView() = 0;

    /**
     * Get the current/maximum for a particular stat
     * @param st     statistic to check
     * @return       pair of current & maximum stat
     */
    virtual StatPair getStat(Stat st) const = 0;

    /**
     * Is the Creature a Hero or Monster, or ClassedMonster?
     * @return      creature class
     */
    virtual Creature::Type creatureType() const = 0;

    /**
     * Is the creature a Hero?
     * @return      non-zero for hero GUID
     */
    virtual int heroGUID() const = 0;

    /**
     * Get maximum distance creature can see
     * @return    max distance in squares
     */
    virtual int getSightRadius() const = 0;

    /**
     * Get colour & character pair displayed for character
     * @param cr     Creature viewing
     * @return Representation pair
     */
    virtual Representation getRepresentation(CreatureH cr) const = 0;

    // Body stuff
    /**
     * Get Creature's inventory
     * @return     ItemPile of inventory
     */
    virtual ItemPileH getInventory() const = 0;

    /**
     * Get item contained in body slot
     * @param t      Body slot to retrieve
     * @return Item held or ItemH() if empty
     */
    virtual ItemH getInvInSlot(Species::BodySlot::Type t) const = 0;

    /**
     * Swap item contained in slot
     * @param t      Which body slot
     * @param it     Item to swap in
     * @return previous ItemH or ItemH() held
     */
    virtual ItemH swapInvInSlot(Species::BodySlot::Type t, ItemH it) = 0;

    /**
     * Increase class level of Creature
     */
    virtual void levelUp() = 0;
  
    /**
     * Can the Creature step there on this map?
     * @param c      Coordinate position to test
     * @return       true if can stand there
     */
    bool canStep(Coords c);

    /**
     * Get the gender applicable to this creature
     * @return       Gender of creature
     */
    virtual Gender getGender() const = 0;  

    /**
     * Get a list of classes as seen by the viewer
     * @param  viewer  creature examining
     * @return         classes to which this creature apparently belongs
     */
    Classes::ClassLevels getApparentClasses(CreatureH viewer) const;

    /**
     * Get basic bonus for attacking this creature. 
     * Takes into account weapon bonuses, position, type of enemy, etc.
     * @param  defender   creature being attacked
     * @return            basic bonus
     */
    virtual int getAttackBonus(CreatureH defender) const = 0;

    /**
     * Get basic bonus for defense if attacked by this creature.
     * Takes into account dodge bonus, armour bonus, type of attacker, etc.
     * @param attacker   creature doing the attacking
     * @return           basic bonus 
     */
    virtual int getDefenseBonus(CreatureH attacker) const = 0;

    /**
     * Get list of damage types & amounts caused
     * @param defender   who was hit
     * @param critical   was this a critical hit?
     * @return           vector of damage types
     */
    virtual std::vector<Damage> getCombatDamage(CreatureH defender, bool critical) const = 0;

    /**
     * Apply damage to creature
     * @param dam        damage to apply
     */
    virtual void applyDamage(Damage const & dam) = 0;

    /**
     * Is the Creature still alive/able to act? Deceased != undead/demonic
     * @return  true if creature can no longer act
     */
    virtual bool deceased() const = 0;

    /**
     * Does the Creature have that (virtual?) skill at that level?
     * @param sk         skill to check
     * @param lev        level to check for
     * @return           true if skill exists
     */
    virtual bool hasSkill(Skills::Type sk, int lev = 1) const = 0;

protected:
    /**
     * Create a creature with a default last-acted date
     */
    Creature();

    /**
     * Get the list of classes to which creature belongs
     * @return     absolute list of classes
     */
    virtual Classes::ClassLevels getClassLevels() const = 0;

    /**
     * Convert 'this' into a CreatureH safely
     * @return      CreatureH handle for this Hero
     */
    CreatureH getCreatureHandle(); 

};



/**
 * Result of combat. Values ORed together
 */
enum CombatResult
{
    DefenderDamaged = 1U, DefenderKilled = 2U, AttackerDamaged = 4U, AttackerKilled = 8U, AllResult = 15U
};

/**
 * Initiate combat between attacker and defender
 * @param  attacker           which creature is attacking
 * @param  defender           which creature is defending
 * @return                    result of combat (ORed together)
 */
CombatResult Fight(CreatureH attacker, CreatureH defender);

#endif

