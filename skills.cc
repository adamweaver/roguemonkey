// -*- Mode: C++ -*-
// RogueMonkey Copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "dice.h"
#include "hero.h"
#include "skills.h"
#include "stllike.h"

namespace 
{
    struct Prereq
    {
        enum Type    { Skill, Class, Token };
        Prereq(Skills::Type sk, int lev) : type(Skill), prereq(sk), level(lev) {}
        Prereq(Classes::Type cl, int lev) : type(Class), prereq(cl), level(lev) {}
        Prereq(int token) : type(Token), prereq(Skills::EndSkills), level(token) {}

        Type     type;
        int      prereq;
        int      level;
    };
    typedef std::vector<Prereq> Prereqs; 
}

//============================================================================
// Class
//============================================================================
namespace
{
    struct ClassStats
    {
        std::string          health_per_level;
        std::string          mana_per_level;  
        std::string          name;
        std::string          abbrev;
        std::string          description;
        Classes::SkillLevels skills_per_level;
        Prereqs              prereqs;

        ClassStats(std::string hp, std::string mana, std::string nm, std::string abb) :
            health_per_level(hp),   
            mana_per_level(mana),   
            name(nm),   
            abbrev(abb),   
            description(),   
            skills_per_level(Skills::EndPoints),
            prereqs()
        {
        }

        ClassStats & SkillsPerLevel(int combat, int magic, int skill, int misc)
        { 
            skills_per_level[0] = combat; 
            skills_per_level[1] = magic;
            skills_per_level[2] = skill;
            skills_per_level[3] = misc; 
            return *this; 
        };

        ClassStats & Description(std::string const & desc)
        { description = desc; return *this; }

        ClassStats & Prerequisite(Skills::Type sk, int l)
        { prereqs.push_back(Prereq(sk, l)); return *this; }

        ClassStats & Prerequisite(Classes::Type cl, int l)
        { prereqs.push_back(Prereq(cl, l)); return *this; }

        ClassStats & Prerequisite(int tok)
        { prereqs.push_back(Prereq(tok)); return *this; }
    };

    ClassStats class_stats[] = 
    {
        // Warrior
        ClassStats("1d8+1m5", "1d2-1", "warrior", "warr").
        Description("Warriors are the masters of the armed fighting arts. Warriors are extremely deadly with "
                    "their chosen weapon, but much less a threat in its absence. ").
        SkillsPerLevel(3, 0, 0, 1),

        // Hunter
        ClassStats("2d4-1", "1", "hunter", "hntr").
        SkillsPerLevel(1, 0, 2, 1),

        // Barbarian,
        ClassStats("1d8+2m5", "1", "barbarian", "brbn").
        SkillsPerLevel(2, 0, 2, 1),

        // Bard
        ClassStats("1d6", "1d2", "bard", "bard").
        SkillsPerLevel(0, 0, 2, 4),
       
        // Rogue,
        ClassStats("1d6", "1d2", "rogue", "rogu").
        SkillsPerLevel(1, 0, 3, 1),

        // Monk,
        ClassStats("3d2", "1d4", "monk", "monk").
        SkillsPerLevel(2, 1, 0, 1).
        Prerequisite(Token::Mysticism).
        Prerequisite(Skills::ArcaneSpark),

        // Warlock,
        ClassStats("1d4", "2d4", "warlock", "wrlk").
        SkillsPerLevel(0, 3, 0, 1).
        Prerequisite(Token::Necromancy).
        Prerequisite(Skills::ArcaneSpark),

        // Imager,
        ClassStats("1d4", "2d4", "imager", "imgr").
        SkillsPerLevel(0, 3, 0, 1).
        Prerequisite(Token::Enchantment).
        Prerequisite(Skills::ArcaneSpark),

        // Druid,
        ClassStats("1d4", "2d4", "druid", "drui").
        SkillsPerLevel(0, 3, 1, 1).
        Prerequisite(Token::Elementalism).
        Prerequisite(Skills::ArcaneSpark),

        // Sorceror,
        ClassStats("1d4", "1d6+1d4", "sorceror", "sorc").
        SkillsPerLevel(0, 3, 1, 1).
        Prerequisite(Skills::ArcaneSpark),

        // Assassin,
        ClassStats("1d6", "1", "assassin", "assn").
        SkillsPerLevel(2, 0, 1, 1),

        // Knight,
        ClassStats("1d8+1", "1", "knight", "knig").
        SkillsPerLevel(2, 0, 0, 1)
    };

    struct ClassLevSorter : public std::binary_function<Classes::ClassLev, Classes::ClassLev, bool>
    {
        bool operator() (Classes::ClassLev l, Classes::ClassLev r)
        {
            return l.second < r.second;
        }
    };
    
}


Classes::Classes() :
    m_class_list(),
    m_last_added()
{
}


Classes::ClassLevels
Classes::getAllowedClasses(HeroH hero) const
{
    ClassLevels current = getClassLevels();
    ClassLevels allowed(current);

    // Zeroeth level has all classes open
    if (current.empty())
    {
        for (int i = 0; i < EndClasses; ++i)
            allowed.push_back(ClassLev(Type(i), 0));
        return allowed;
    }

    // put the last-added class right at the top
    ClassLevels::iterator it = std::find_if(allowed.begin(), allowed.end(), FirstEquals<ClassLev>(m_last_added));
    assert(it != allowed.end() && "Last Added Class not updated");
    ClassLev last(*it);
    allowed.erase(it);
    allowed.insert(allowed.begin(), last);

    // Now trawl through all classes to work out the available choices
    for (int i = 0; i < EndClasses; ++i)
    {
        // are we already there? it's always ok to advance in an existing class
        if (std::find_if(current.begin(), current.end(), FirstEquals<ClassLev>(Type(i))) != current.end())
            continue;

        bool meets_prereq = true;
        for (Prereqs::const_iterator p = class_stats[i].prereqs.begin();
             meets_prereq && p != class_stats[i].prereqs.end(); ++p)
        {
            switch (p->type)
            {
            case Prereq::Skill:
                if (!hero->hasSkill(Skills::Type(p->prereq), p->level))
                    meets_prereq = false;
                break;
            case Prereq::Class:
                it = std::find_if(current.begin(), current.end(), FirstEquals<ClassLev>(Type(p->prereq)));
                if (it == current.end() || it->second < p->level)
                    meets_prereq = false;
                break;
            case Prereq::Token:
                if (!hero->hasToken(p->prereq))
                    meets_prereq = false;
                break;
            default:
                assert(!"Unhandled class prerequisite type");
            }
        }
        if (meets_prereq)
            allowed.push_back(ClassLev(Type(i), 0));
    }

    return allowed;
}



Classes::ClassLevels
Classes::getClassLevels() const
{
    ClassLevels nc(m_class_list.begin(), m_class_list.end());
    std::sort(nc.begin(), nc.end(), ClassLevSorter());
    return nc;
}


void 
Classes::addClass(Type t)
{
    m_class_list[t]++;
    m_last_added = t;
}


int 
Classes::getLevelByClass(Type t) const
{
    ClassList::const_iterator c = m_class_list.find(t);
    return c == m_class_list.end() ? 0 : c->second;
}


int 
Classes::getTotalLevel() const
{
    int num = 0;
    for (ClassList::const_iterator c = m_class_list.begin(); c != m_class_list.end(); ++c)
        num += c->second;
    return num;
}


int 
Classes::getHealthGranted() const
{
    return Dice::Random(class_stats[m_last_added].health_per_level);
}


int 
Classes::getManaGranted() const
{
    return Dice::Random(class_stats[m_last_added].mana_per_level);
}


int 
Classes::HealthGranted(Type t)
{
    return Dice::Random(class_stats[t].health_per_level);
}


int 
Classes::ManaGranted(Type t)
{
    return Dice::Random(class_stats[t].mana_per_level);
}


std::string
Classes::GetAbbrev(Type t)
{
    return class_stats[t].abbrev;
}


std::string
Classes::GetName(Type t)
{
    return class_stats[t].name;
}


void
Classes::Init()
{
}


//============================================================================
// Skills
//============================================================================
namespace
{
    struct SkillStats
    {
        typedef std::vector<int>    SkillCost;

        std::string    name;
        std::string    shortdesc;
        std::string    longdesc;
        int            max_levels;
        Prereqs        prerequisites;
        SkillCost      cost;
        
        SkillStats(std::string const & n, int ml, std::string const & sd) :
            name(n),
            shortdesc(sd),
            longdesc(""),
            max_levels(ml),
            prerequisites(),
            cost(3, 1)
        {
        }
        
        SkillStats & LongDesc(std::string const & d)
        { longdesc = d; return *this; }

        SkillStats & Prerequisite(Skills::Type sk, int l)
        { prerequisites.push_back(Prereq(sk, l)); return *this; }

        SkillStats & Prerequisite(Classes::Type cl, int l)
        { prerequisites.push_back(Prereq(cl, l)); return *this; }

        SkillStats & Prerequisite(int tok)
        { prerequisites.push_back(Prereq(tok)); return *this; }

        SkillStats & Cost(int combat, int magic, int skill)
        { cost[0] = combat; cost[1] = magic; cost[2] = skill; return *this; };
    };

    SkillStats skill_stats[Skills::EndSkills] =
    {
        SkillStats("Sure Footed", 3, "Difficult terrain provides less of a hinderance to movement").
        Cost(0, 0, 2),

        SkillStats("Dodge", 5, "You are &s at avoiding attacks").
        Cost(1, 0, 1),

        SkillStats("Parry", 3, "You are more easily able to block incoming blows").
        Cost(2, 0, 0),

        SkillStats("Riposte", 1, "You gain an immediate counter attack upon a successful block").
        Prerequisite(Skills::Parry, 2).
        Cost(2, 0, 0),

        SkillStats("Deflection", 1, "You are able to block incoming missiles with your weapon").
        Cost(2, 0, 0),

        SkillStats("Moving Target", 1, "You are harder to hit by a missile attack if your last "
                   "action was a move or attack").
        Prerequisite(Skills::Dodge, 3).
        Cost(2, 0, 1),

        SkillStats("Whirling Defence", 1, "You are more easily able to defend yourself from "
                   "multiple attackers").
        Prerequisite(Skills::Parry, 1).
        Cost(3, 0, 0),
        
        SkillStats("Short Blades", 20, "You are &s with the use of a dagger or other short blade").
        Cost(1, 0, 0),

        SkillStats("Swords", 20, "You are &s with the use of swords").
        Cost(1, 0, 0),

        SkillStats("Hafted Weapons", 20, "You are &s with the use of maces, flails, axes, and "
                   "other hafted weapons").
        Cost(1, 0, 0),

        SkillStats("Polearms", 20, "You are &s with the use of longer weapons to keep enemies at bay").
        Cost(1, 0, 0),

        SkillStats("Exotics", 20, "You are &s in the art of using unusual and rare weapons").
        Cost(1, 0, 0),

        SkillStats("Wrestling", 20, "You are &s in the art of grappling and pinning your foes").
        Cost(1, 0, 0),

        SkillStats("Martial Arts", 15, "You are &s in the art of unarmed strikes").
        Cost(1, 0, 0),

        SkillStats("Bows", 20, "You are &s with the use of bows").
        Cost(1, 0, 0),

        SkillStats("Thrown Weapons", 15, "You are &s in the use of hand-thrown weapons "
                   "and improvised projectiles").
        Cost(1, 0, 0),

        SkillStats("Crossbows & Gunnes", 10, "You are &s with the use of crossbows and gunnes").
        Cost(1, 0, 0),

        SkillStats("Two Weapon Combat", 15, "You are &s in the use of dual weapon fighting techniques").
        Cost(2, 0, 0),

        SkillStats("Iron Fist Style", 10, "You are &s in the Iron Fist discipline.").
        LongDesc("Iron Fist style is focused towards hard attacks. Less fluid and "
                 "defensive than sister styles, Iron Fist proponents firmly believe that "
                 "the most effective defence is an overwhelming offence.").
        Prerequisite(Skills::MartialArts, 5).
        Prerequisite(Token::IronFist).
        Cost(1, 0, 1),

        SkillStats("Drunken Boxing", 10, "You are &s in Drunken Boxing.").
        LongDesc("Drunken Boxing is an extremely unpredictable style with an equal focus on "
                 "offence and defence. Proponents dodge and weave erratically and can be "
                 "difficult to hit.").
        Prerequisite(Skills::MartialArts, 4).
        Prerequisite(Skills::Dodge, 1).
        Prerequisite(Token::DrunkenBoxing).
        Cost(1, 0, 1),
        
        SkillStats("Metal Dragon Style", 10, "You are &s in the Metal Dragon discipline.").
        LongDesc("Metal Dragon style is focused towards armourd defence. Requiring less "
                 "freedom of movement, practitioners can enjoy the defensive benefits of "
                 "armour and shielding, and still strike opponents.").
        Prerequisite(Skills::MartialArts, 4).
        Prerequisite(Skills::ArmourLore).
        Prerequisite(Token::MetalDragon).
        Cost(1, 0, 1),
        
        SkillStats("Eight Walls Style", 10, "You are & in the Eigh Walls discipline.").
        LongDesc("Eight Walls style is focused towards total defence from all directions. "
                 "Practitioners can both defend themselves ably from attack, and use their "
                 "attackers' own momentum to attack").
        Prerequisite(Skills::MartialArts, 4).
        Prerequisite(Skills::Parry).
        Prerequisite(Token::EightWalls).
        Cost(1, 0, 1),

        SkillStats("Backstabbing", 1, "You are able to strike the vital points of unaware foes for great damage").
        Prerequisite(Skills::ShortBlades, 3).
        Cost(2, 0, 0),

        SkillStats("Close Infighting", 1, "You are able to more easily strike grappled opponents").
        Prerequisite(Skills::ShortBlades, 3).
        Prerequisite(Skills::Wrestling, 3).
        Cost(2, 0, 0),

        SkillStats("Monster Lore", 5, "You are &s in your knowledge of monsters and other beings").
        Cost(0, 0, 2),

        SkillStats("Weapon Lore", 3, "You are &s in your knowledge of weapons").
        Cost(1, 0, 1),

        SkillStats("Armour Lore", 3, "You are &s in your knowledge of armourings").
        Cost(1, 0, 1),

        SkillStats("Alchemy", 10, "You are &s in your knowledge of potions, philtres, and magical concoctions").
        Cost(0, 1, 1),

        SkillStats("Language - Guilder", 3, "You have a &s grasp of the Guildanian tongue").
        Cost(0, 0, 2),

        SkillStats("Language - Al'Arqa", 3, "You have a &s grasp of the Al'Arqan tongue").
        Cost(0, 0, 2),

        SkillStats("Language - Xian", 3, "You have a &s grasp of the Xian tongue").
        Cost(0, 0, 2),

        SkillStats("Arcane Spark", 1, "You have discovered talent for magical power").
        Cost(0, 3, 0),

        SkillStats("Arcane Lore", 10, "You are &s in your magical abilities").
        Prerequisite(Skills::ArcaneSpark).
        Cost(0, 2, 0), 

        SkillStats("Mysticism", 20, "You are &s in innermost reflection and concentration of power").
        Prerequisite(Skills::ArcaneSpark).
        Prerequisite(Token::Mysticism).
        Cost(0, 1, 0),

        SkillStats("Necromancy", 20, "You are &s in your study of forgotten magics").
        Prerequisite(Skills::ArcaneSpark).
        Prerequisite(Token::Necromancy).
        Cost(0, 1, 0),

        SkillStats("Enchantment", 20, "You are &s in your study of magical effect on the world").
        Prerequisite(Skills::ArcaneSpark).
        Prerequisite(Token::Enchantment).
        Cost(0, 1, 0),

        SkillStats("Elementalism", 20, "You are &s in your study of natural forces").
        Prerequisite(Skills::ArcaneSpark).
        Prerequisite(Token::Elementalism).
        Cost(0, 1, 0),

        SkillStats("Sorcery", 25, "You have a &s knack for sorcerous magic").
        Prerequisite(Skills::ArcaneSpark).
        Cost(0, 1, 0)
        // No Token required - Sorcery is open to most by its very nature.

       

    };

}
