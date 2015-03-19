// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "creature.h"
#include "display.h"
#include "error.h"
#include "fov.h"
#include "hero.h"
#include "item.h"
#include "option.h"
#include "textutils.h"

 //============================================================================
// Display and friends
//============================================================================

DisplayH DISPLAY;
unsigned int REFRESH = Display::AllUpdate;

int Display::StatPos::Name_pos       = 0;          
int Display::StatPos::Location_pos   = 1;          
int Display::StatPos::Health_pos     = 4;          
int Display::StatPos::Mana_pos       = 5;          
int Display::StatPos::Exp_pos        = 7;          
int Display::StatPos::Class_pos      = 8;          
int Display::StatPos::Stats_pos      = 11;         
int Display::StatPos::Cond_pos       = 16;         
int Display::StatPos::Equip_pos      = 19;         
int Display::StatPos::Action_pos     = 24;       


struct Display::ActionLookup
{
    typedef std::pair<int, std::string> ActionPair;
    typedef std::vector<Input::KeyPress> KeyPresses;
    typedef Dictionary<KeyPresses, int> Lookup;
    typedef std::multimap<int, std::string> HumanLookup;

    std::vector<Lookup> alookup;
    std::vector<HumanLookup> hlookup;
};

namespace
{
    unsigned char const ControlSentinel = 200;
    unsigned char const MetaSentinel = 201;
    unsigned char const AltSentinel = 202;

    struct AbbKeyHum {char const *abbrev; int key; char const *human;};
    AbbKeyHum a2k[] =
    {
        {"@PAGEDOWN", Input::PageDown, "<Page Down>"},
        {"@PAGEUP", Input::PageUp, "<Page Up>"}, {"@HOME", Input::Home, "<Home>"},
        {"@END", Input::End, "<End>"}, {"@ESCAPE", Input::Esc, "<Esc>"},
        {"@ENTER", Input::Enter, "<Enter>"}, {"@DEL", Input::Delete, "<Delete>"},
        {"@TAB", Input::Tab, "<Tab>"}, {"@KP1", Input::KP1, "<Keypad 1>"},
        {"@KP2", Input::KP2, "<Keypad 2>"}, {"@KP3", Input::KP3, "<Keypad 3>"},
        {"@KP4", Input::KP4, "<Keypad 4>"}, {"@KP5", Input::KP5, "<Keypad 5>"},
        {"@KP6", Input::KP6, "<Keypad 6>"}, {"@KP7", Input::KP7, "<Keypad 7>"},
        {"@KP8", Input::KP8, "<Keypad 8>"}, {"@KP9", Input::KP9, "<Keypad 9>"},
        {"@KP0", Input::KP0, "<Keypad 0>"}, {"@F1", Input::F1, "<Function 1>"},
        {"@F2", Input::F2, "<Function 2>"}, {"@F3", Input::F3, "<Function 3>"},
        {"@F4", Input::F4, "<Function 4>"}, {"@F5", Input::F5, "<Function 5>"},
        {"@F6", Input::F6, "<Function 6>"}, {"@F7", Input::F7, "<Function 7>"},
        {"@F8", Input::F8, "<Function 8>"}, {"@F9", Input::F9, "<Function 9>"},
        {"@F10", Input::F10, "<Function 10>"}, {"@F11", Input::F11, "<Function 11>"},
        {"@F12", Input::F12, "<Function 12>"}, {"@UP", Input::Up, "<Cursor Up>"},
        {"@DOWN", Input::Down, "<Cursor Down>"},  {"@LEFT", Input::Left, "<Cursor Left>"},
        {"@RIGHT", Input::Right, "<Cursor Right>"}, {"@SPACE", ' ', "Spacebar"},
        {"@@", '@', "@"}, {"@CTRL-", Input::Control, "<Control>"},
        {"@META-", Input::Meta, "<Meta>"}, {"@ALT-", Input::Alt, "<Alt>"}
    };

    std::size_t NA2K = sizeof(a2k) / sizeof(a2k[0]);
    typedef std::vector<Input::KeyPress> KeyList;
    typedef std::pair<KeyList, std::string> KeyHuman;

    KeyHuman GetKeySequence(std::string const & seq)
    {
        KeyHuman key;
        Input::KeyPress kp(0);
        std::string::const_iterator strit = seq.begin();

        while (strit != seq.end())
        {
            if (*strit != '@')
            {
                kp.data.key = *strit;
                key.first.push_back(kp);
                key.second += *strit++;
                kp.data.key = 0;
                kp.meta = 0;
                continue;
            }

            bool found = false;
            for (AbbKeyHum *spec = &a2k[0]; !found && spec != &a2k[0] + NA2K; ++spec)
            {
                // we have a match
                int len = static_cast<int>(std::strlen(spec->abbrev));
                if (seq.compare(0, len, spec->abbrev) == 0)
                {
                    // have a meta
                    if (spec->abbrev[seq.size() - 1] == '-')
                        kp.meta |= spec->key;
                    else
                    {
                        kp.data.key = spec->key;
                        key.first.push_back(kp);
                        kp.data.key = 0;
                        kp.meta = 0;
                    }

                    key.second += spec->human;
                    strit += len;
                    found = true;
                }
            }
            if (!found)
            {
                Error<FormatE> err;
                err << "Unrecognised special token(s) '"
                    << std::string(strit, seq.end()) << "' for action: ";
                throw err;
            }
        }

        if (kp.meta)
        {
            Error<FormatE> err;
            err << "Did you forget a key to follow modifier? Input sequence '"
                << key.second << "' invalid for action: ";
            throw err;
        }

        return key;
    }


    void ReadAndSplit(int end, OptionH tree, char const *names[],
                      Display::ActionLookup::Lookup & lookup,
                      Display::ActionLookup::HumanLookup & hlookup)
    {
        typedef std::vector<std::string> VecS;

        for (int i = 0; i < end; ++i)
        {
            char const *actname = names[i];
            if (!actname || !actname[0])
                continue;

            std::string sequence = tree->get(actname);
            VecS seqs = TextUtils::Tokenise(sequence, " ", "\\", "\"");

            for (VecS::iterator ci = seqs.begin(); ci != seqs.end(); ++ci)
            {
                try
                {
                    KeyHuman p = GetKeySequence(*ci);
                    for (Display::ActionLookup::Lookup::iterator it = lookup.begin();
                         it != lookup.end(); ++it)
                    {
                        if (it->first.size() <= p.first.size() &&
                            std::equal(it->first.begin(), it->first.end(), p.first.begin()))
                        {
                            Error<FormatE> err;
                            err << "Key sequence  for action: " << names[it->second]
                                << " hides (or vice-versa) key sequence '"
                                << p.second << "' for action: ";
                            throw err;
                        }
                    }

                    lookup.insert(p.first, i);
                    hlookup.insert(std::make_pair(i, p.second));
                }
                catch (Error<FormatE> & /*e*/)
                {
                    // TODO: Proper logging
                    throw;
                }

            } // end for vector iterator

        } // end for option names
    }


    void DeleteFOVSettings(fov_settings_type *fov)
    {
        fov_settings_free(fov);
    }

    void HeroLightCell(void *mp, int x, int y, int /*dx*/, int /*dy*/, void * /*src*/)
    {
        Map *mymap = static_cast<Map *>(mp);
        Coords mapsize = mymap->getSize();

        if (x >= 0 && y >= 0 && x < mapsize.X() && y < mapsize.Y())
        {
            mymap->setSeenGrid(x, y, Map::Lit);
            mymap->setHeroSeenChar(x, y);
        }
    }

    bool HeroIsNotOpaque(void *mp, int x, int y)
    {
        Map *mymap = static_cast<Map *>(mp);
        Coords mapsize = mymap->getSize();
        if (x < 0 || y < 0 || x >= mapsize.X() || y >= mapsize.Y())
            return false;
        return !mymap->blocksVision(Coords(x, y), HERO);
    }

}



Display::Display(OptionH opt) :
    m_fov_settings(new fov_settings_type, DeleteFOVSettings),
    m_actionlookup(new ActionLookup)
{
    using namespace Actions;

    m_actionlookup->alookup.resize(4);
    m_actionlookup->hlookup.resize(4);

    OptionH normal = opt->getSub("Keys/Normal");
    Option::Sources normals(&NormalMode::ActionNames[0], 
                            &NormalMode::ActionNames[0] + NormalMode::EndNormalMode);
    normal->checkRequiredKeys(normals, "RogueMonkey/Display/Keys/Normal");

    OptionH menu = opt->getSub("Keys/Menu");
    Option::Sources menus(&MenuMode::ActionNames[0],
                          &MenuMode::ActionNames[0] + MenuMode::EndMenuMode);
    menu->checkRequiredKeys(menus, "RogueMonkey/Display/Keys/Menu");

    OptionH target = opt->getSub("Keys/Targetting");
    Option::Sources targets(&TargetMode::ActionNames[0],
                            &TargetMode::ActionNames[0] + TargetMode::EndTargetMode);
    target->checkRequiredKeys(targets, "RogueMonkey/Display/Keys/Targetting");

    ReadAndSplit(NormalMode::EndNormalMode, normal, NormalMode::ActionNames,
                 m_actionlookup->alookup[Actions::Normal], m_actionlookup->hlookup[Normal]);

    ReadAndSplit(MenuMode::EndMenuMode, menu, MenuMode::ActionNames,
                 m_actionlookup->alookup[Menu], m_actionlookup->hlookup[Menu]);

    ReadAndSplit(TargetMode::EndTargetMode, target, TargetMode::ActionNames,
                 m_actionlookup->alookup[Targetting], m_actionlookup->hlookup[Targetting]);

    // Set up FOV (using puyo's excellent libfov)
    fov_settings_init(m_fov_settings.get());
    fov_settings_set_shape(m_fov_settings.get(), FOV_SHAPE_OCTAGON);
    fov_settings_set_corner_peek(m_fov_settings.get(), FOV_CORNER_NOPEEK);
    fov_settings_set_opaque_apply(m_fov_settings.get(), FOV_OPAQUE_APPLY);
    fov_settings_set_opacity_test_function(m_fov_settings.get(), HeroIsNotOpaque);
    fov_settings_set_apply_lighting_function(m_fov_settings.get(), HeroLightCell);
}


Actions::NormalMode::Type
Display::getNormalAction()
{
    return static_cast<Actions::NormalMode::Type>(getAction(Actions::Normal));
}



Actions::TargetMode::Type
Display::getTargetAction()
{
    return static_cast<Actions::TargetMode::Type>(getAction(Actions::Targetting));
}


std::pair<int, int>
Display::getMenuAction()
{
    std::pair<int, int> found(0, 0);
    std::vector<Input::KeyPress> keys;

    while (found.first == 0)
    {
        Input::KeyPress kp = getRawKeyPress();

        if (kp.type == Input::KeyPress::Key && std::isalnum(kp.data.key))
        {
            if (std::isalpha(kp.data.key))
            {
                found.first = - kp.data.key;
                return found;
            }
            found.second *= 10;
            found.second += kp.data.key - '0';
            continue;
        }

        keys.push_back(kp);
        ActionLookup::Lookup::iterator it = m_actionlookup->alookup[Actions::Menu].lower_bound(keys);

        if (it == m_actionlookup->alookup[Actions::Menu].end() ||
            !std::equal(keys.begin(), keys.end(), it->first.begin()))
        {
            // not found at all or not keys don't match so far
            break;
        }
        else if (it->first == keys)
        {
            // exact match
            found.first = it->second;
        }
        // else we keep looping
    }
    return found;
}


std::string
Display::getKeySequence(Actions::Mode mode, int action, Colour::Type clr,
                        std::string const & sep) const
{
    typedef ActionLookup::HumanLookup::const_iterator Const_iterator;
    Display::ActionLookup::HumanLookup const & lookup = m_actionlookup->hlookup[mode];
    std::pair<Const_iterator, Const_iterator> range = lookup.equal_range(action);
    std::ostringstream out;
    for (Const_iterator it = range.first; it != range.second; ++it)
    {
        if (it != range.first)
            out << sep;
        out << Colour::ColourNames[clr] << it->second;
    }
    return out.str();
}



int
Display::getAction(Actions::Mode mode)
{
    std::vector<Input::KeyPress> keys;
    int found = 0;

    while (found == 0)
    {
        Input::KeyPress kp = getRawKeyPress();
        keys.push_back(kp);
        ActionLookup::Lookup::iterator it = m_actionlookup->alookup[mode].lower_bound(keys);

        if (it == m_actionlookup->alookup[mode].end() ||
            !std::equal(keys.begin(), keys.end(), it->first.begin()))
        {
            // not found at all or not keys don't match so far
            break;
        }
        else if (it->first == keys)
        {
            // exact match
            found = it->second;
        }
        // else we keep looping
    }
    return found;
}


ItemH
Display::selectItemUsingShortcut(ItemPileH source, Item::ItemType classes, CreatureH cr,
                                 std::string prompt, std::string const & title)
{
    typedef std::map<char, ItemH> C2Item;
    ItemPileH shortlist = source;
    if (classes != Item::All)
        shortlist = source->getTempPileByClass(classes, ItemPile::Nil);
    C2Item c2item;
    std::string allowed;
    for (ItemPile::iterator it = shortlist->begin(); it != shortlist->end(); ++it)
        c2item.insert(C2Item::value_type((*it)->getEffect(ItemEffect::AlphaIndex).edata1, *it));
    for (C2Item::iterator it = c2item.begin(); it != c2item.end(); ++it)
        allowed.push_back(it->first);
    std::sort(allowed.begin(), allowed.end(), AlphaComparator());
    prompt += COLOUR_WHITE "- " COLOUR_LGREY;
    prompt += allowed;
    prompt += COLOUR_WHITE " ?*";
    allowed += "-?*\n\r ";
    allowed += static_cast<char>(Input::Esc);

    ItemH selected;
    while (!selected)
    {
        Input::KeyPress answer = askSingleKeyQuestion(prompt, allowed);
        assert(answer.type == Input::KeyPress::Key && "Illegal mouse input in selectItemsUsingShortcut");
        if (answer.data.key == '-')
        {
            selected = BogusItem::getInstance();
        }
        else if (answer.data.key == '*' || answer.data.key == '?')
        {
            ItemPicker::Selected picked = selectItemsFromPile(
                (answer.data.key == '*') ? source : shortlist,
                (answer.data.key == '*') ? Item::All : classes,
                Selector::SelectOne, cr, title);
            if (!picked.empty())
                selected = picked.front();
        }
        else if (std::isalpha(answer.data.key))
        {
            selected = c2item[answer.data.key];
        }
        else
        {
            printMessage("Cancelled.");
            render();
            break;
        }
    }
    return selected;
}


void
Display::drawMap()
{
    Coords mypos = HERO->getCoords();
    MapH mymap = mypos.M();
    Coords mapsize = mymap->getSize();

    Coords vis_size = getMainMapSize();
    Coords vis_ctr = Coords((vis_size.X() + 1) / 2, (vis_size.Y() + 1) / 2);
    int sightradius = HERO->getSightRadius();
    if (sightradius > std::min(vis_ctr.X(), vis_ctr.Y()))
        sightradius = std::min(vis_ctr.X(), vis_ctr.Y());

    mymap->clearSeenGrid();
    fov_circle(m_fov_settings.get(), mymap.get(), &HERO, mypos.X(), mypos.Y(), sightradius);
    HeroLightCell(mymap.get(), mypos.X(), mypos.Y(), 0, 0, &HERO);
    drawCellsToMap(mymap, mypos.X() - sightradius, mypos.Y() - sightradius,
                   1 + 2 * sightradius, 1 + 2 * sightradius);
}




Display::~Display()
{
}


Display::Fact &
Display::GetFactory()
{
    static Fact factory;
    return factory;
}

