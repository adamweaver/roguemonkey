#ifndef H_INPUTDEF_
#define H_INPUTDEF_

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <list>
#include <string>
#include <utility>


#include "handles.h"


namespace Colour
{
    /**
     * Enumeration of colours available for display
     */
    enum Type
    {
        Black, DGrey, LGrey, Silver, White, Red, Pink, Orange, Brown,
        Gold, Yellow, DGreen, LGreen, DBlue, LBlue, Aqua, Purple, Magenta,
        EndColour
    };

    extern char const *ColourNames[EndColour];
    extern char const Delimiter;
    extern char const *INIColourNames[EndColour];

    typedef std::pair<Type, std::string> ColourString;
    typedef std::list<ColourString> ColourStringVec;
    ColourStringVec SplitStringByColour(std::string const & str, Type initial);
}
#define COLOUR_BLACK   "~Black~"
#define COLOUR_DGREY   "~DGrey~"
#define COLOUR_LGREY   "~LGrey~"
#define COLOUR_SILVER  "~Silver~"
#define COLOUR_WHITE   "~White~"
#define COLOUR_RED     "~Red~"
#define COLOUR_PINK    "~Pink~"
#define COLOUR_ORANGE  "~Orange~"
#define COLOUR_BROWN   "~Brown~"
#define COLOUR_GOLD    "~Gold~"
#define COLOUR_YELLOW  "~Yellow~"
#define COLOUR_DGREEN  "~DGreen~"
#define COLOUR_LGREEN  "~LGreen~"
#define COLOUR_DBLUE   "~DBlue~"
#define COLOUR_LBLUE   "~LBlue~"
#define COLOUR_AQUA    "~Aqua~"
#define COLOUR_PURPLE  "~Purple~"
#define COLOUR_MAGENTA "~Magenta~"


typedef std::pair<char, Colour::Type> Representation;



namespace Actions
{
    enum Mode
    {
        Normal, Menu, Targetting
    };

    namespace NormalMode
    {
        enum Type
        {
            Invalid,
            Help,

            MoveSouthWest, MoveSouth, MoveSouthEast,
            MoveWest, MoveEast,
            MoveNorthWest, MoveNorth, MoveNorthEast,

            WaitHere, LookAround,

            PickUpFromGround, SingleDropToGround, MultiDropToGround,
            DisplayInventory, DisplayEquipped,

            Wield, Wear, UnWear, PrevMessages,

#ifdef WIZARD
            WizardCommand,
#endif
            Cancel, QuitGame,
            EndNormalMode
        };
        extern char const *ActionNames[EndNormalMode];
        extern char const *HelpNames[EndNormalMode];
        Type ActionTypeByName(std::string const & name);
        bool IsMove(Type t);
        int GetXMoveOffset(Type t);
        int GetYMoveOffset(Type t);

    }

    namespace MenuMode
    {
        enum Type
        {
            Invalid,

            Help,
            Up, Down, PageUp, PageDown, Home, End,
            Select, SelectAll, Deselect, DeselectAll,
            Confirm, Quit,

            EndMenuMode,
            Shortcut
        };
        extern char const *ActionNames[EndMenuMode];
        extern char const *HelpNames[EndMenuMode];
        Type ActionTypeByName(std::string const &name);
    }

    namespace TargetMode
    {
        enum Type
        {
            Invalid,

            MoveSouthWest, MoveSouth, MoveSouthEast,
            MoveWest, MoveEast,
            MoveNorthWest, MoveNorth, MoveNorthEast,

            Select, NextTarget, PrevTarget, Home,

            Quit,

            EndTargetMode
        };

        extern char const *ActionNames[EndTargetMode];
        extern char const *HelpNames[EndTargetMode];
        Type ActionTypeByName(std::string const &name);
    }

    namespace WizardMode
    {
	enum Type
	{
	    Invalid,
	    DisplayMap,
	    Quit,
	    EndWizardMode
	};

	extern char const *ActionNames[EndWizardMode];
	extern char const *HelpNames[EndWizardMode];
	Type ActionTypeByName(std::string const &name);
    }

}



namespace Input
{
    /**
     * Enumeration of special non-ascii keypresses
     */
    enum SpecialKey
    {
        // remember to alter Abbrev2Key in display.cc
        Invalid = 0,
        Tab = 9, Enter = 13, Esc = 27, Delete = 127,
        PageDown = 128, PageUp, Home, End,
        KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8, KP9, KP0,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        Up, Down, Left, Right,

        Control = 256, Alt = 512, Meta = 1024,
        AllModifierKeys = Control | Alt | Meta
    };

    struct KeyPress
    {
        enum Type {Key, Mouse} type;
        union Data {int key; struct Coord{int x; int y; int b;} coord;} data;
        int meta;

        explicit KeyPress(char c, int m = 0) :
            type(Key),
            meta(m)
        {
            data.key = c;
        }

        void reset()
        {
            type = Key;
            meta = 0;
            data.key = 0;
        }

        bool operator<(KeyPress const & rhs) const
        {
            if (type != rhs.type)
                return type < rhs.type;
            if (type == Key)
                return data.key < rhs.data.key || meta < rhs.meta;
            return data.coord.x < rhs.data.coord.x ||
                data.coord.y < rhs.data.coord.y ||
                data.coord.b < rhs.data.coord.b || meta < rhs.meta;

        }

        bool operator==(KeyPress const & rhs) const
        {
            if (type != rhs.type)
                return false;
            if (type == Key)
                return data.key == rhs.data.key && meta == rhs.meta;
            return data.coord.x == rhs.data.coord.x &&
                data.coord.y == rhs.data.coord.y &&
                data.coord.b == rhs.data.coord.b &&
                meta == rhs.meta;
        }

        bool operator!() const
        {
            return meta == 0 && ((type == Key) ? data.key == 0 :
                                 data.coord.x == 0 && data.coord.y == 0 &&
                                 data.coord.b == 0);
        }

        bool operator!=(KeyPress const & rhs) const
        {
            return !operator==(rhs);
        }
    };
}

#endif
