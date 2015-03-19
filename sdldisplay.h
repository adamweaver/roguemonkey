#ifndef H_SDLDISPLAY_
#define H_SDLDISPLAY_ 1

// -*- Mode: C++ -*-
// RogueMonkey Copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file COPYING

#include <list>

#include "SDL.h"      // for SDL_Rect
#include "SDL_ttf.h"

#include "display.h"
#include "handles.h"


typedef boost::shared_ptr<TTF_Font> FontH;
typedef boost::shared_ptr<SDL_Cursor> CursorH;
typedef boost::shared_ptr<SDL_Surface> SurfaceH;

/**
 * Margin width (so as not to print hard up against border)
 */
int const LeftMargin = 5;

/**
 * Margin width (so as not to print hard up against border)
 */
int const RightMargin = 5;

/**
 * Width between columns
 */
int const Separator = 10;

/**
 * Mapping of RGB values for colours
 */
extern SDL_Color RGB[Colour::EndColour];

extern std::string DefaultFontName;
extern int DefaultFontSize;
extern long DefaultFontIndex;

//============================================================================
//============================================================================
/**
 * Display manager
 */
class SDLDisplay : public Display
{
public:
    virtual ~SDLDisplay();

    /**
     * Create the SDLDisplay using an option file for input
     *
     * @param  opt    Option file to use for parameters
     * @return        created Display
     */
    static Display *Create(OptionH opt);

    /**
     * Print a message in the main message output (statuses etc) area.
     * Colours are embedded in the actual string itself.
     *
     * @param msg   message to print
     */
    virtual void printMessage(std::string const &msg);

    /**
     * Update the stats panel with requested info
     *
     * @param types        types of data to update (made up of class constants)
     */
    virtual void updateStats(unsigned int types);

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
                                                     std::string const & title);

    /**
     * Ask a question of the user, waiting for a single KeyPress
     *
     * @param prompt      prompt to display
     * @param allowed     allowed keypresses
     * @return            keypress answer
     */
    virtual Input::KeyPress askSingleKeyQuestion(std::string const & prompt,
                                                 std::string const & allowed);


    /**
     * Show a help screen for key commands
     *
     * @param type       Type of help commands to show (normal/menu/targetting)
     */
    virtual void displayKeyHelp(Actions::Mode type);

    /**
     * Select from a list
     * 
     * @param vec        vector<> of items to pick among
     * @param title      title to display at top
     * @return           index of selected item. -1 for none
     */
    virtual int selectFromList(std::vector<std::string> const & vec, std::string const & title);

    /**
     * Select from a list, displaying more information as each selected
     * @param sel        items to select from
     * @param desc       description to display
     * @param title      title to display at time
     * @return           index of selected item. -1 for none
     */
    virtual int selectFromList(std::vector<std::string> const & sel, std::vector<std::string> const & desc,
                               std::string const & title);

    /**
     * Show the messages previously accumulated
     */
    virtual void showPreviousMessages();

    /**
     * Actually display the previous changes to the user.
     */
    virtual void render();

    /**
     * Get a raw (i.e. not translated to an action) keypress/mouse position
     *
     * @return     virtual keypress / mouse button press
     */
    virtual Input::KeyPress getRawKeyPress();

    /**
     * Get the list of options which must be available for this Display type
     *
     * @return      vector of strings for required option names
     */
    virtual Option::Sources getRequiredOpts() const;

private:
    SDLDisplay(OptionH opt);
    virtual Coords getMainMapSize() const;

#ifdef WIZARD
    virtual void drawWizardMap();
#endif

    /**
     * Draw the calculated cells to the main map
     *
     * @param map        Map to draw
     * @param tlx        top left column of map area to draw
     * @param tly        top left row of map area to draw
     * @param cols       # columns which can be seen (x axis)
     * @param rows       # rows which can be seen (y axis)
     */
    virtual void drawCellsToMap(MapH map, int tlx, int tly, int cols, int rows);

    /**
     * Initialise SDL & TTF_SDL, ready for use
     *
     * @param  w     width of main canvas
     * @param  h     height of main canvas
     * @param  bpp   bits-per-pixel (usually 8, 16, or 24) - colour depth
     * @return       initialised main surface, ready for use
     */
    static SurfaceH InitSDL(int w, int h, int bpp);

    typedef std::list<SDLPanelH> PopupPanels;

    OptionH           m_options;

    SurfaceH          m_mainsurf;
    SurfaceH          m_background;

    SDLMapPanelH      m_main_map;
    SDLStatPanelH     m_main_stats;
    SDLMessagePanelH  m_main_messages;

    SDL_Event         m_events;
    PopupPanels       m_popups;
    int               m_background_opacity;
    int               m_inactive_opacity;
    int               m_meta;
    int               m_max_hp;
    int               m_max_mana;
};


/**
 * Deleters for smart pointer
 */
void DeleteFont(TTF_Font *);
void DeleteSurface(SDL_Surface *);
void DeleteMainSurface(SDL_Surface *);

/**
 * Creates an SDL_Surface of a specific width and height. BPP extrapolated from main surface.
 * @param  width       width of surface
 * @param  height      height of surface
 * @return             newly created surface, with DeleteSurface() as deleter
 */
SurfaceH CreateSurface(int width, int height);

/**
 * Loads a surface from file
 * @param  filename    name of file to read from
 * @return             newly created surface, with DeleteSurface() as deleter
 */
SurfaceH LoadSurface(std::string const & filename);

/**
 * Load a font by namespace, fall back to default font otherwise
 *
 * @param fontname       name of font (including path) to open
 * @param ptsize         point size of font
 * @param face           font face to open
 * @return               handle to font
 */
FontH LoadFont(std::string const & fontname, int ptsize, long face);

/**
 * Convenience function to create a SDL_Rect
 * @param  x           start position of x
 * @param  y           start position of y
 * @param  w           width of rectangle
 * @param  h           height of rectangle
 * @return             newly created rectangle
 */
SDL_Rect CreateSDLRect(int x, int y, int w, int h);

/**
 * Write (coloured) text in a single line
 * @param  text        text to print, interspersed with colour/formatting directives
 * @param  font        font to print in
 * @param  width       width of text to print. additional is discarded
 * @return             newly created surface, with DeleteSurface() as deleter
 */
SurfaceH CreateSingleText(std::string const & text, FontH font, int width);

/**
 * Write (coloured) text, wrapping at word boundaries, starting at specified position.
 * @param  text        text to print, interspersed with colour/formatting directives
 * @param  font        font to print in
 * @param  width       maximum width of text to print, wrapping at boundaries
 * @param  num_lines   number of lines printed over
 * @return             newly created surface, with DeleteSurface() as deleter
 */
SurfaceH CreateMultiText(std::string const & text, FontH font, int width, int & num_lines);

/**
 * Create a new text surface
 * @param  text        text to print
 * @param  font        font to print in
 * @param  colour      colour to print in
 * @return             newly created surface, with DeleteSurface() as deleter
 */
SurfaceH CreateTextSurface(std::string const & text, FontH font, Colour::Type colour);

#endif

