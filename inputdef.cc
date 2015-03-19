// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL refer to included file LICENCE.txt

#include <algorithm>
#include <cassert>

#include "inputdef.h"
#include "stllike.h"


//============================================================================
// Colour
//============================================================================
char const *Colour::ColourNames[Colour::EndColour] =
{
    COLOUR_BLACK, COLOUR_DGREY, COLOUR_LGREY, COLOUR_SILVER, COLOUR_WHITE,
    COLOUR_RED, COLOUR_PINK, COLOUR_ORANGE, COLOUR_BROWN, COLOUR_GOLD,
    COLOUR_YELLOW, COLOUR_DGREEN, COLOUR_LGREEN, COLOUR_DBLUE, COLOUR_LBLUE,
    COLOUR_AQUA, COLOUR_PURPLE, COLOUR_MAGENTA
};

char const * Colour::INIColourNames[Colour::EndColour] = 
{
    "Black", "Dark Grey", "Light Grey", "Silver", "White",
    "Red", "Pink", "Orange", "Brown", "Gold",
    "Yellow", "Dark Green", "Light Green", "Dark Blue", "Light Blue"
    "Aqua", "Purple", "Magenta"
};

char const Colour::Delimiter = '~';


Colour::ColourStringVec
Colour::SplitStringByColour(std::string const & str, Colour::Type current)
{
    ColourStringVec vec;
    std::string::const_iterator begin = str.begin();
    while (begin != str.end())
    {
        std::string::const_iterator end = std::find(begin, str.end(), Colour::Delimiter);
        if (begin == end)
        {
            // first character is a colour identifier
            end = std::find(begin + 1, str.end(), Colour::Delimiter);
            if (end != str.end())
            {
                std::string colourname(begin, ++end);
                current = StringToEnum(colourname, EndColour, ColourNames, current);
            }
        }
        else
        {
            std::string word(begin, end);
            if (word != "" && word[0] != '\0')
                vec.push_back(ColourString(current, word));
        }
        begin = end;
    }
    return vec;
}



//============================================================================
// Actions
//============================================================================
namespace
{
    int XMoveOffset[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    int YMoveOffset[8] = { 1, 1, 1, 0, 0, -1, -1, -1 };

}

char const * Actions::NormalMode::ActionNames[Actions::NormalMode::EndNormalMode] =
{
    "",
    "Help",
    "SouthWest", "South", "SouthEast", "West",
    "East", "NorthWest", "North", "NorthEast",
    "Wait", "Look", "PickUp", "Drop", "MultiDrop", "Inventory",
    "Equipped", "Wield", "Wear", "UnWear", "PrevMessages",
#ifdef WIZARD
    "Wizard",
#endif
    "Cancel", "QuitGame"
};

char const * Actions::NormalMode::HelpNames[Actions::NormalMode::EndNormalMode] =
{
    "",
    "Show Help",
    "Move South West", "Move South", "Move South East", "Move West",
    "Move East", "Move North West", "Move North", "Move North East",
    "Wait", "Look Around", "Pick up Item", "Drop Item", "Multi-Drop Item",
    "Show Inventory", "Show Equipment", "Wield Primary Hand", "Wear / Wield Off Hand",
    "Remove Equipment", "Show Previous Messages",
#ifdef WIZARD
    "Use Wizard Command",
#endif
    "Cancel mode", "Quit Game"
};



char const * Actions::MenuMode::ActionNames[Actions::MenuMode::EndMenuMode] =
{
    "",

    "Help", "Up", "Down", "PageUp", "PageDown", "Home", "End",
    "Select", "SelectAll", "Deselect", "DeselectAll",
    "Confirm", "Quit"
};

char const * Actions::MenuMode::HelpNames[Actions::MenuMode::EndMenuMode] =
{
    "",
    "Show Help", "Cursor Up", "Cursor Down", "Cursor Page Up", "Cursor Page Down",
    "Cursor to Start", "Cursor to End", "Select", "Select All", "Deselect", "Deselect All",
    "Confirm Selection", "Quit"
};


char const * Actions::TargetMode::ActionNames[Actions::TargetMode::EndTargetMode] =
{
    "",

    "SouthWest", "South", "SouthEast", "West",
    "East", "NorthWest", "North", "NorthEast",
    "Select", "NextTarget", "PrevTarget", "Home",
    "Quit"
};


char const * Actions::TargetMode::HelpNames[Actions::TargetMode::EndTargetMode] =
{
    "",
    "Move South West", "Move South", "Move South East", "Move West", "Move East",
    "Move North Weest", "Move North", "Move North East", "Select Target",
    "Next Target", "Previous Target", "Move to Home", "Quit"
};



Actions::NormalMode::Type
Actions::NormalMode::ActionTypeByName(std::string const & name)
{
    for (int i = 0; i < EndNormalMode; ++i)
    {
        if (!name.compare(ActionNames[i]))
            return static_cast<Type>(i);
    }
    return Invalid;
}


bool
Actions::NormalMode::IsMove(Actions::NormalMode::Type t)
{
    return t >= MoveSouthWest && t <= MoveNorthEast;
}


int
Actions::NormalMode::GetXMoveOffset(Actions::NormalMode::Type t)
{
    assert(IsMove(t));
    return XMoveOffset[t - MoveSouthWest];
}


int
Actions::NormalMode::GetYMoveOffset(Actions::NormalMode::Type t)
{
    assert(IsMove(t));
    return YMoveOffset[t - MoveSouthWest];
}


Actions::MenuMode::Type
Actions::MenuMode::ActionTypeByName(std::string const & name)
{
    for (int i = 0; i < EndMenuMode; ++i)
    {
        if (!name.compare(ActionNames[i]))
            return static_cast<Type>(i);
    }
    return Invalid;
}


Actions::TargetMode::Type
Actions::TargetMode::ActionTypeByName(std::string const & name)
{
    for (int i = 0; i < EndTargetMode; ++i)
    {
        if (!name.compare(ActionNames[i]))
            return static_cast<Type>(i);
    }
    return Invalid;
}

