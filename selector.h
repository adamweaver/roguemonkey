#ifndef H_SELECTOR_
#define H_SELECTOR_ 1

// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <string>
#include <vector>

#include "handles.h"
#include "item.h"


/** 
 * Generic base class for all selection types
 */
class Selector
{
public:
    /**
     * Style of selecting items from a list
     */
    enum SelectType
    {
        DisplayOnly, SelectOne, SelectMultiple
    };

    struct LineData
    {
        char              alpha_shortcut;
        int               num_to_select;
        int               num_selected;
        std::string       line;

        LineData() :
            alpha_shortcut(0),
            num_to_select(-1),
            num_selected(-1),
            line("")
        {
        }

        LineData(char alpha, int num_to_sel, std::string const & l) :
            alpha_shortcut(alpha),
            num_to_select(num_to_sel),
            num_selected(0),
            line(l)
        {
        }
    };
    typedef std::vector<LineData> Lines;

protected:
    int                   m_num_lines;
    int                   m_topline;
    int                   m_botline;
    int                   m_selected_line;
    int                   m_hilite_line;
    int                   m_screen_height;
    Lines                 m_lines;
    mutable bool          m_redraw_required;
    bool                  m_allow_help;
    SelectType            m_seltype;

    Selector(int num_lines, int start_pos, int height, bool allow_help = true);

public:
    virtual ~Selector() = 0;

    virtual void selectLine(int line, int num = 0) = 0;
    virtual void deselectLine(int line, int num = 0) = 0;
    virtual void toggleLine(int line, int num = 0) = 0;
    virtual void redraw() = 0;
    virtual char getSeparator(int num) = 0;

    void processEvents(Display *disp);
    void scroll(int delta);
    void setScreenHeight(int height);
    int getTopLine() const;
    int getBotLine() const;
    int getCurLine() const;
    int getHiliteLine() const;
    bool redrawRequired() const;
    int getItemNo(char shortcut);


};


/**
 * Displays info on screen, with no option to select'
 */
class InfoDisplayer : public Selector
{
public:
    InfoDisplayer(int num_lines, int start_pos, int height, bool allow_help = true);

    virtual void selectLine(int line, int num = 0);
    virtual void deselectLine(int line, int num = 0);
    virtual void toggleLine(int line, int num = 0);
    virtual void redraw() = 0;
    virtual char getSeparator(int line);
};


class LinePicker : public Selector
{
    int         m_selected;
public:
    LinePicker(std::vector<std::string> const & lines, bool use_alphas, int start_pos, int height);
    
    virtual void selectLine(int line, int num);
    virtual void deselectLine(int line, int num);
    virtual void toggleLine(int line, int num);
    virtual void redraw() = 0;
    virtual char getSeparator(int num);

    int getResult(Display *disp);
};



class ItemPicker : public Selector
{
public:
    typedef std::vector<ItemH> Selected;

    /**
     * Create a selector for items with titles
     *
     * @param ipile         ItemPile of items to select among
     * @param classes       classes of items to choose among
     * @param type          Are we displaying, or allowing selections?
     * @param screenheight  Size of viewport
     * @param cr            Creature viewing items
     */
    ItemPicker(ItemPileH ipile, Item::ItemType classes, Selector::SelectType type,
               int screenheight, CreatureH cr);

    /**
     * Create a selector for items without titles
     *
     * @param items         vector of Items to select among
     * @param type          Are we displaying, or allowing selections?
     * @param screenheight  Size of viewport
     * @param cr            Creature viewing items
     */
    ItemPicker(Selected items, Selector::SelectType type, int screenheight, CreatureH cr);

    virtual void selectLine(int line, int num = 0);
    virtual void deselectLine(int line, int num = 0);
    virtual void toggleLine(int line, int num = 0);
    virtual void redraw() = 0;

    virtual Selected getResult(Display *disp);
    virtual char getSeparator(int num);

protected:
    Selected    m_items;

};

#endif

