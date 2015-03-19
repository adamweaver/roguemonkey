#ifndef H_ITEM_
#define H_ITEM_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>

#include "handles.h"
#include "inputdef.h"
#include "species.h"
#include "stllike.h"

struct Ident
{
    enum Type
    {
        Unknown = 0,
        Basename = 1,
        Holiness = 2,
        Plusses = 4,

        All = 7,
        EndIdentStatus = 8
    };
};


struct ItemEffect
{
    enum Type
    {
        AlphaIndex,
        Equipped,
        Selected,

        PlayerID,

        EndItemEffect
    };

    int edata1;

    ItemEffect()
    {
    }

    explicit ItemEffect(int e1)
    :   edata1(e1)
    {
    }

};



/**
 * Items (Weapons, Armour, Scrolls, etc) manipulation
 */
class Item : public Describable
{
public:
    /**
     * Class of items (Weapons, Armour, Scrolls, etc)
     */
    enum ItemType
    {
        Weapon     =  1,
        Armour     =  2,
        Amulet     =  4,
        Ring       =  8,
        Scroll     =  16,
        Potion     =  32,
        Wand       =  64,
        Food       =  128,
        Gems       =  256,
        All        =  511,
        EndItem    =  512
    };

    static int const Num_Types = 9;

    static std::string const & GetItemClassName(ItemType t);

    virtual bool isBogus() const
    {
        return false;
    }

    /**
     * Create a random item of specified type
     *
     * @param t      type of item to be created
     * @return created item
     */
    static ItemH createItem(ItemType t);

    /**
     * Create an identical item
     *
     * @return       identical item to current
    */
    ItemH clone() const;

    using Describable::describe;
    std::string describe(CreatureH cr, int num, bool fulldesc) const;
    virtual std::string describeNum(CreatureH cr, int num) const = 0;

    /**
     * Number of hands required to wield
     *
     * @param str    wielder strength
     * @return       1 for 1, 2 for 2 hands, 3 for 1.5 hands
     */
    virtual int numHandsRequired(int str) const;


    /**
     * Which body slot is used by item
     *
     * @return Slot type
     */
    virtual Species::BodySlot::Type slotRequired() const = 0;


    /**
     * Get type of item
     *
     * @return type of item
     */
    virtual ItemType getItemType() const = 0;

    /**
     * Get Representation for an Item (ie '(' and Cyan for weapons)
     *
     * @return character to be displayed
     */
    virtual Representation getRepresentation() const = 0;

    /**
     * Will this item stack with itself?
     *
     * @return true for scrolls, potions, missile weapons, etc
     */
    virtual bool willStack() const = 0;

    /**
     * Get number of items in stack (ie ammunition)
     *
     * @return number of items in stack (at least 1)
     */
    int getNumber() const;

    /**
     * Adds (or subtracts) to the existing number of items in stack. Item stack is
     * deleted if zero or below
     *
     * @param num    number to add
     * @return new number of items.
     */
    int incNumber(int num);

    /**
     * Sets the number of items in stack
     *
     * @param num    new number in stack
     */
    void setNumber(int num);

    /**
     * Add an ItemEffect to this item
     *
     * @param t      Type of ItemEffect to add
     * @param e      ItemEffect to add
     */
    void addEffect(ItemEffect::Type t, ItemEffect const & e);

    /**
     * Remove an ItemEffect from this item
     *
     * @param t      Type of effect to remove
     */
    void delEffect(ItemEffect::Type t);

    /**
     * Does the current item have this effect?
     *
     * @param t      Effect to test
     *
     * @return True if exists
     */
    bool hasEffect(ItemEffect::Type t) const;

    /**
     * Get the effect from the item
     *
     * @param t      Type of effect to retrieve
     *
     * @return Chosen effect
     */
    ItemEffect & getEffect(ItemEffect::Type t) const;

    bool isEquivalent(ItemH r) const;
    bool equals(ItemH r) const;

    bool lessThan(ItemH r)
    {
        Item::ItemType lh = getItemType();
        Item::ItemType rh = r->getItemType();
        return lh < rh ? true : lh == rh ? doLessThan(r) : false;
    }

protected:
    Item();
    virtual ~Item() = 0;
    virtual ItemH doClone() const = 0;
    virtual bool doLessThan(ItemH r) const = 0;
    virtual bool doEquivalent(ItemH r) const = 0;
    virtual bool doEquals(ItemH r) const = 0;

    typedef std::map<ItemEffect::Type, ItemEffect> Effect;

    mutable Effect m_effects;
    int m_number;

private:
    static std::string const TypeString[EndItem];
};


class BogusItem : public Item
{
public:
    static ItemH getInstance();
    virtual bool isBogus() const { return true; }
    virtual Species::BodySlot::Type slotRequired() const { return Species::BodySlot::Amulet; }
    virtual ItemType getItemType() const { return Item::Weapon; }
    virtual Representation getRepresentation() const { return Representation('X', Colour::Magenta); }
    virtual std::string describe() const;
    virtual std::string describe(CreatureH viewer) const;
    virtual std::string describeIndef(CreatureH cr) const;
    virtual std::string describeIndef() const;
    virtual std::string describeNum(CreatureH viewer, int num) const;
    virtual Gender getGender() const { return Neuter; }
    virtual bool willStack() const { return false; }
    virtual std::string describeIndefinite(Ident::Type, int) const { return "Bogus #Item of Bugginess"; }
protected:
    BogusItem() {}
    virtual ItemH doClone() const { assert(!"Attempting to clone a Bogus Item"); return ItemH(); }
    virtual bool doLessThan(ItemH ) const { return false; }
    virtual bool doEquivalent(ItemH ) const { return false; }
    virtual bool doEquals(ItemH ) const { return false; }
};


typedef std::pair<bool, ItemH> ItemSuccess;

/**
 * Group of Items (ie, inventory, list of items in a Cell)
 */
class ItemPile
{
    typedef std::set<ItemH> IPile;
    IPile                   m_ipile;
    int                     m_max_size;


public:
    virtual ~ItemPile() { }

    /**
     * Type of Items to exclude from ItemPile (for creating sub-lists)
     */
    enum ExcludedType
    {
        Nil, AlreadyWorn, NotWorn, MaxExcluded
    };

    /**
     * Create a new itempile
     *
     * @param max_size          maximum number of stacks in pile
     */
    ItemPile(int max_size);

    /**
     * Checks to see if adding a new item will stack with an existing item
     *
     * @param i      Item to be checked
     * @return true if an existing stack would be incremented
     */
    bool willStack(ItemH i) const;

    /**
     * Adds an item to the current pile.
     *
     * @param item    item to be added, points to added item after insertion
     * @param num     number of items to be added (omit for all)
     * @return        true if added, false otherwise, and new item location
     */
    virtual ItemSuccess addItemToPile(ItemH item, int num = 0);

    /**
     * Removes an item from the current pile
     *
     * @param i      item to be removed
     */
    virtual void delItem(ItemH i);

    /**
     * Removes all items from the current pile
     */
    virtual void delAllItems();

    /**
     * Get a temporary pile from the current pile based on Item types.
     *
     * @param t      class of item (Weapon, Armour, etc) ORed together
     * @param ex     reasons to exclude items
     * @return       temporary ItemPile
     */
    ItemPileH getTempPileByClass(Item::ItemType t, ExcludedType ex = Nil) const;

    /**
     * Number of items in pile, counting stacks as one apiece
     *
     * @return items & stacks in pile
     */
    int numStacks() const;

    /**
     * Number of items in pile, taking stacks into account
     *
     * @return items in pile
     */
    int numItems() const;

    /**
     * Is the ItemPile empty?
     *
     * @return true if empty
     */
    bool empty() const;

    /**
     * Delete an effect from items in pile
     */
    void delAllEffect(ItemEffect::Type t);

    typedef IPile::iterator iterator;
    typedef IPile::const_iterator const_iterator;
    typedef IPile::reverse_iterator reverse_iterator;
    typedef IPile::const_reverse_iterator const_reverse_iterator;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    ItemH front() const;
    iterator find(ItemH it);
    const_iterator find(ItemH it) const;
    iterator lower_bound(ItemH it);
    const_iterator lower_bound(ItemH it) const;

    /**
     * Remove an Item from the ItemPile, designated by iterator
     *
     * @param i    iterator point to Item to remove
     */
    void erase(iterator i);

};

/**
 * Transfer an item between ItemPile
 *
 * @param  it      Item to transfer
 * @param  source  Pile to transfer from
 * @param  sink    Pile to transfer to
 * @param num      number of Item in stack to transfer (omit for all)
 * @return         true if transfer succeeded, and new item location
 */
ItemSuccess TransferItem(ItemH it, ItemPileH source, ItemPileH sink, int num = 0);


/**
 * Add entire ItemPile to current itempile
 *
 * @param source     ItemPile to transfer from - will be cleared afterwards
 * @param sink       ItemPile to transfer to
 * @return           true if fully succeeded, false if partial success or failed
 */
bool TransferAllItems(ItemPileH source, ItemPileH sink);


class ItemPileWithAlphas : public ItemPile
{
    typedef std::set<char, AlphaComparator> Alphas;
    Alphas m_alphas;
    char m_last_used;

public:
    ItemPileWithAlphas(int max_size);

   /**
     * Adds an item to the current pile.
     *
     * @param i      item to be added, points to added item after insertion
     * @param num    number of items to be added (omit for all)
     * @return       true if added, false otherwise
     */
    virtual ItemSuccess addItemToPile(ItemH i, int num = 0);

    /**
     * Removes an item from the current pile
     *
     * @param i      item to be removed
     */
    virtual void delItem(ItemH i);

    /**
     * Removes all items from the current pile
     */
    virtual void delAllItems();

};


inline Item::ItemType operator&(Item::ItemType l, Item::ItemType r)
{
    return Item::ItemType(static_cast<unsigned>(l) & static_cast<unsigned>(r));
}

inline Item::ItemType operator&(Item::ItemType l, int r)
{
    return Item::ItemType(static_cast<unsigned>(l) & static_cast<unsigned>(r));
}

inline Item::ItemType operator&(int l, Item::ItemType r)
{
    return Item::ItemType(static_cast<unsigned>(l) & static_cast<unsigned>(r));
}

inline Item::ItemType operator|(Item::ItemType l, Item::ItemType r)
{
    return Item::ItemType(static_cast<unsigned>(l) | static_cast<unsigned>(r));
}

inline Item::ItemType operator|(Item::ItemType l, int r)
{
    return Item::ItemType(static_cast<unsigned>(l) | static_cast<unsigned>(r));
}

inline Item::ItemType operator|(int l, Item::ItemType r)
{
    return Item::ItemType(static_cast<unsigned>(l) | static_cast<unsigned>(r));
}

inline Item::ItemType operator++(Item::ItemType & i)
{
    int j = i;
    j *= 2;
    i = static_cast<Item::ItemType>(j);
    return i;
}



#endif

