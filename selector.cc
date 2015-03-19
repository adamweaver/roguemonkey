// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include "display.h"
#include "selector.h"

//============================================================================
// SelectorCreator
//============================================================================
Selector::Selector(int num_lines, int start_pos, int height, bool allow_help) :
    m_num_lines(num_lines),
    m_topline(0),
    m_botline(height),
    m_selected_line(start_pos),
    m_hilite_line(start_pos),
    m_screen_height(height),
    m_lines(),
    m_redraw_required(true),
    m_allow_help(allow_help),
    m_seltype(SelectOne)
{
    scroll(0);
}


Selector::~Selector()
{
}


int
Selector::getTopLine() const
{
    return m_topline;
}


int
Selector::getBotLine() const
{
    return m_botline;
}


int
Selector::getCurLine() const
{
    return m_selected_line;
}


int
Selector::getHiliteLine() const
{
    return m_hilite_line;
}


void
Selector::setScreenHeight(int h)
{
    m_screen_height = h;
    m_botline = h;
    scroll(0);
}


bool
Selector::redrawRequired() const
{
    bool red = m_redraw_required;
    m_redraw_required = false;
    return red;
}

namespace
{
    struct FindByAlpha : public std::unary_function<Selector::LineData, bool>
    {
        char alpha_to_find;

        FindByAlpha(char alpha) :
            alpha_to_find(alpha)
        {
        }

        bool operator()(Selector::LineData rhs) const
        {
            return rhs.alpha_shortcut == alpha_to_find;
        }
    };
}


int
Selector::getItemNo(char shortcut)
{
    Lines::iterator it = std::find_if(m_lines.begin(), m_lines.end(), FindByAlpha(shortcut));
    return it != m_lines.end() ? static_cast<int>(it - m_lines.begin()) : -1;
}


void
Selector::scroll(int delta)
{
    int const halfway = m_screen_height / 2;

    // First we move the selected index
    m_selected_line += delta;
    if (m_selected_line < 0)
        m_selected_line = 0;
    else if (m_selected_line >= m_num_lines)
        m_selected_line = m_num_lines - 1;

    // then fit the screen to the index
    // are we in the top half? (cursor moves)
    if (m_selected_line < halfway)
    {
        m_hilite_line = m_selected_line;
        m_topline = 0;
        m_botline = m_num_lines < m_screen_height ? m_num_lines : m_screen_height;
    }
    // are we in the bottom index? (cursor moves)
    else if (m_selected_line > m_num_lines - halfway)
    {
        m_botline = m_num_lines;
        m_topline = m_botline - m_screen_height < 0 ? 0 : m_botline - m_screen_height;
        m_hilite_line = m_selected_line - m_topline;
    }
    // otherwise we move screen, keeping cursor in middle, if we're using rotating view
    else if (m_seltype != Selector::DisplayOnly)
    {
        m_hilite_line = halfway;
        m_topline = m_selected_line - halfway;
        m_botline = m_topline + m_screen_height;
    }
    if (m_botline > m_num_lines)
    {
        m_botline = m_num_lines;
    }
    m_redraw_required = true;
}


void
Selector::processEvents(Display *disp)
{
    bool done = false;
    //int repeat = 1;

    // main loop through keys
    while (done == false)
    {
        if (redrawRequired())
        {
            redraw();
            disp->render();
        }

        std::pair<int, int> action = disp->getMenuAction();

        switch (action.first)
        {
        case Actions::MenuMode::Help:
            if (m_allow_help)
                disp->displayKeyHelp(Actions::Menu);
            break;

        case Actions::MenuMode::Up:
            if (m_seltype == Selector::DisplayOnly)
            {
                scroll(-m_screen_height);
                if (m_selected_line < m_screen_height && m_selected_line > 0)
                    scroll(-m_selected_line);
            }
            else
                scroll(-1);
            break;

        case Actions::MenuMode::Down:
            if (m_seltype == Selector::DisplayOnly)
            {
                scroll(m_screen_height);
                if (m_selected_line > (m_num_lines - m_screen_height))
                    scroll(m_screen_height);
            }
            else
                scroll(+1);
            break;

        case Actions::MenuMode::PageUp:
            scroll(-m_screen_height);
            if (m_seltype == Selector::DisplayOnly && m_selected_line < m_screen_height)
                scroll(-m_selected_line);
            break;

        case Actions::MenuMode::PageDown:
            scroll(m_screen_height);
            if (m_seltype == Selector::DisplayOnly && m_selected_line > (m_num_lines - m_screen_height))
                scroll(m_screen_height);
            break;

        case Actions::MenuMode::Home:
            scroll(-getCurLine());
            break;

        case Actions::MenuMode::End:
            scroll(m_num_lines);
            break;

        case Actions::MenuMode::Select:
            if (m_seltype != Selector::DisplayOnly)
            {
                selectLine(getCurLine(), action.second);
                if (m_seltype == Selector::SelectOne)
                    done = true;
            }
            break;

        case Actions::MenuMode::SelectAll:
            if (m_seltype == Selector::SelectMultiple)
            {
                for (int i = 0; i < m_num_lines; ++i)
                    selectLine(i, action.second);
            }
            break;

        case Actions::MenuMode::Deselect:
            if (m_seltype != Selector::DisplayOnly)
            {
                deselectLine(getCurLine(), action.second);
            }
            break;

        case Actions::MenuMode::DeselectAll:
            if (m_seltype != Selector::DisplayOnly)
            {
                for (int i = 0; i < m_num_lines; ++i)
                    deselectLine(i, action.second);
            }
            break;

        case Actions::MenuMode::Confirm:
	    selectLine(getCurLine(), action.second);
            done = true;
            break;

        case Actions::MenuMode::Quit:
            if (m_seltype != Selector::DisplayOnly)
            {
                for (int i = 0; i < m_num_lines; ++i)
                    deselectLine(i);
            }
            done = true;
            break;

        case Actions::MenuMode::Shortcut:
        default:
            if (m_seltype != Selector::DisplayOnly && action.first < 0)
            {
                int shortcutno = getItemNo(-action.first);
                if (shortcutno != -1)
                {
                    scroll(shortcutno - getCurLine());
                    toggleLine(shortcutno, action.second);
                    if (m_seltype == Selector::SelectOne)
                        done = true;
                }
            }
            break;
        }
    }
}


//============================================================================
// ItemPicker
//============================================================================
namespace
{
    typedef std::pair<ItemH, char> SortPair;
    struct ItemSorterF : public std::binary_function<SortPair, SortPair, bool>
    {
        bool operator() (SortPair l, SortPair r) const
        {
            Item::ItemType lt = l.first->getItemType();
            Item::ItemType rt = r.first->getItemType();
            return (lt < rt) ? true : (lt > rt) ? false : AlphaComparator()(l.second, r.second);
        }
    };
    typedef std::set<SortPair, ItemSorterF> ItemSorter;
}



ItemPicker::ItemPicker(ItemPileH ipile, Item::ItemType classes, Selector::SelectType type,
                       int screenheight, CreatureH cr) :
    Selector(ipile->numStacks(), 0, screenheight),
    m_items()
{
    m_seltype = type;
    char const * alpha_ptr = &AtoZ[0];
    ItemSorter sorter;

    for (ItemPile::iterator iter = ipile->begin(); iter != ipile->end(); ++iter)
    {
        if (classes & (*iter)->getItemType())
        {
            char alphacode = (*iter)->hasEffect(ItemEffect::AlphaIndex) ?
                (*iter)->getEffect(ItemEffect::AlphaIndex).edata1 : *alpha_ptr++;
            if (alphacode == 'Z' + 1)
            {
                alphacode = 'a';
                alpha_ptr = &AtoZ[0] + 1;
            }
            sorter.insert(SortPair(*iter, alphacode));
        }
    }

    Item::ItemType old_type = Item::EndItem;
    for (ItemSorter::iterator iter = sorter.begin(); iter != sorter.end(); ++iter)
    {
        int num = iter->first->getNumber();
        std::string summary = iter->first->describe(cr, 0, true);
        Item::ItemType cur_type = iter->first->getItemType();

        if (cur_type != old_type)
        {
            if (old_type != Item::EndItem)
            {
                m_lines.push_back(LineData('\0', -1, ""));
                m_items.push_back(ItemH());
            }

            old_type = cur_type;
            std::string section_title(COLOUR_LGREY "[" COLOUR_MAGENTA);
            section_title.append(Item::GetItemClassName(cur_type));
            section_title.append(COLOUR_LGREY "]");
            m_lines.push_back(LineData('\0', -1, section_title));
            m_items.push_back(ItemH());
        }

        m_lines.push_back(LineData(iter->second, num, summary));
        m_items.push_back(iter->first);
    }
    m_num_lines = static_cast<int>(m_lines.size());
    scroll(0);
}


ItemPicker::ItemPicker(ItemPicker::Selected items, Selector::SelectType type,
                       int screenheight, CreatureH cr) :
    Selector(static_cast<int>(items.size()), 0, screenheight),
    m_items()
{
    m_seltype = type;
    char const * alpha_ptr = &AtoZ[0];

    for (Selected::iterator iter = items.begin(); iter != items.end(); ++iter)
    {
        char alphacode = *alpha_ptr++;
        if (alphacode == 'Z' + 1)
        {
            alphacode = 'a';
            alpha_ptr = &AtoZ[0] + 1;
        }
        int num = (*iter)->getNumber();
        std::string summary = (*iter)->describe(cr, 0, true);
        m_lines.push_back(LineData(alphacode, num, summary));
        m_items.push_back(*iter);
    }
    m_num_lines = static_cast<int>(m_lines.size());
    scroll(0);
}



ItemPicker::Selected
ItemPicker::getResult(Display *disp)
{
    processEvents(disp);
    Selected picker;
    Selected::iterator sel = m_items.begin();
    for (Lines::iterator iter = m_lines.begin(); iter != m_lines.end(); ++iter, ++sel)
    {
        if (iter->num_selected > 0)
        {
            (*sel)->addEffect(ItemEffect::Selected, ItemEffect(iter->num_selected));
            picker.push_back(*sel);
        }
    }
    return picker;
}



char
ItemPicker::getSeparator(int i)
{
    int num_selected = m_lines[i].num_selected;
    return (m_lines[i].num_to_select == -1) ? ' ' : (num_selected == 0) ? '-' :
        (num_selected != m_lines[i].num_to_select) ? '#' : '+';

}


void
ItemPicker::selectLine(int i, int num)
{
    if (m_seltype == Selector::DisplayOnly || m_lines[i].num_to_select == -1)
        return;
    if (num  == 0 || m_lines[i].num_selected + num > m_lines[i].num_to_select)
        m_lines[i].num_selected = m_lines[i].num_to_select;
    else
        m_lines[i].num_selected += num;
    m_redraw_required = true;
}


void
ItemPicker::deselectLine(int i, int num)
{
    if (m_seltype == Selector::DisplayOnly || m_lines[i].num_to_select == -1)
        return;
    if (num == 0 || m_lines[i].num_selected - num < 0)
        m_lines[i].num_selected = 0;
    else
        m_lines[i].num_selected -= num;
    m_redraw_required = true;
}


void
ItemPicker::toggleLine(int i, int num)
{
    if (m_seltype == Selector::DisplayOnly || m_lines[i].num_to_select == -1)
        return;
    if (m_lines[i].num_selected == 0 || num == 0)
        selectLine(i, num);
    else
        deselectLine(i, num);
    m_redraw_required = true;
}

//============================================================================
// InfoDisplayer
//============================================================================
InfoDisplayer::InfoDisplayer(int num_lines, int start_pos, int height, bool allow_help) :
    Selector(num_lines, start_pos, height, allow_help)
{
    m_num_lines = num_lines;
    m_seltype = Selector::DisplayOnly;
}


char
InfoDisplayer::getSeparator(int /*line*/)
{
    return ' ';
}

void
InfoDisplayer::selectLine(int /*line*/, int /*num*/)
{
}


void
InfoDisplayer::deselectLine(int /*line*/, int /*num*/)
{
}


void
InfoDisplayer::toggleLine(int /*line*/, int /*num*/)
{
}

//============================================================================
// LinePicker
//============================================================================
LinePicker::LinePicker(std::vector<std::string> const & lines, bool use_alpha, 
                       int start_pos, int height) :
    Selector(static_cast<int>(lines.size()), start_pos, height, true),
    m_selected(-1)
{
    m_seltype = Selector::SelectOne;

    char const *alpha_ptr = &AtoZ[0];

    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
    {
        m_lines.push_back(Selector::LineData(use_alpha ? *alpha_ptr++ : '\0', 1, *it));
        if (*alpha_ptr == '0')
            alpha_ptr = &AtoZ[0];
    }

    scroll(0);
}


void
LinePicker::selectLine(int line, int /*num*/)
{
    m_selected = line;
}



void 
LinePicker::deselectLine(int /*line*/, int /*num*/)
{
}


void
LinePicker::toggleLine(int line, int /*num*/)
{
    m_selected = (m_selected == -1) ? line : -1;
}


char
LinePicker::getSeparator(int /*num*/)
{
    return ' ';
}

int
LinePicker::getResult(Display *disp)
{
    processEvents(disp);
    return m_selected;
}

