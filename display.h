#ifndef H_DISPLAY_
#define H_DISPLAY_ 1

// -*- Mode: C++ -*-
// RogueMonkey Copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file COPYING

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "factory.h"
#include "handles.h"
#include "inputdef.h"
#include "item.h"
#include "option.h"
#include "selector.h"


struct fov_settings_type;


/**
 * Display runs the interface. Characters are not actually printed to the
 * screen until the render() method is called.
 */
class Display
{
public:
    static unsigned int const HPUpdate = 1U;
    static unsigned int const ManaUpdate = 2U;
    static unsigned int const StatUpdate = 4U;
    static unsigned int const NameUpdate = 8U;
    static unsigned int const ExpUpdate = 64U;
    static unsigned int const ClassesUpdate = 32U;
    static unsigned int const EquipUpdate = 64U;
    static unsigned int const MapUpdate = 128U;
    static unsigned int const LocationUpdate = 256U;
    static unsigned int const EffectsUpdate = 512U;
    static unsigned int const LastActionUpdate = 1024U;
    static unsigned int const Labels = 2048U;
    static unsigned int const AllUpdate = 32767U;

    struct StatPos
    {
        static int Name_pos;                    
        static int Location_pos;               
        static int Health_pos;                  
        static int Mana_pos;                    
        static int Exp_pos;                     
        static int Class_pos;                  
        static int Stats_pos;                  
        static int Cond_pos;                   
        static int Equip_pos;                  
        static int Action_pos;                  
    };

    struct ActionLookup;
    virtual ~Display();

    /**
     * Draw the map surrounding the Creature (usually the Hero)
     */
    void drawMap();

    /**
     * Print a message in the main message output (statuses etc) area.
     * Colours are embedded in the actual string itself.
     *
     * @param msg   message to print
     */
    virtual void printMessage(std::string const &msg) = 0;

    /**
     * Update the stats panel with requested info
     *
     * @param types        types of data to update (made up of class constants)
     */
    virtual void updateStats(unsigned int types) = 0;

    /**
     * Select items from an ItemPile
     *
     * @param source      items to select from
     * @param classes     classes of items to select from
     * @param sel         style of selection
     * @param cr          creature viewing
     * @param title       Title to display
     * @return            Selected items. source remains intact
     */
    virtual ItemPicker::Selected selectItemsFromPile(ItemPileH source, Item::ItemType classes,
                                                     Selector::SelectType sel, CreatureH cr,
                                                     std::string const & title) = 0;

    /**
     * Select a single item from Inventory using shortcuts in pile
     *
     * @param source      Inventory to use
     * @param classes     classes to select from initially
     * @param cr          creature viewing
     * @param prompt      prompt to display with list of keys
     * @param title       title to display in longer selection
     * @return            Selected item
     */
    ItemH selectItemUsingShortcut(ItemPileH source, Item::ItemType classes, CreatureH cr,
                                  std::string prompt, std::string const & title);

    /**
     * Ask a question of the user, waiting for a single KeyPress
     *
     * @param prompt      prompt to display
     * @param allowed     allowed keypresses
     * @return            keypress answer
     */
    virtual Input::KeyPress askSingleKeyQuestion(std::string const & prompt,
                                                 std::string const & allowed) = 0;

    /**
     * Show a help screen for key commands
     *
     * @param type       Type of help commands to show (normal/menu/targetting)
     */
    virtual void displayKeyHelp(Actions::Mode type) = 0;

    /**
     * Select from a list
     * 
     * @param vec        vector<> of items to pick among
     * @param title      title to display at top
     * @return           index of selected item. -1 for none
     */
    virtual int selectFromList(std::vector<std::string> const & vec, std::string const & title) = 0;

    /**
     * Select from a list, displaying more information as each selected
     * @param sel        items to select from
     * @param desc       description to display
     * @param title      title to display at time
     * @return           index of selected item. -1 for none
     */
    virtual int selectFromList(std::vector<std::string> const & sel, std::vector<std::string> const & desc,
                               std::string const & title) = 0;

    /**
     * Show the messages previously accumulated
     */
    virtual void showPreviousMessages() = 0;

    /**
     * Actually display the previous changes to the user.
     */
    virtual void render() = 0;

    /**
     * Get a raw (i.e. not translated to an action) keypress/mouse position
     *
     * @return     virtual keypress / mouse button press
     */
    virtual Input::KeyPress getRawKeyPress() = 0;

    /**
     * Get the next action from user input
     *
     * @return     next action required from user
     */
    Actions::NormalMode::Type getNormalAction();

    /**
     * Get the next targetting (cursor move, mostly) from user input
     *
     * @return    next action required
     */
    Actions::TargetMode::Type getTargetAction();

    /**
     * Get the next menu (cursor move, select) action from user input
     *
     * @return    pair of next action required & count (num times to apply)
     */
    std::pair<int, int> getMenuAction();

    /**
     * Get a string representing keypress sequences for a Normal action
     *
     * @param  mode       Mode of action
     * @param  action     Type of action to return
     * @param  clr        colour to print sequences in
     * @param  sep        separator string to put between options
     * @return            string of sequences
     */
    std::string getKeySequence(Actions::Mode mode, int action, Colour::Type clr,
                               std::string const & sep) const;

    /**
     * Get the list of options which must be available for this Display type
     *
     * @return            vector of strings for required option names
     */
    virtual Option::Sources getRequiredOpts() const = 0;


    typedef Display * (*FactFunc)(OptionH);
    typedef Factory<Display *, std::string, FactFunc, DisplayH> Fact;
    static Fact & GetFactory();

#ifdef WIZARD
    virtual void drawWizardMap() = 0;
#endif
    
protected:
    Display(OptionH opt);

private:
    virtual Coords getMainMapSize() const = 0;


    /**
     * Draw the calculated cells to the main map
     *
     * @param map        Map to draw
     * @param tlx        top left column of map area to draw
     * @param tly        top left row of map area to draw
     * @param cols       # columns which can be seen (x axis)
     * @param rows       # rows which can be seen (y axis)
     */
    virtual void drawCellsToMap(MapH map, int tlx, int tly, int cols, int rows) = 0;


    /**
     * Get the next action from user input
     *
     * @param mode       type of action required
     * @return           (int) of action enum (must be cast to appropriate type)
     */
    int getAction(Actions::Mode mode);

    typedef boost::shared_ptr<fov_settings_type> FOVSettings;

    FOVSettings                     m_fov_settings;
    boost::shared_ptr<ActionLookup> m_actionlookup;
};



#endif

