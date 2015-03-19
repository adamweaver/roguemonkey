#ifndef H_HERO_
#define H_HERO_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>
#include <vector>

#include "creature.h"
#include "handles.h"
#include "inputdef.h"
#include "item.h"
#include "selector.h"
#include "skills.h"
#include "species.h"

class Hero : public Creature
{
    typedef std::vector<StatPair> MyStats;
    std::string                m_name;
    SpeciesH                   m_species;
    MyStats                    m_stats;
    int                        m_guid;
    ItemPileH                  m_inventory;
    Classes                    m_classes;
    Actions::NormalMode::Type  m_last_action;

public:
    /**
     * Create a Hero per player preferences
     * @return         Newly-created Hero
     */
    static HeroH CreateHero();
    virtual ~Hero();

    virtual std::string describe(CreatureH viewer) const;
    virtual std::string describe() const;
    virtual std::string describeIndef(CreatureH cr) const;
    virtual std::string describeIndef() const;

    /**
     * Perform AI action
     * @return         Number of ticks before next action
     */
    virtual unsigned int act();

    // Creature
    /**
     * Update creature's virtual FOV
     */
    virtual void updateView();

    /**
     * Get the current/maximum for a particular stat
     * @param st     statistic to check
     * @return       pair of current & maximum stat
     */
    virtual StatPair getStat(Stat st) const;

    /**
     * Is the Creature a Hero or Monster, or ClassedMonster?
     * @return      creature class
     */
    virtual Creature::Type creatureType() const;

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
     * Get colour & character pair displayed for character
     * @param cr     Creature viewing
     *
     * @return Representation pair
     */
    virtual Representation getRepresentation(CreatureH cr) const;

    // Body stuff
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
     * Increase class level of Hero
     */
    virtual void levelUp();

    /**
     * Get Hero's default speed
     * @return       Speed
     */
    virtual Actor::Speed getSpeed() const;

    /**
     * Get the gender applicable to this hero
     * @return       Gender of hero
     */
    virtual Gender getGender() const;

    /**
     * Get the list of classes to which Hero belongs
     * @return     absolute list of classes
     */    
    Classes::ClassLevels getClassLevels() const;

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
     * Is the Hero still alive/able to act? Deceased != undead/demonic
     * @return  true if hero can no longer act
     */
    virtual bool deceased() const;

    /**
     * Print a message to the main buffer and immediately flush
     * @param str        string to print
     */
    void printImmediateMessage(std::string const & str);

    /**
     * Does the Hero have that (virtual?) skill at that level?
     * @param sk         skill to check
     * @param lev        level to check for
     * @return           true if skill exists
     */
    virtual bool hasSkill(Skills::Type sk, int lev = 1) const;

    /** 
     * Does the Hero have that virtual token?
     * @param token      token to check for (enumerated in skills.h)
     * @return           true if token is held
     */
    bool hasToken(int token) const;

    /**
     * What was the last action taken?
     * @return        last action taken - invalid for none
     */
    Actions::NormalMode::Type getLastAction() const;

private:

    // Hero
    Hero();
    void paintMap();
    void paintStats();

    /**
     * Convert 'this' into a HeroH safely
     * @return      HeroH handle for this Hero
     */   
    HeroH getHeroHandle();

    /**
     * Try to move hero in a direction
     * @param t      Action/Direction to move
     * @return Base time taken
     */
    unsigned int doMove(int);

    /**
     * Bump Hero in a direction. Auto chooses between attack/open/etc
     * @param c      Coordinates to bump
     * @return Base time taken
     */
    unsigned int doBump(Coords c);

    /**
     * Use mouse/keyboard to move cursor around to examine field
     * @return base time taken
     */
    unsigned int doLookAround(int);

    /**
     * Display info about a cell in field
     * @param  c              Coordinates of cell to examine
     * @param  max_to_print   maximum number of Items to list
     * @param  verbose        do we detail the Terrain?
     * @return                base time taken
     */
    unsigned int doLookAtPos(Coords c, int max_to_print, bool verbose);

    /**
     * Pick up item(s) from the ground
     * @return               base time taken
     */  
    unsigned int doPickUpFromGround(int);
    unsigned int doSingleDropToGround(int);
    unsigned int doMultiDropToGround(int);
    unsigned int doDrop(ItemPicker::Selected & picked);
    unsigned int doDisplayInventory(int);
    unsigned int doWield(int);
    unsigned int doWear(int);
    unsigned int doUnWear(int);
    unsigned int doShowEquipped(int);
    unsigned int doNothing(int);
    unsigned int doDisplayHelp(int);
    unsigned int doShowPreviousMessages(int);
    
    void doGainLevel();


#ifdef WIZARD
public:
    unsigned int doWizard(int);
    void         wizDisplayMap();
    void         wizGainLevel();
    
private:
#endif

    static unsigned int GetAction(Hero *hero);

    Coords doVanillaTargetting(Coords startpos, char cursor,
                               Colour::Type cursorcolour);
};






#endif

