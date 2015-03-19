#ifndef H_MONSTER_
#define H_MONSTER_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <deque>
#include <string>


#include "creature.h"
#include "handles.h"
#include "item.h"
#include "species.h"


class Monster : public Creature
{
    SpeciesH            m_species;
    ItemPileH           m_inventory;
    CreatureH           m_target_cr;
    Coords              m_target_pos;
    std::deque<Coords>  m_path;

    explicit Monster(Species::Type t);

public:
    /**
     * Monster constructor
     * @param t      Species of new monster
     */
    static CreatureH createMonster(Species::Type t);

    virtual ~Monster();


    virtual std::string describe(CreatureH viewer) const;
    virtual std::string describe() const;
    virtual std::string describeIndef(CreatureH cr) const;
    virtual std::string describeIndef() const;
    /**
     * Is the Creature a Hero or Monster, or ClassedMonster?
     * @return      creature class
     */
    virtual Creature::Type creatureType() const;

    /**
     * Get colour & character pair displayed for character
     * @param cr     Creature viewing
     * @return Representation pair
     */
    virtual Representation getRepresentation(CreatureH cr) const;

    /**
     * Perform AI action
     * @return         Number of ticks before next action
     */
    virtual unsigned int act();

    /**
     * Get the current/maximum for a particular stat
     * @param st     statistic to check
     * @return       pair of current & maximum stat
     */
    virtual StatPair getStat(Stat st) const;

    /**
     * Update creature's virtual FOV
     */
    virtual void updateView();

    /**
     * Is the creature a Hero?
     * @return      non-zero for hero GUID
     */
    virtual int heroGUID() const;

    /**
     * Get maximum distance creature can see
     * @return    max distance in squares
     */
    virtual int getSightRadius() const;

    /**
     * Get Creature's inventory
     * @return     ItemPile of inventory
     */
    virtual ItemPileH getInventory() const;

    /**
     * Get item contained in body slot
     * @param t      Body slot to retrieve
     * @return Item held or ItemH() if empty
     */
    virtual ItemH getInvInSlot(Species::BodySlot::Type t) const;

    /**
     * Swap item contained in slot
     * @param t      Which body slot
     * @param it     Item to swap in
     * @return previous ItemH or ItemH() held
     */
    virtual ItemH swapInvInSlot(Species::BodySlot::Type t, ItemH it);

    /**
     * Increase class level of Monster
     */
    virtual void levelUp();

    /**
     * Get Monster's default speed
     * @return       Speed
     */
    virtual Actor::Speed getSpeed() const;

    /**
     * Get the list of classes to which Monster belongs
     * @return     absolute list of classes
     */    
    virtual Classes::ClassLevels getClassLevels() const;

    /**
     * Get basic bonus for attacking this creature. 
     * Takes into account weapon bonuses, position, type of enemy, etc.
     * @param  defender   creature being attacked
     * @return            basic bonus
     */
    virtual int getAttackBonus(CreatureH defender) const;

    /**
     * Get basic bonus for defense if attacked by this creature.
     * Takes into account dodge bonus, armour bonus, type of attacker, etc.
     * @param attacker   creature doing the attacking
     * @return           basic bonus 
     */
    virtual int getDefenseBonus(CreatureH attacker) const;  

    /**
     * Get list of damage types & amounts caused
     * @param defender   who was hit
     * @param critical   was this a critical hit?
     * @return           vector of damage types
     */
    virtual std::vector<Damage> getCombatDamage(CreatureH defender, bool critical) const;

    /**
     * Apply damage to creature
     * @param dam        damage to apply
     */
    virtual void applyDamage(Damage const & dam);

    /**
     * Is the Monster still alive/able to act? Deceased != undead/demonic
     * @return  true if monster can no longer act
     */
    virtual bool deceased() const;

    /**
     * Get the gender applicable to this monster
     * @return       Gender of monster
     */
    virtual Gender getGender() const;

    /**
     * Does the Monster have that (virtual?) skill at that level?
     * @param sk         skill to check
     * @param lev        level to check for
     * @return           true if skill exists
     */
    virtual bool hasSkill(Skills::Type sk, int lev = 1) const;


    enum Orders
    {
       GuardPosition, GuardCreature, Patrol, SeekPosition, SeekCreature,
        Wander
    };

    void setTargetPosition(Coords pos);
    Coords getTargetPosition() const;
    void updatePathToPosition();
    
};




#endif

