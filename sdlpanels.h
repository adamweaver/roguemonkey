#ifndef H_SDLPANELS_
#define H_SDLPANELS_ 1

// -*- Mode: C++ -*-
// RogueMonkey Copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file COPYING

#include <vector>

#include "boost/shared_ptr.hpp"
#include "SDL.h"
#include "SDL_ttf.h"



typedef boost::shared_ptr<TTF_Font> FontH;
typedef boost::shared_ptr<SDL_Cursor> CursorH;
typedef boost::shared_ptr<SDL_Surface> SurfaceH;


//============================================================================
/**
 * Base canvas - derive other drawables from this
 */
class SDLPanel
{
public:
    virtual ~SDLPanel();

    /**
     * Blit the container and panel to another surface
     *
     * @param  surf        Surface to render to
     */
    void blitToSurface(SurfaceH surf);

    /**
     * Erases the entire canvas and reinstates background
     */
    virtual void clearAll();

protected:
    SDLPanel(OptionH opt);
    SDLPanel(OptionH opt, int width, int height);

    /**
     * Blit a string to a specified position
     * @param  str     string to print
     * @param  row     row to print on 
     * @param  xpos    x-position to start at (in pixels)
     * @param  width   maximum width to print, stopping short otherwise
     * @param  surface surface to print on
     * @return         width of printed line
     */
    int blitString(std::string const & str, int row, int xpos, int width, SurfaceH surf);

    /**
     * Blit a string to a specified position, wrapping onto next line if necessary
     * @param  str     string to print
     * @param  row     row to print on
     * @param  xpos    x-position to start at (in pixels)
     * @param  width   maximum width to print, wrapping onto next line
     * @param  surface surface to print upon
     * @return         number of rows printed over
     */
    int blitMultiString(std::string const & str, int row, int xpos, int width, SurfaceH surf = SurfaceH());

    /**
     * Erase a rectangle (and reinstate background if required)
     *
     * @param tlx       top left x-coordinate of rect
     * @param tly       top left y-coordinate of rect
     * @param brx       bottom right x-coordinate of rect
     * @param bry       bottom right y-coordinate of rect
     */
    void clearRect(int tlx, int tly, int brx, int bry);

    /**
     * Hilights a line
     *
     * @param   x          top left coordinate of rectangle
     * @param   y          top left coordinate of rectangle
     * @param  col         colour to hilight in
     * @param  alpha       alpha blend to use
     */
    void hilightLine(int tlx, int y, Colour::Type col, int alpha);

    std::string m_font_name;
    int         m_font_size;
    long        m_font_index;
    FontH       m_font;
    int         m_line_height;

    SDL_Rect    m_rect;
    SurfaceH    m_surface;
    int         m_centre_opacity;

    SDL_Rect    m_outer_rect;
    SurfaceH    m_outer;
};


//============================================================================
/**
 * Map canvas
 */
class SDLMapPanel : public SDLPanel
{
public:
    SDLMapPanel(OptionH opt);
    virtual ~SDLMapPanel();

    /**
     * Plot a character (from pre-prepared character map canvas) to
     * position on map area (0,0) is top left.
     *
     * @param x          x-coordinate in cells
     * @param y          y-coordinate in cells
     * @param c          character to plot
     * @param col        colour to print in
     */
    void plotCharacter(int x, int y, char c, Colour::Type col);

    /**
     * Gets the number of cells available in the canvas
     *
     * @return       Coords containing number of cells
     */
    Coords numCells() const;

private:
    /**
     * Initialise character data surface from font. Load canvas from disk
     * if available.
     *
     * @param fontname          name of font to load
     * @param font              Font structure to be used
     * @param fontsize          point size of font to use
     * @param fontindex         index of font in fontfile
     * @return                  prepared canvas
     */
    SurfaceH InitMapCanvas(std::string const & fontanme, FontH font,
                           int fontsize, long fontindex);


    typedef std::vector<SurfaceH> TerrainSurfaces;

    int             m_cellwidth;
    int             m_cellheight;
    SurfaceH        m_data_surf;
    TerrainSurfaces m_terrain_surfaces;
};

class SDLDisplay;
//============================================================================
/**
 * Output message canvas
 */
class SDLMessagePanel : public SDLPanel
{
    friend class SDLDisplay;
    typedef std::vector<std::string> PastMessages;

    SDLDisplay * m_display;
    PastMessages m_past_messages;
    int m_y;
    int m_num_lines;
    int m_repeat;
    int m_num_so_far;
    unsigned int m_current_turn;
    bool m_logging;

public:
    /**
     * Create a new message panel using Options file
     *
     * @param  opt      options file to parse
     * @param disp      owner creating panel
     */
    SDLMessagePanel(OptionH opt, SDLDisplay *);
    virtual ~SDLMessagePanel();

    /**
     * Clear entire message canvas
     */
    virtual void clearAll();

    /**
     * How many messages have been printed so far?
     *
     * @return     number of printed messages (1 per line)
     */
    int numPastMessages() const;

    /**
     * Get a copy of past message
     *
     * @param  num  message # to retrieve
     * @return      previously printed message (sans formatting)
     */
    std::string const & retrievePastMessage(int num) const;

    /**
     * Print a message (including formatting directives) to the message area
     *
     * @param  msg   message to print
     */
    void printMessage(std::string const &msg);

    /**
     * Moves printing line down one
     */
    void nl();

    /**
     * Get the messages previously accumulated.
     */
    std::vector<std::string> const & getPreviousMessages() const;

private:
    /**
     * Prints a (sub)string in colour. String will not overwrite borders
     *
     * @param  str        string to print
     * @param  col        Colour to print
     * @return            number of characters printed
     */
    int printString(std::string const & str, Colour::Type col);

};



//============================================================================
/**
 * Statistic canvas
 */
class SDLStatPanel : public SDLPanel
{
public:
    SDLStatPanel(OptionH opt);
    virtual ~SDLStatPanel();

    /**
     * Update the stats panel with requested info
     *
     * @param types        types of data to update (made up of class constants)
     */
    virtual void updateStats(unsigned int types);

private:
    void updateHPMana(int pos, int cur, int mx);

    OptionH m_option;
};


//============================================================================
/**
 * Popup selector canvas
 */
class SDLItemPicker : public SDLPanel
{
public:

};



#endif

