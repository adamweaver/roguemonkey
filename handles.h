#ifndef H_HANDLES_
#define H_HANDLES_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include "boost/enable_shared_from_this.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"

#include <string>
#include <utility>

/**
 * Version information
 */
#define VERSION "RogueMonkey v0.1"

/**
 * If wizard mode is allowd
 */
#define WIZARD 1


// http://www.parashift.com/c++-faq-lite/pointers-to-members.html
// the macro is actually a good idea
#define CALL_PTR_TO_MEMBER_FN(object, ptrToMember) ((object)->*(ptrToMember))
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))


/**
 * Characters sorted by a-z then A-Z
 * Alphabetics only can be compared by (a ^ 32) < (b ^ 32)
 */
char const AtoZ[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIKLMNOPQRSTUVWXYZ";


//==========================================================================
// Display
//==========================================================================
class Display;
typedef boost::shared_ptr<Display> DisplayH;
extern DisplayH DISPLAY;
extern unsigned int REFRESH;

//==========================================================================
// Actors, Creatures, and MOBs Handles
//==========================================================================

struct Species;
/**
 * Pointer handle for a Species
 */
typedef boost::shared_ptr<Species> SpeciesH;

class Actor;
/**
 * Pointer handle for an Actor
 */
typedef boost::shared_ptr<Actor> ActorH;

class Creature;
/**
 * Pointer handle for a Creature
 */
typedef boost::shared_ptr<Creature> CreatureH;
typedef boost::weak_ptr<Creature> CreatureWH;


class Hero;
/**
 * Pointer handle for a Hero
 */
typedef boost::shared_ptr<Hero> HeroH;
extern HeroH HERO;

class DungeonMaster;
/**
 * Pointer handle for the DungeonMaster
 */
typedef boost::shared_ptr<DungeonMaster> DungeonMasterH;

//==========================================================================
// Item Handles
//==========================================================================
class Material;

class Item;
/**
 * Pointer handle for a generic Item
 */
typedef boost::shared_ptr<Item> ItemH;
typedef boost::weak_ptr<Item> ItemWH;

class Armour;
/**
 * Pointer handle for an Armour item
 */
typedef boost::shared_ptr<Armour> ArmourH;

class Weapon;
/**
 * Pointer handle for a Weapon item
 */
typedef boost::shared_ptr<Weapon> WeaponH;

class ItemPile;
/**
 * Pointer handle for an ItemPile (group of items)
 */
typedef boost::shared_ptr<ItemPile> ItemPileH;


//============================================================================
// SDL
//============================================================================
class SDLPanel;
class SDLMapPanel;
class SDLMessagePanel;
class SDLStatPanel;
class SDLItemSelector;

typedef boost::shared_ptr<SDLPanel> SDLPanelH;
typedef boost::shared_ptr<SDLMapPanel> SDLMapPanelH;
typedef boost::shared_ptr<SDLMessagePanel> SDLMessagePanelH;
typedef boost::shared_ptr<SDLStatPanel> SDLStatPanelH;
typedef boost::shared_ptr<SDLItemSelector> SDLItemSelectorH;


//==========================================================================
// Miscellaneous Handles
//==========================================================================
class Map;
/**
 * Pointer handle for a Map
 */
typedef boost::shared_ptr<Map> MapH;


class Option;
/**
 * Pointer handle for an option file chunk
 */
typedef boost::shared_ptr<Option> OptionH;
extern OptionH OPTION;


//============================================================================
// Coords
//============================================================================

/**
 * Coords represent coordinates or locations on a map
 */
class Coords
{
    friend class Map;
    MapH setMap(MapH mp) { MapH tmp(m); m = mp; return tmp; }

    int x;
    int y;
    MapH m;

public:
    Coords(int xx, int yy) : x(xx), y(yy), m() {}
    Coords() : x(-1), y(-1), m() {}
    int X() const { return x; }
    int Y() const { return y; }
    MapH M() const { return m; }
    bool containsPoint(Coords const & rhs) const { return rhs.x >= 0 && rhs.x < x && rhs.y >= 0 && rhs.y < y; }
    bool operator<(Coords const & rh) const { return long(y) * 1024 + x < long(rh.y) * 1024 + rh.x; }
    bool operator==(Coords const & rh) const { return m == rh.m && x == rh.x && y == rh.y; }
    bool operator!=(Coords const & rh) const { return !(*this == rh); }
    Coords & operator+=(Coords const & rh)   { x += rh.x; y += rh.y; return *this; }
    Coords & operator-=(Coords const & rh)   { x -= rh.x; y -= rh.y; return *this; }
    Coords operator+(Coords const & r) const { Coords c(*this); return c += r; }
    Coords operator-(Coords const & r) const { Coords c(*this); return c -= r; }
};


//============================================================================
// Damage Type
//============================================================================
struct Damage
{
    enum Type { Slashing, Piercing, Bludgeoning, Fire, Cold, Lightning, Poison, Acid, Magic };
    Damage(Type t, int n) : type(t), num(n) {}
    Type type;
    int  num;
};

//============================================================================
// Describable
//============================================================================

enum Gender
{
    Male = 0, Female, Neuter
};

class Describable;
typedef boost::shared_ptr<Describable> DescribableH;
typedef boost::weak_ptr<Describable> DescribableWH;

class Describable : public  boost::enable_shared_from_this<Describable>,
                    private boost::noncopyable
{
public:
    virtual ~Describable() {}
    virtual std::string describe(CreatureH viewer) const = 0;
    virtual std::string describe() const = 0;
    virtual std::string describeIndef(CreatureH viewer) const = 0;
    virtual std::string describeIndef() const = 0;
    virtual Gender getGender() const = 0;
    DescribableH getDescribableHandle() { return boost::dynamic_pointer_cast<Describable>(shared_from_this()); }
};



#endif

