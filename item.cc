// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <functional>
#include <numeric>

#include "armour.h"
#include "creature.h"
#include "error.h"
#include "item.h"
#include "weapon.h"


//============================================================================
// BogusItem
//============================================================================
std::string 
BogusItem::describe(CreatureH) const
{
    return describe();
}


std::string
BogusItem::describe() const
{
    static std::string const name("bogus #item");
    return name;
}


std::string 
BogusItem::describeIndef(CreatureH) const
{
    return describe();
}


std::string 
BogusItem::describeIndef() const
{
    return describe();
}


std::string
BogusItem::describeNum(CreatureH, int) const
{
    return describe();
}


ItemH
BogusItem::getInstance()
{
    static ItemH bogus(new BogusItem);
    return bogus;
}




//============================================================================
// Item
//============================================================================
std::string const Item::TypeString[Item::EndItem] =
{
    "Weapon", "Armour", "Scroll", "Potion", "Food", "Gems", "Buggy Class"
};



std::string const &
Item::GetItemClassName(Item::ItemType t)
{
    int i = t;
    int num = 0;

    while (i /= 2)
        ++num;


    return TypeString[num];
}


ItemH
Item::createItem(Item::ItemType t)
{
    switch (t)
    {
    case Item::Weapon:
        return ItemH(Weapon::createWeapon());

    case Item::Armour:
    default:
        return ItemH(Armour::createArmour());
    }

}


Item::Item()
:   m_effects(),
    m_number(1)
{
}


Item::~Item()
{
}


ItemH
Item::clone() const
{
    ItemH item(doClone());
    item->m_effects = m_effects;
    item->m_number = m_number;
    return item;
}


std::string
Item::describe(CreatureH cr, int num, bool fulldesc) const
{
    Ident::Type ident;

    if (cr->heroGUID())
    {
        ident = hasEffect(ItemEffect::PlayerID) ? Ident::Type(getEffect(ItemEffect::PlayerID).edata1) : Ident::Unknown;
    }
    else
        ident = Ident::All;

    std::string summary = describeNum(cr, !num ? getNumber() : num);
    if (fulldesc && hasEffect(ItemEffect::Equipped))
    {
        switch (getEffect(ItemEffect::Equipped).edata1)
        {
        case Species::BodySlot::RingsL:
            summary.append(" (left hand)");
            break;
        case Species::BodySlot::RingsR:
            summary.append(" (right hand)");
            break;
        case Species::BodySlot::WieldedL:
            summary.append(" (wielded left hand)");
            break;
        case Species::BodySlot::WieldedR:
            summary.append(" (wielded right hand)");
            break;
        case Species::BodySlot::PseudoDualWielded:
            summary.append(" (wielded both hands)");
            break;
        case Species::BodySlot::Quiver:
        case Species::BodySlot::Launcher:
            summary.append(" (readied)");
            break;
        default:
            summary.append(" (worn)");
            break;
        }
    }
    return summary;
}


int
Item::getNumber() const
{
    return m_number;
}


int
Item::incNumber(int num)
{
    m_number += num;
    return m_number;
}


void
Item::setNumber(int num)
{
    if (!num)
        num = 1;
    m_number = num;
}


bool
Item::isEquivalent(ItemH r) const
{
    return getItemType() == r->getItemType() ? doEquivalent(r) : false;
}


bool
Item::equals(ItemH r) const
{
    return getItemType() == r->getItemType() ? doEquals(r) : false;
}


void
Item::addEffect(ItemEffect::Type t, ItemEffect const & e)
{
    m_effects[t] = e;
}


void
Item::delEffect(ItemEffect::Type t)
{
    m_effects.erase(t);
}


bool
Item::hasEffect(ItemEffect::Type t) const
{
    return m_effects.find(t) != m_effects.end();
}


ItemEffect &
Item::getEffect(ItemEffect::Type t) const
{
    Effect::iterator it = m_effects.find(t);
    if (it != m_effects.end())
    {
        return it->second;
    }
    throw Error<NotFoundE>("Item does not contain this effect");
}


int
Item::numHandsRequired(int /*str*/) const
{
    return 1;
}




//============================================================================
// ItemPile
//============================================================================

ItemPile::ItemPile(int maxsize)
:   m_ipile(),
    m_max_size(maxsize)
{
}

ItemPile::iterator
ItemPile::find(ItemH item)
{
    return m_ipile.find(item);
}


ItemPile::const_iterator
ItemPile::find(ItemH item) const
{
    return m_ipile.find(item);
}


ItemPile::iterator
ItemPile::lower_bound(ItemH item)
{
    return m_ipile.lower_bound(item);
}


ItemPile::const_iterator
ItemPile::lower_bound(ItemH item) const
{
    return m_ipile.lower_bound(item);
}


bool
ItemPile::willStack(ItemH item) const
{
    return item->willStack() && find(item) != end();
}


ItemSuccess
ItemPile::addItemToPile(ItemH item, int num)
{
    if (static_cast<int>(m_ipile.size()) == m_max_size && !willStack(item))
        return ItemSuccess(false, item);

    iterator ci = lower_bound(item);
    if (ci != end() && item->isEquivalent(*ci) && item->willStack())
    {
        if (num > item->getNumber() || num == 0)
            num = item->getNumber();
        (*ci)->incNumber(num);
        item->incNumber(-num);
        return ItemSuccess(true, *ci);
    }
    m_ipile.insert(ci, item);
    return ItemSuccess(true, item);
}


ItemSuccess
TransferItem(ItemH item, ItemPileH source, ItemPileH sink, int num)
{
    ItemPile::iterator iter = source->find(item);
    assert(iter != source->end() && item->equals(*iter) && "Trying to transfer Item from wrong ItemPile");
    int num_items = item->getNumber();
    if (num > num_items || num == 0)
    {
        num = item->getNumber();
        ItemSuccess succ = sink->addItemToPile(item);
        if (succ.first)
            source->erase(iter);
        return succ;
    }
    ItemH clone(item->clone());
    clone->setNumber(num);
    ItemSuccess succ = sink->addItemToPile(clone);
    if (succ.first)
        item->incNumber(-num);
    return succ;
}


bool
TransferAllItems(ItemPileH source, ItemPileH sink)
{
    bool success = true;
    for (ItemPile::iterator iter = source->begin(); iter != source->end(); ++iter)
    {
        ItemSuccess succ = TransferItem(*iter, source, sink);
        if (success)
            success = succ.first;
    }
    return success;
}


void
ItemPile::delItem(ItemH item)
{
    m_ipile.erase(item);
}


void
ItemPile::delAllItems()
{
    m_ipile.clear();
}


ItemPileH
ItemPile::getTempPileByClass(Item::ItemType t, ItemPile::ExcludedType ex) const
{
    ItemPileH tmp(new ItemPile(52));

    for (const_iterator ib = begin(); ib != end(); ++ib)
    {
        if ((*ib)->getItemType() & t)
        {
            switch (ex)
            {
            case AlreadyWorn:
                if (!(*ib)->hasEffect(ItemEffect::Equipped))
                    tmp->m_ipile.insert(*ib);
                break;

            case NotWorn:
                if ((*ib)->hasEffect(ItemEffect::Equipped))
                    tmp->m_ipile.insert(*ib);
                break;

            case Nil:
            default:
                tmp->m_ipile.insert(*ib);
                break;
            }
        }
    }
    return tmp;
}


void
ItemPile::delAllEffect(ItemEffect::Type t)
{
    for (iterator it = begin(); it != end(); ++it)
        (*it)->delEffect(t);
}


int
ItemPile::numStacks() const
{
    return static_cast<int>(m_ipile.size());
}


namespace
{
    struct CountItems : public std::binary_function<int, ItemH, int>
    {
        int operator() (int l, ItemH const & r) const
        {
            return r->getNumber() + l;
        }
    };
}


int
ItemPile::numItems() const
{
    return std::accumulate(m_ipile.begin(), m_ipile.end(), 0, CountItems());
}


bool
ItemPile::empty() const
{
    return m_ipile.size() == 0;
}


ItemPile::iterator
ItemPile::begin()
{
    return m_ipile.begin();
}


ItemPile::const_iterator
ItemPile::begin() const
{
    return m_ipile.begin();
}


ItemPile::iterator
ItemPile::end()
{
    return m_ipile.end();
}


ItemPile::const_iterator
ItemPile::end() const
{
    return m_ipile.end();
}

ItemPile::reverse_iterator
ItemPile::rbegin()
{
    return m_ipile.rbegin();
}

ItemPile::const_reverse_iterator
ItemPile::rbegin() const
{
    return m_ipile.rbegin();
}


ItemPile::reverse_iterator
ItemPile::rend()
{
    return m_ipile.rend();
}


ItemPile::const_reverse_iterator
ItemPile::rend() const
{
    return m_ipile.rend();
}


ItemH
ItemPile::front() const
{
    return *m_ipile.begin();
}


void
ItemPile::erase(ItemPile::iterator i)
{
    m_ipile.erase(i);
}

//============================================================================
// ItemPileWithAlphas
//============================================================================
ItemPileWithAlphas::ItemPileWithAlphas(int maxsize) :
    ItemPile(maxsize),
    m_alphas(&AtoZ[0], &AtoZ[0] + sizeof(AtoZ) - 1, AlphaComparator()),
    m_last_used(0)
{
}


ItemSuccess
ItemPileWithAlphas::addItemToPile(ItemH item, int num)
{
    ItemSuccess success = ItemPile::addItemToPile(item, num);
    if (success.first)
    {
        // do we not have an alpha or is the current alpha already taken?
        if (item->hasEffect(ItemEffect::AlphaIndex) == false ||
            m_alphas.find(item->getEffect(ItemEffect::AlphaIndex).edata1) == m_alphas.end())
        {
            // yes. we need to find an available alpha
            Alphas::iterator ait = m_alphas.lower_bound(m_last_used);

            if (ait == m_alphas.end())
                ait = m_alphas.begin();

            m_last_used = *ait;
            m_alphas.erase(m_last_used);
            ItemEffect eff(m_last_used++);

            if (m_last_used == 'z' + 1)
                m_last_used = 'A';
            else if (m_last_used == 'Z' + 1)
                m_last_used = 'a';

            item->addEffect(ItemEffect::AlphaIndex, eff);
        }
        else
        {
            // remove the alpha from the available-list
            m_alphas.erase(item->getEffect(ItemEffect::AlphaIndex).edata1);
        }
    }
    return success;
}


void
ItemPileWithAlphas::delItem(ItemH i)
{
    ItemPile::delItem(i);
    m_alphas.insert(i->getEffect(ItemEffect::AlphaIndex).edata1);
}


void
ItemPileWithAlphas::delAllItems()
{
    ItemPile::delAllItems();
    m_alphas.insert(&AtoZ[0], &AtoZ[0] + sizeof(AtoZ));
}

