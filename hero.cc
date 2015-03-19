// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dice.h"
#include "dictionary.h"
#include "display.h"
#include "hero.h"
#include "item.h"
#include "map.h"
#include "species.h"
#include "textutils.h"
#include "world.h"

namespace
{
    std::ios_base::openmode SSOUT(std::ios_base::out | std::ios_base::app | std::ios_base::ate);
}


HeroH HERO;

//============================================================================
// Hero
//============================================================================
HeroH
Hero::CreateHero()
{
    return HeroH(new Hero);
}




Hero::Hero() :
    Creature(),
    m_name("Spudboy"),
    m_species(new Species(Species::Human)),
    m_stats(EndStats, Creature::StatPair(10, 10)),
    m_guid(1),
    m_inventory(new ItemPileWithAlphas(52)),
    m_classes(),
    m_last_action(Actions::NormalMode::Invalid)
{
    m_classes.addClass(Classes::Warrior);
    m_stats[Creature::Experience].first = m_stats[Creature::Experience].second = 0;
    m_stats[Creature::Health].first = m_stats[Creature::Health].second = m_classes.getHealthGranted();
    m_stats[Creature::Mana].first = m_stats[Creature::Mana].second = m_classes.getManaGranted();
}


Hero::~Hero()
{
}


std::string
Hero::describe() const
{
    return m_name;
}


std::string
Hero::describe(CreatureH) const
{
    return "hero";
}


std::string 
Hero::describeIndef(CreatureH) const
{
    return "a hero";
}


std::string 
Hero::describeIndef() const
{
    return "a hero";
}



Actions::NormalMode::Type 
Hero::getLastAction() const
{
    return m_last_action;
}


void 
Hero::printImmediateMessage(std::string const & str)
{
    DISPLAY->printMessage(str);
    DISPLAY->render();
}


bool
Hero::deceased() const
{
    return false;
}

unsigned int
Hero::GetAction(Hero *hero)
{
    typedef std::pair<Actions::NormalMode::Type, unsigned int (Hero::*)(int)> DoAction;
    typedef Dictionary<Actions::NormalMode::Type, unsigned int (Hero::*)(int)> DoActionList;

    static DoAction actions[] = 
    {
        DoAction(Actions::NormalMode::Invalid, &Hero::doNothing),
        DoAction(Actions::NormalMode::Help, &Hero::doDisplayHelp),
        DoAction(Actions::NormalMode::MoveSouthWest, &Hero::doMove),
        DoAction(Actions::NormalMode::MoveSouth, &Hero::doMove),
        DoAction(Actions::NormalMode::MoveSouthEast, &Hero::doMove),
        DoAction(Actions::NormalMode::MoveWest, &Hero::doMove),
        DoAction(Actions::NormalMode::MoveEast, &Hero::doMove),
        DoAction(Actions::NormalMode::MoveNorthWest, &Hero::doMove),
        DoAction(Actions::NormalMode::MoveNorth, &Hero::doMove),
        DoAction(Actions::NormalMode::MoveNorthEast, &Hero::doMove),
        DoAction(Actions::NormalMode::WaitHere, &Hero::doNothing),
        DoAction(Actions::NormalMode::LookAround, &Hero::doNothing),
        DoAction(Actions::NormalMode::PickUpFromGround, &Hero::doPickUpFromGround),
        DoAction(Actions::NormalMode::SingleDropToGround, &Hero::doSingleDropToGround),
        DoAction(Actions::NormalMode::MultiDropToGround, &Hero::doMultiDropToGround),
        DoAction(Actions::NormalMode::DisplayInventory, &Hero::doDisplayInventory),
        DoAction(Actions::NormalMode::DisplayEquipped, &Hero::doShowEquipped),
        DoAction(Actions::NormalMode::Wield, &Hero::doWield),
        DoAction(Actions::NormalMode::Wear, &Hero::doWear),
        DoAction(Actions::NormalMode::UnWear, &Hero::doUnWear),
#ifdef WIZARD
        DoAction(Actions::NormalMode::WizardCommand, &Hero::doWizard),
#endif
        DoAction(Actions::NormalMode::Cancel, &Hero::doNothing),
        DoAction(Actions::NormalMode::QuitGame, &Hero::doNothing),
        DoAction(Actions::NormalMode::PrevMessages, &Hero::doShowPreviousMessages),
    };
    static int const action_list_size = sizeof(actions) / sizeof(actions[0]);
    static DoActionList action_list(&actions[0], &actions[0] + action_list_size);

    unsigned int time_taken = 0U;
    while (time_taken == 0U)
    {
        Actions::NormalMode::Type action = DISPLAY->getNormalAction();
        DoActionList::iterator it = action_list.find(action);
        assert(it != action_list.end() && "Invalid NormalAction requested in Hero::act()");
        time_taken = CALL_PTR_TO_MEMBER_FN(hero, it->second)(static_cast<int>(action));
        hero->m_last_action = action;
    }
    DISPLAY->updateStats(Display::LastActionUpdate);
    DISPLAY->render();

    return time_taken;
}
    


unsigned int
Hero::act()
{
    if (REFRESH & Display::MapUpdate)
        updateView();

    if (REFRESH & ~Display::MapUpdate)
        DISPLAY->updateStats(REFRESH);

    if (REFRESH)
    {
        DISPLAY->render();
        REFRESH = 0U;
    }

    unsigned int time_taken = GetAction(this);
    return time_taken * Actor::Normal;
}


Creature::StatPair
Hero::getStat(Creature::Stat st) const
{
    return m_stats[st];
}


void
Hero::updateView()
{
    DISPLAY->drawMap();
}


Creature::Type
Hero::creatureType() const
{
    return Creature::Hero;
}


int
Hero::heroGUID() const
{
    return m_guid;
}


HeroH
Hero::getHeroHandle()
{
    return boost::dynamic_pointer_cast<Hero>(shared_from_this());
}


Representation
Hero::getRepresentation(CreatureH /*cr*/ ) const
{
    return Representation('@', Colour::White);
}


int
Hero::getSightRadius() const
{
    return 6;
}


ItemPileH
Hero::getInventory() const
{
    return m_inventory;
}


ItemH
Hero::getInvInSlot(Species::BodySlot::Type t) const
{
    return m_species->getInvInSlot(t);
}


ItemH
Hero::swapInvInSlot(Species::BodySlot::Type t, ItemH it)
{
    return m_species->swapInvInSlot(t, it);
}



void
Hero::levelUp()
{

}


Gender 
Hero::getGender() const
{
    return Female;
}


Classes::ClassLevels 
Hero::getClassLevels() const
{
    return m_classes.getClassLevels();
}


Actor::Speed
Hero::getSpeed() const
{
    return Actor::Normal;
}

unsigned int
Hero::doNothing(int n)
{
    return (n == Actions::NormalMode::WaitHere) * getSpeed();
}


unsigned int
Hero::doDisplayHelp(int)
{
    DISPLAY->displayKeyHelp(Actions::Normal);
    return 0;
}


unsigned int
Hero::doMove(int tt)
{
    Actions::NormalMode::Type t = static_cast<Actions::NormalMode::Type>(tt);
    assert(IsMove(t) && "invalid dir passed to Hero::doMove()");
    Coords adj(Actions::NormalMode::GetXMoveOffset(t),
               Actions::NormalMode::GetYMoveOffset(t));
    Coords mypos = getCoords();
    mypos += adj;
    MapH mymap = mypos.M();
    CreatureH me = getCreatureHandle();

    if (mymap->isPassable(mypos, me) && !mymap->getCreature(mypos))
    {
        mymap->moveCreature(mypos, me);
        doLookAtPos(mypos, 5, false);
        REFRESH |= Display::MapUpdate;
        return 1U;
    }

    if (!mymap->getSize().containsPoint(mypos))
    {
        // outside of boundaries
        return 0U;
    }

    return doBump(mypos);
}



unsigned int
Hero::doLookAtPos(Coords c, int max_items, bool verbose)
{
    std::ostringstream out("You see ", SSOUT);
    bool printed_items = false;

    ItemPileH items(c.M()->getItemPile(c));
    if (items.get() && !items->empty())
    {
        printed_items = true;
        int num_stacks = items->numStacks();
        if (num_stacks > max_items)
            out << "a pile of items ";
        else
        {
            ItemPile::const_iterator cit = items->begin();
            ItemPile::const_iterator cen = items->end();
            for (int i = 0; i < max_items && cit != cen; ++i, ++cit)
            {
                out << (*cit)->describeIndef(getCreatureHandle());
                out << ((i == num_stacks - 2) ? " and " : (i == num_stacks - 1) ? " " : ", ");
            }
            if (verbose)
                out << "and ";
        }
    }
    if (verbose)
        out << c.M()->getTerrainName(c);

    out << "here.";

    if (verbose || printed_items)
    {
        DISPLAY->printMessage(out.str());
        DISPLAY->render();
    }
    return 0U;
}



unsigned int
Hero::doBump(Coords c)
{
    MapH mymap = c.M();
    CreatureH me = getCreatureHandle();
    if (!mymap->isPassable(c, me))
    {
        // hit a wall or the like
        std::string cannot_pass("You can't move through ");
        cannot_pass += mymap->getTerrainName(c);
        cannot_pass += '!';
        DISPLAY->printMessage(cannot_pass);
        DISPLAY->render();
        return 0U;
    }

    // TODO: Bump combat
    CreatureH defender = mymap->getCreature(c);
    if (defender)
    {
        Fight(me, defender);
    }
    return 1U;
}



unsigned int
Hero::doLookAround(int)
{
//    Actions::NormalMode::Type action = DISPLAY->getNormalAction();
    return 0;
}


unsigned int
Hero::doPickUpFromGround(int)
{
    using namespace Actions;

    Coords mypos = getCoords();
    MapH mymap = mypos.M();
    ItemPileH items = mymap->getItemPile(mypos);
    if (!items || items->empty())
        return 0U;

    ItemPileH inventory = getInventory();
    int numitems = items->numItems();
    ItemPicker::Selected picked;

    if (numitems == 1)
    {
        items->front()->addEffect(ItemEffect::Selected, ItemEffect(items->front()->getNumber()));
        picked.push_back(items->front());
    }
    else
    {
        std::ostringstream title(COLOUR_WHITE "What do you wish to pick up? " COLOUR_LGREY "(Press ", SSOUT);
        title << DISPLAY->getKeySequence(Menu, MenuMode::Help, Colour::White, COLOUR_LGREY " or ")
              << COLOUR_LGREY " for help)";
        picked = DISPLAY->selectItemsFromPile(items, Item::All, Selector::SelectMultiple,
                                                getCreatureHandle(), title.str());
    }

    if (picked.empty())
        return 0U;

    unsigned int time_taken = 1U;
    std::ostringstream out;
    bool at_least_one = false;
    for (ItemPicker::Selected::iterator it = picked.begin(); it != picked.end(); ++it)
    {
        int num_to_xfer = (*it)->getEffect(ItemEffect::Selected).edata1;
        std::string itemname = (*it)->describeNum(getCreatureHandle(), num_to_xfer);
        ItemSuccess succ = TransferItem(*it, items, inventory, num_to_xfer);
        if (succ.first)
        {
            if (at_least_one == false)
            {
                out << "You pick up ";
                at_least_one = true;
            }
            else
            {
                out << ", ";
            }
            char alpha = (char)succ.second->getEffect(ItemEffect::AlphaIndex).edata1;
            out << alpha << " - " << itemname;
            items->delItem(*it);
            ++time_taken;
        }
        else
        {
            out << ((at_least_one) ? " but y" : "Y");
            out << "ou cannot fit any more in your backpack!";
            break;
        }
    }
    DISPLAY->printMessage(out.str());
    DISPLAY->updateStats(Display::EquipUpdate);
    DISPLAY->render();
    return time_taken;
}


unsigned int
Hero::doMultiDropToGround(int)
{
    using namespace Actions;

    Coords mypos = getCoords();
    MapH mymap = mypos.M();
    ItemPileH inventory = getInventory();
    if (!inventory || inventory->empty())
        return 0U;

    ItemPileH items = mymap->getItemPile(mypos);
    ItemPicker::Selected picked;

    std::ostringstream title(COLOUR_WHITE "What do you wish to drop? " COLOUR_LGREY "(Press ", SSOUT);
    title << DISPLAY->getKeySequence(Menu, MenuMode::Help, Colour::White, COLOUR_LGREY " or ")
          << COLOUR_LGREY " for help)";
    picked = DISPLAY->selectItemsFromPile(inventory, Item::All, Selector::SelectMultiple,
                                            getCreatureHandle(), title.str());
    return doDrop(picked);
}


unsigned int
Hero::doSingleDropToGround(int)
{
    using namespace Actions;

    Coords mypos = getCoords();
    MapH mymap = mypos.M();
    ItemPileH inventory = getInventory();
    if (!inventory || inventory->empty())
        return 0U;

    ItemPileH items = mymap->getItemPile(mypos);

    std::ostringstream title(COLOUR_WHITE "What do you wish to drop?" COLOUR_LGREY "(Press ", SSOUT);
    title << DISPLAY->getKeySequence(Menu, MenuMode::Help, Colour::White, COLOUR_LGREY " or ")
          << COLOUR_LGREY " for help)";
    ItemH pick = DISPLAY->selectItemUsingShortcut(inventory, Item::All, getCreatureHandle(),
                                                    COLOUR_WHITE "Drop what? ", title.str());
    ItemPicker::Selected picked(1, pick);
    return pick ? doDrop(picked) : 0U;
}


unsigned int
Hero::doDrop(ItemPicker::Selected & picked)
{

    if (picked.empty())
        return 0U;

    Coords mypos = getCoords();
    MapH mymap = mypos.M();
    ItemPileH items = mymap->getItemPile(mypos);
    ItemPileH inventory = getInventory();
    std::ostringstream out;
    unsigned int time_taken = 1U;
    bool at_least_one = false;

    for (ItemPicker::Selected::iterator it = picked.begin(); it != picked.end(); ++it)
    {
        Species::BodySlot::Type wieldedpos = Species::BodySlot::EndSlots;
        int num_to_xfer = (*it)->hasEffect(ItemEffect::Selected) ? 
            (*it)->getEffect(ItemEffect::Selected).edata1 : (*it)->getNumber();
        std::string itemname = (*it)->describeNum(getCreatureHandle(), num_to_xfer);

        if ((*it)->hasEffect(ItemEffect::Equipped))
        {
            wieldedpos = Species::BodySlot::Type((*it)->getEffect(ItemEffect::Equipped).edata1);
            if (wieldedpos != Species::BodySlot::WieldedR 
                && wieldedpos != Species::BodySlot::WieldedL 
                && wieldedpos != Species::BodySlot::PseudoDualWielded 
                && wieldedpos != Species::BodySlot::Launcher 
                && wieldedpos != Species::BodySlot::Quiver)
            {
                if (at_least_one)
                    out << ". ";
                out << "You need to take off your " << itemname << " before you can drop it. ";
                at_least_one = false;
                continue;
            }
        }

        ItemSuccess succ = TransferItem(*it, inventory, items, num_to_xfer);

        if (succ.first)
        {
            if (wieldedpos != Species::BodySlot::EndSlots)
            {
                swapInvInSlot(wieldedpos, ItemH());
                DISPLAY->updateStats(Display::EquipUpdate);
            }
            if (at_least_one == false)
            {
                out << "You drop your ";
                at_least_one = true;
            }
            else if (it - picked.end() != -1)
            {
                out << ", ";
            }
            else
            {
                out << " and ";
            }
            out << itemname;
            (*it)->delEffect(ItemEffect::Equipped);
            inventory->delItem(*it);
            ++time_taken;
        }
        else
        {
            out << ((at_least_one) ? ", but now y" : "Y");
            out << "ou cannot drop any more here";
            break;
        }
    }
    out << '.';
    DISPLAY->printMessage(out.str());
    DISPLAY->render();
    return time_taken;
}


unsigned int
Hero::doDisplayInventory(int)
{
    using namespace Actions;
    ItemPileH inventory = getInventory();
    int numitems = inventory->numStacks();

    if (numitems == 0)
    {
        DISPLAY->printMessage("Your backpack is empty!");
        DISPLAY->render();
        return 0U;
    }

    std::ostringstream title(COLOUR_WHITE "Your backpack contains: " COLOUR_LGREY "(Press ", SSOUT);
    title << DISPLAY->getKeySequence(Menu, MenuMode::Help, Colour::White, COLOUR_LGREY " or ")
          << COLOUR_LGREY " for help)";
    DISPLAY->selectItemsFromPile(inventory, Item::All, Selector::DisplayOnly,
                                   getCreatureHandle(), title.str());
    return 0U;
}


unsigned int
Hero::doShowEquipped(int)
{
    using namespace Actions;
    ItemPileH equipped = getInventory()->getTempPileByClass(Item::All, ItemPile::NotWorn);
    DISPLAY->selectItemsFromPile(equipped, Item::All, Selector::DisplayOnly, getCreatureHandle(),
                                   COLOUR_WHITE "You have equipped:");
    return 0U;
}


unsigned int
Hero::doWield(int)
{
    using namespace Actions;
    ItemPileH inventory = getInventory();
    if (inventory->empty())
    {
        DISPLAY->printMessage("You have nothing to wield!");
        DISPLAY->render();
        return 0U;
    }
    std::ostringstream title(COLOUR_WHITE "Wield what? " COLOUR_LGREY "(Press ", SSOUT);
    title << DISPLAY->getKeySequence(Menu, MenuMode::Help, Colour::White, COLOUR_LGREY " or ")
          << COLOUR_LGREY " for help)";
    ItemH picked = DISPLAY->selectItemUsingShortcut(inventory, Item::Weapon, getCreatureHandle(),
                                                      COLOUR_WHITE "Wield what? ", title.str());

    if (!picked || picked == getInvInSlot(Species::BodySlot::WieldedR))
        return 0U;
    if (picked->isBogus())
        picked = ItemH();

    Species::BodySlot::Type bodyslot = picked->slotRequired();
    if (bodyslot != Species::BodySlot::WieldedR &&
        bodyslot != Species::BodySlot::WieldedL &&
        bodyslot != Species::BodySlot::PseudoDualWielded &&
        bodyslot != Species::BodySlot::Launcher &&
        bodyslot != Species::BodySlot::Quiver)
    {
        bodyslot = Species::BodySlot::WieldedR;
    }
    ItemH old = swapInvInSlot(bodyslot, picked);
    std::ostringstream msg("You ", SSOUT);
    if (old)
        msg << "stop wielding " << old->describeIndef(getCreatureHandle()) << ", and now ";
    if (picked)
        msg << "wield " << picked->describeIndef(getCreatureHandle()) << ".";
    else
        msg << "are empty-handed.";

    DISPLAY->printMessage(msg.str());
    DISPLAY->updateStats(Display::EquipUpdate);
    DISPLAY->render();
    return 1U;
}


unsigned int
Hero::doWear(int)
{
    using namespace Actions;
    ItemPileH inventory = getInventory();
    if (inventory->empty())
    {
        DISPLAY->printMessage("You have nothing to wear!");
        DISPLAY->render();
        return 0U;
    }
    std::ostringstream title(COLOUR_WHITE "Wear what? " COLOUR_LGREY "(Press ", SSOUT);
    title << DISPLAY->getKeySequence(Menu, MenuMode::Help, Colour::White, COLOUR_LGREY " or ")
          << COLOUR_LGREY " for help)";
    ItemH picked = DISPLAY->selectItemUsingShortcut(inventory, Item::Armour, getCreatureHandle(),
                                                      COLOUR_WHITE "Wear what? ", title.str());
    if (!picked || picked->isBogus())
        return 0U;

    Species::BodySlot::Type slot = picked->slotRequired();
    if (slot == Species::BodySlot::WieldedR)
        slot = Species::BodySlot::WieldedL;

    ItemH old = swapInvInSlot(slot, picked);
    std::ostringstream msg("You ", SSOUT);
    if (old)
        msg << "take off " << old->describeIndef(getCreatureHandle()) << ", and now ";
    msg << "put on " << picked->describeIndef(getCreatureHandle()) << ".";
    DISPLAY->printMessage(msg.str());
    DISPLAY->updateStats(Display::EquipUpdate);
    DISPLAY->render();
    return 1U;
}


unsigned int
Hero::doUnWear(int)
{
    using namespace Actions;
    ItemPileH equipped = getInventory()->getTempPileByClass(Item::Armour, ItemPile::NotWorn);
    if (equipped->empty())
    {
        DISPLAY->printMessage("You have nothing to remove!");
        DISPLAY->render();
        return 0U;
    }
    std::ostringstream title(COLOUR_WHITE "Remove what? " COLOUR_LGREY "(Press ", SSOUT);
    title << DISPLAY->getKeySequence(Menu, MenuMode::Help, Colour::White, COLOUR_LGREY " or ")
          << COLOUR_LGREY " for help)";
    ItemH picked = DISPLAY->selectItemUsingShortcut(equipped, Item::Armour, getCreatureHandle(),
                                                      COLOUR_WHITE "Remove what? ", title.str());
    if (!picked || picked->isBogus())
        return 0U;

    Species::BodySlot::Type slot = picked->slotRequired();
    swapInvInSlot(slot, ItemH());
    std::ostringstream msg("You take off your", SSOUT);
    msg << picked->describe(getCreatureHandle()) << ".";
    DISPLAY->printMessage(msg.str());
    DISPLAY->updateStats(Display::EquipUpdate);
    DISPLAY->render();
    return 1U;
}


unsigned int
Hero::doShowPreviousMessages(int)
{
    DISPLAY->showPreviousMessages();
    return 0U;
}


int
Hero::getAttackBonus(CreatureH /*defender*/) const
{
    return 5;
}


int
Hero::getDefenseBonus(CreatureH /*attacker*/) const
{
    return 0;
}



std::vector<Damage> 
Hero::getCombatDamage(CreatureH /*defender*/, bool critical) const
{
    return std::vector<Damage>(1, Damage(Damage::Magic, 4 + 4 * critical));
}


void
Hero::applyDamage(Damage const & dam)
{
    m_stats[Creature::Health].first -= dam.num;
}


void
Hero::doGainLevel()
{
    Classes::ClassLevels allowed = m_classes.getAllowedClasses(getHeroHandle());
    
    int selectFromList(std::vector<std::string> const & vec, std::string const & title);

}


bool 
Hero::hasSkill(Skills::Type /*sk*/, int /*lev*/) const
{
    return false;
}


bool 
Hero::hasToken(int /*token*/) const
{
    return false;
}


//============================================================================
// Wizard stuff
//============================================================================
#ifdef WIZARD
namespace
{
    struct WizCmds
    {
        std::string name;
        void (Hero::*function)();
    };

    WizCmds Wizard_Commands[] = 
    {
        {"Display Map", &Hero::wizDisplayMap},
        {"Gain Level", &Hero::wizGainLevel}
        
    };
    int const num_wiz_commands = sizeof(Wizard_Commands) / sizeof(WizCmds);
    
}


unsigned int
Hero::doWizard(int)
{
    std::vector<std::string> wiz;
    for (int i = 0; i < num_wiz_commands; ++i)
        wiz.push_back(Wizard_Commands[i].name);
    int res = DISPLAY->selectFromList(wiz, COLOUR_GOLD "Select Wizard-Mode command");

    if (res > -1 && res < num_wiz_commands)
    {
        CALL_PTR_TO_MEMBER_FN(this, Wizard_Commands[res].function) ();
    }
    
    return 0U;
}




void
Hero::wizDisplayMap()
{
    DISPLAY->drawWizardMap();
}



void
Hero::wizGainLevel()
{
    doGainLevel();
}
#endif

