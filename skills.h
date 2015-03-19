#ifndef H_SKILLS_
#define H_SKILLS_ 1

// -*- Mode: C++ -*-
// RogueMonkey Copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>
#include <utility>
#include <vector>

#include "dictionary.h"
#include "handles.h"


struct Token
{
    static int const IronFist = 1;
    static int const DrunkenBoxing = 2;
    static int const MetalDragon = 3;
    static int const EightWalls = 4;
    static int const Mysticism = 10;
    static int const Necromancy = 11;
    static int const Enchantment = 12;
    static int const Elementalism = 13;
};


class Classes
{
public:
    enum Type
    {
        // Melee brute. skilled with weapons styles 
        Warrior,

        // tracking dude
        Hunter,

        // melee brute. skilled with combat (regardless of weapon) & grappling
        Barbarian,

        // charm
        Bard,

        // stealth
        Rogue,

        // unarmed combat & DBZ style powerup
        Monk,

        // eldritch blaster
        Warlock,

        // teleport & illusion
        Imager,

        // animal and plant 
        Druid,

        // minor magics. create potions
        Sorceror,

        // stealthy fighter
        Assassin,

        // guardian
        Knight,

        EndClasses
    };
    typedef std::pair<Type, int> ClassLev;
    typedef std::vector<ClassLev> ClassLevels;
    typedef std::vector<int>      SkillLevels;

    void addClass(Type t);
    ClassLevels getClassLevels() const;
    int getLevelByClass(Type t) const;
    int getTotalLevel() const;
    int getHealthGranted() const;
    int getManaGranted() const;
    SkillLevels getSkillLevels() const;
    ClassLevels getAllowedClasses(HeroH hero) const;

    static int HealthGranted(Type t);
    static int ManaGranted(Type t);
    static std::string GetAbbrev(Type t);
    static std::string GetName(Type t);

    static void Init();

    Classes();


private:
    typedef Dictionary<Type, int> ClassList;

    ClassList m_class_list;
    Type      m_last_added;

};



class Skills
{
public:
    enum Points
    {
        CombatPoint, MagicPoint, SkillPoint, UnnamedPoint, EndPoints
    };


    enum Type
    {
        // Defensive
        SureFooted, Dodge, Parry, Riposte, Deflection, MovingTarget, WhirlingDefence, 

        // Weapons
        ShortBlades, Swords, Hafted, Polearms, Exotics, Wrestling, MartialArts, Bows, 
        Thrown, Crossbows,

        // Two Weapon
        TwoWeapon,

        // Martial Arts
        IronFist, DrunkenBoxing, MetalDragon, EightWalls, 

        // Short Blade Styles
        Backstab, CloseSlash,

        // Knowledge
        MonsterLore, WeaponLore, ArmourLore, Alchemy, 

        // Language
        Guilder, AlArqan, Xian,


        // Magic
        ArcaneSpark, ArcaneLore, Mysticisim, Necromancy, Enchantment, Elementalism, Sorcery,

        EndSkills
    };

    static void Init();

private:
    Type       m_skill;
    int        m_level;
};



#endif

