// -*- Mode: C++ -*-
// RogueMonkey Copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file COPYING

#include <iomanip>
#include <sstream>

#include "display.h"
#include "hero.h"
#include "item.h"
#include "map.h"
#include "option.h"
#include "sdldisplay.h"
#include "sdlpanels.h"
#include "species.h"
#include "textutils.h"


namespace
{
    char const Ascii_Begin = 33;
    char const Ascii_End = 127;
}


//============================================================================
// SDLPanel
//============================================================================
SDLPanel::SDLPanel(OptionH opt) :
    m_font_name(opt->get("Font", DefaultFontName)),
    m_font_size(opt->get<int>("FontSize", DefaultFontSize)),
    m_font_index(opt->get<int>("FontIndex", DefaultFontIndex)),
    m_font(LoadFont(m_font_name, m_font_size, m_font_index)),
    m_line_height(TTF_FontLineSkip(m_font.get())),

    m_rect(CreateSDLRect(opt->get<int>("TopLeftX", -1), opt->get<int>("TopLeftY", -1),
                         opt->get<int>("Width"), opt->get<int>("Height"))),
    m_surface(CreateSurface(m_rect.w, m_rect.h)),

    m_outer_rect(),
    m_outer()
{
    SDL_Surface *all = SDL_GetVideoSurface();
    if (m_rect.x < 0)
        m_rect.x = (all->w - m_rect.w) / 2;
    if (m_rect.y < 0)
        m_rect.y = (all->h - m_rect.h) / 2;

    if (opt->hasSub("Container"))
    {
        opt = opt->getSub("Container");
        int left = opt->get<int>("LeftBorder");
        int right = opt->get<int>("RightBorder");
        int top = opt->get<int>("TopBorder");
        int bot = opt->get<int>("BottomBorder");

        m_outer_rect = CreateSDLRect(m_rect.x - left, m_rect.y - top, m_rect.w + right + left, m_rect.h + top + bot);
        m_outer = CreateSurface(m_outer_rect.w, m_outer_rect.h);
        SurfaceH bkg(LoadSurface(opt->get("Image")));
        if (bkg)
        {
            SDL_Rect src = CreateSDLRect(0, 0, m_outer_rect.w, m_outer_rect.h);
            SDL_BlitSurface(bkg.get(), &src, m_outer.get(), 0);
        }
    }

}


SDLPanel::SDLPanel(OptionH opt, int width, int height) :
    m_font_name(opt->get("Font", DefaultFontName)),
    m_font_size(opt->get<int>("FontSize", DefaultFontSize)),
    m_font_index(opt->get<int>("FontIndex", DefaultFontIndex)),
    m_font(LoadFont(m_font_name, m_font_size, m_font_index)),
    m_line_height(TTF_FontLineSkip(m_font.get())),

    m_rect(CreateSDLRect(opt->get<int>("TopLeftX", -1), opt->get<int>("TopLeftY", -1), width, height)),
    m_surface(CreateSurface(m_rect.w, m_rect.h)),

    m_outer_rect(),
    m_outer()    


SDLPanel::~SDLPanel()
{
}


void
SDLPanel::blitToSurface(SurfaceH surf)
{
    if (m_outer)
        SDL_BlitSurface(m_outer.get(), 0, surf.get(), &m_outer_rect);

    SDL_BlitSurface(m_surface.get(), 0, surf.get(), &m_rect);
}



void
SDLPanel::clearRect(int tlx, int tly, int brx, int bry)
{
    SDL_Rect dst = CreateSDLRect(tlx, tly, brx - tlx, bry - tly);
    SDL_FillRect(m_surface.get(), &dst, 0);
}


void
SDLPanel::clearAll()
{
    SDL_FillRect(m_surface.get(), 0, 0);
}



void
SDLPanel::hilightLine(int x, int y, Colour::Type col, int alpha)
{
    Uint32 rgba = SDL_MapRGBA(m_surface->format, RGB[col].r, RGB[col].g, RGB[col].b, alpha);
    SDL_Rect dest = CreateSDLRect(x, y * m_line_height, m_rect.w, m_line_height);
    SurfaceH line = CreateSurface(dest.w, dest.h);
    SDL_FillRect(line.get(), 0, rgba);
    SDL_SetAlpha(line.get(), SDL_SRCALPHA, alpha);
    SDL_BlitSurface(line.get(), 0, m_surface.get(), &dest);
}




int
SDLPanel::blitString(std::string const & str, int row, int xpos, int width, SurfaceH surf)
{
    SurfaceH text = CreateSingleText(str, m_font, width);
    SDL_Rect rect = CreateSDLRect(xpos, row * m_line_height, width, m_line_height);
    SDL_BlitSurface(text.get(), 0, surf.get(), &rect);
    return text->w;
}


int
SDLPanel::blitMultiString(std::string const & str, int row, int xpos, int width, SurfaceH surf)
{
    if (!surf)
        surf = m_surface;
    int num_lines = 0;

    SurfaceH text(CreateMultiText(str, m_font, width - xpos, num_lines));
    SDL_Rect rect(CreateSDLRect(xpos, row * m_line_height, text->w, text->h));
    SDL_BlitSurface(text.get(), 0, surf.get(), &rect);
    return num_lines;
}

//============================================================================
// SDLMapPanel
//============================================================================
SDLMapPanel::SDLMapPanel(OptionH opt) :
    SDLPanel(opt),
    m_cellwidth(opt->get<int>("CellWidth", 12)),
    m_cellheight(opt->get<int>("CellHeight", 12)),
    m_data_surf(InitMapCanvas(m_font_name, m_font, m_font_size, m_font_index)),
    m_terrain_surfaces(Map::EndTerrain)
{

}


SDLMapPanel::~SDLMapPanel()
{
}


SurfaceH
SDLMapPanel::InitMapCanvas(std::string const & fontname, FontH font, int fontsize, long fontindex)
{
    std::ostringstream fname(fontname, std::ios_base::app | std::ios_base::ate);
    fname << '_' << fontindex << '_' << fontsize << "pt_" << m_cellwidth << 'x' << m_cellheight << ".bmp";
    SurfaceH canvas(LoadSurface(fname.str()));
    if (canvas.get())
    {
        SDL_SetColorKey(canvas.get(), SDL_SRCCOLORKEY, 0);
        return canvas;
    }

    // Doesn't exist - must create & save
    TTF_SetFontStyle(font.get(), TTF_STYLE_BOLD);
    canvas = CreateSurface((Ascii_End - Ascii_Begin) * m_cellwidth, Colour::EndColour * m_cellheight);

    for (int c = Ascii_Begin; c < Ascii_End; ++c)
    {
        char line[2] = {'\0', '\0'};
        line[0] = c;

        // Skip Colour::Black
        for (int y = 1; y < Colour::EndColour; ++y)
        {
            // TTF_RenderGlyph_Blended puts short characters (like '.') in too small a bounding box
            SurfaceH character(TTF_RenderText_Blended(font.get(), line, RGB[y]), &DeleteSurface);
            SDL_Rect src_rect = CreateSDLRect(0, 0, m_cellwidth, m_cellheight);
            SDL_Rect dest_rect = CreateSDLRect((c - Ascii_Begin) * m_cellwidth, y * m_cellheight,
                                               m_cellwidth, m_cellheight);
            SDL_BlitSurface(character.get(), &src_rect, canvas.get(), &dest_rect);
        }
    }

    SDL_SetColorKey(canvas.get(), SDL_SRCCOLORKEY, 0);
    SDL_SaveBMP(canvas.get(), fname.str().c_str());
    return canvas;
}




Coords
SDLMapPanel::numCells() const
{
    return Coords(m_rect.w / m_cellwidth, m_rect.h / m_cellheight);
}


void
SDLMapPanel::plotCharacter(int x, int y, char c, Colour::Type col)
{
    assert(m_data_surf.get() && "Data Surface not initialised!");
    assert(c >= Ascii_Begin && c < Ascii_End && "Character out of range");

    SDL_Rect src =  CreateSDLRect((c - Ascii_Begin) * m_cellwidth, col * m_cellheight, m_cellwidth, m_cellheight);
    SDL_Rect dest = CreateSDLRect(x * m_cellwidth, y * m_cellheight, m_cellwidth, m_cellheight);
    SDL_BlitSurface(m_data_surf.get(), &src, m_surface.get(), &dest);
}


//============================================================================
// SDLMessagePanel
//============================================================================
SDLMessagePanel::SDLMessagePanel(OptionH opt, SDLDisplay *disp) :
    SDLPanel(opt),
    m_display(disp),
    m_past_messages(),
    m_y(0),
    m_num_lines(m_rect.h / m_line_height),
    m_repeat(0),
    m_num_so_far(0),
    m_current_turn(0),
    m_logging(false)
{
    if (opt->hasKey("Logging"))
        m_logging = TextUtils::IsTrue(opt->get("Logging"));
}


SDLMessagePanel::~SDLMessagePanel()
{
}


void
SDLMessagePanel::clearAll()
{
    SDLPanel::clearAll();
    m_y = 0;
}


int
SDLMessagePanel::numPastMessages() const
{
    return static_cast<int>(m_past_messages.size());
}


std::string const &
SDLMessagePanel::retrievePastMessage(int num) const
{
    assert(num >= 0 && num < numPastMessages() && "Invalid msg requested");
    return m_past_messages[num];
}



void
SDLMessagePanel::printMessage(std::string const & msg)
{
    static SurfaceH more = CreateSingleText("--more--", m_font, m_rect.w);
    if (m_logging)
    {
        if (m_past_messages.empty() || m_past_messages.back().compare(0, msg.size(), msg))
        {
            m_repeat = 1;
            m_past_messages.push_back(msg);
        }
        else
        {
            std::ostringstream out(msg, std::ios_base::ate | std::ios_base::app);
            out << " (x" << ++m_repeat << " times)";
            m_past_messages.back() = out.str();
        }
    }

    unsigned int now = HERO->getTurn();
    if (now != m_current_turn)
    {
        m_num_so_far = 0;
        m_current_turn = now;
    }

    int sz = 0;
    SurfaceH multi = CreateMultiText(msg, m_font, m_rect.w - LeftMargin - RightMargin, sz);
    for (int i = 0; i < sz; ++i)
    {
        SDL_Rect src = CreateSDLRect(0, i * m_line_height, multi->w, m_line_height);
        SDL_Rect dst = CreateSDLRect(LeftMargin, m_y * m_line_height, multi->w, m_line_height);
        SDL_BlitSurface(multi.get(), &src, m_surface.get(), &dst);
        if (++m_num_so_far == m_num_lines - 1)
        {
            dst.y = m_line_height * m_num_so_far;
            SDL_BlitSurface(more.get(), 0, m_surface.get(), &dst);
            m_display->render();
            m_display->getRawKeyPress();
            m_num_so_far = 0;
            clearRect(dst.x, dst.y, dst.w, dst.h + dst.y);
        }
        nl();
    }
}


void
SDLMessagePanel::nl()
{
    if (++m_y == m_num_lines)
    {
        SurfaceH old = CreateSurface(m_rect.w, m_rect.h);
        SDL_BlitSurface(m_surface.get(), 0, old.get(), 0);
        clearAll();
        SDL_Rect src = CreateSDLRect(0, m_line_height, m_rect.w, m_rect.h - m_line_height);
        SDL_BlitSurface(old.get(), &src, m_surface.get(), 0);
        m_y = m_num_lines - 1;
    }
}



std::vector<std::string> const &
SDLMessagePanel::getPreviousMessages() const
{   
    return m_past_messages;
}

//============================================================================
// SDLStatPanel
//============================================================================
SDLStatPanel::SDLStatPanel(OptionH opt) :
    SDLPanel(opt)
{

}


SDLStatPanel::~SDLStatPanel()
{
}

void
SDLStatPanel::updateHPMana(int pos, int cur, int mx)
{
    std::ostringstream str("");
    clearRect(int (m_rect.w / 2.5), pos * m_line_height, m_rect.w, (pos + 1) * m_line_height);
    str << (cur > mx         ? COLOUR_LGREEN :
            cur < mx / 4     ? COLOUR_RED :
            cur < mx / 2     ? COLOUR_ORANGE :
            cur < mx * 3 / 4 ? COLOUR_LGREY : COLOUR_WHITE)
            << std::setw(3) << cur << "  " COLOUR_WHITE "/  " << std::setw(3) << mx;
    blitString(str.str(), pos, int(m_rect.w / 2.5) + LeftMargin, m_rect.w, m_surface);

}


void
SDLStatPanel::updateStats(unsigned int types)
{
    static char const *StatNames[] = {COLOUR_GOLD "Health", COLOUR_GOLD "Mana", COLOUR_BROWN "Might", 
                                      COLOUR_BROWN "Agility", COLOUR_BROWN "Intellect", COLOUR_BROWN "Charisma"};
    std::ostringstream str;

    if (types & Display::Labels)
    {
        blitString(StatNames[0], Display::StatPos::Health_pos, LeftMargin, m_rect.w / 2, m_surface);
        blitString(StatNames[1], Display::StatPos::Mana_pos, LeftMargin, m_rect.w / 2, m_surface);

        for (int stat = 2; stat < 6; ++stat)
        {
            str.str("");
            str << StatNames[stat] << ':';
            blitString(str.str(), Display::StatPos::Stats_pos + stat - 2, LeftMargin, m_rect.w / 2, m_surface);
        }
        blitString(COLOUR_GOLD "Last action:", Display::StatPos::Action_pos, LeftMargin, m_rect.w, m_surface);
    }
    if (types & Display::NameUpdate)
    {
        clearRect(0, Display::StatPos::Name_pos * m_line_height, m_rect.w, (Display::StatPos::Name_pos + 1) * m_line_height);
        blitString(COLOUR_DGREEN "Spudboy", Display::StatPos::Name_pos, LeftMargin, m_rect.w, m_surface);
    }
    if (types & Display::LocationUpdate)
    {
        clearRect(0, Display::StatPos::Location_pos * m_line_height, m_rect.w, (Display::StatPos::Location_pos + 2) * m_line_height);
        blitString(COLOUR_LGREY "10.00am", Display::StatPos::Location_pos, LeftMargin, m_rect.w, m_surface);
        blitString(COLOUR_LGREY "Test Dungeon", Display::StatPos::Location_pos + 1, LeftMargin, m_rect.w, m_surface);
    }
    if (types & Display::HPUpdate)
    {
        Creature::StatPair sp(HERO->getStat(Creature::Health));
        updateHPMana(Display::StatPos::Health_pos, sp.first, sp.second);
    }
    if (types & Display::ManaUpdate)
    {
        Creature::StatPair sp(HERO->getStat(Creature::Mana));
        updateHPMana(Display::StatPos::Mana_pos, sp.first, sp.second);
    }
    if (types & Display::ExpUpdate)
    {
        clearRect(0, Display::StatPos::Exp_pos * m_line_height, m_rect.w, (Display::StatPos::Exp_pos + 1) * m_line_height);
        Classes::ClassLevels nc(HERO->getApparentClasses(HERO));
        Creature::StatPair exp(HERO->getStat(Creature::Experience));
        str.str("");
        str << "Level " << 1 << " / " << exp.first;
        if (exp.first != exp.second)
            str << COLOUR_DGREY " (" << exp.second << ")";
        blitString(str.str(), Display::StatPos::Exp_pos, LeftMargin, m_rect.w, m_surface);
    }
    if (types & Display::ClassesUpdate)
    {
        clearRect(0, Display::StatPos::Class_pos * m_line_height, m_rect.w, (Display::StatPos::Class_pos + 2) * m_line_height);
        Classes::ClassLevels nc(HERO->getApparentClasses(HERO));
        str.str("");
        for (Classes::ClassLevels::const_iterator c = nc.begin(); c != nc.end(); ++c)
        {
            if (c != nc.begin())
                str << COLOUR_DGREY " / ";
            str << COLOUR_WHITE << TextUtils::Cap1st(Classes::GetName(c->first)) << COLOUR_LGREY << '-' << c->second;
        }   

    }
    if (types & Display::StatUpdate)
    {
        clearRect(int(m_rect.w / 2.5), Display::StatPos::Stats_pos * m_line_height, m_rect.w, (Display::StatPos::Stats_pos + 4) * m_line_height);
        for (int stat = 2; stat < 6; ++stat)
        {
            str.str("");
            Creature::StatPair sp = HERO->getStat(static_cast<Creature::Stat>(stat));
            str << (sp.first < sp.second ? COLOUR_ORANGE :
                    sp.first > sp.second ? COLOUR_LGREEN : COLOUR_WHITE)
                << std::setw(3) << sp.first << "  " COLOUR_WHITE "/  " << std::setw(3) << sp.second;
            blitString(str.str(), Display::StatPos::Stats_pos + stat - 2, int(m_rect.w / 2.5) + LeftMargin, m_rect.w, m_surface);
        }
    }
    if (types & Display::EffectsUpdate)
    {
        clearRect(0, Display::StatPos::Cond_pos * m_line_height, m_rect.w, (Display::StatPos::Cond_pos + 2) * m_line_height);
    }    
    if (types & Display::EquipUpdate)
    {
        static int WeaponSlots[4] = {Species::BodySlot::WieldedR, Species::BodySlot::WieldedL, Species::BodySlot::Launcher};
        static char const *Headers[3] = {"PH:", "OH:", "R :"};

        for (int i = 0; i < 3; ++i)
        {
            str.str("");
            str << Headers[i];
            ItemH item = HERO->getInvInSlot(Species::BodySlot::Type(WeaponSlots[i]));
            if (item)
            {
                Representation irep = item->getRepresentation();
                str << Colour::ColourNames[irep.second] << irep.first << ' '
                    << COLOUR_LGREY << item->describe(HERO, 0, false);
            }
            blitString(str.str(), Display::StatPos::Equip_pos + i, LeftMargin, m_rect.w, m_surface);
        }
        str.str("");
        str << "B :" << HERO->getInventory()->numStacks() << " items in pack";
        blitString(str.str(), Display::StatPos::Equip_pos + 3, LeftMargin, m_rect.w, m_surface);
    }
    if (types & Display::LastActionUpdate)
    {
        clearRect(0, (Display::StatPos::Action_pos + 1) * m_line_height, m_rect.w, (Display::StatPos::Action_pos + 2) * m_line_height);
        Actions::NormalMode::Type act = HERO->getLastAction();
        if (act != Actions::NormalMode::Invalid)
            blitString(Actions::NormalMode::HelpNames[act], Display::StatPos::Action_pos + 1, LeftMargin, m_rect.w, m_surface);
    }
}

//============================================================================
// SDLPopup
//============================================================================
SDLPopup::SDLPopup(OptionH opt, int num_items, std::string const & title) :
    SDLPanel(opt),
    m_infopanel(),
    m_bigsurf(CreateSurface(m_rect.w, m_line_height * num_items))
{
    clearAll();
    blitString(title, 0, LeftMargin, m_rect.w - LeftMargin - RightMargin, m_surface);
}


void
SDLPopup::blitLinesToSurface(int top, int bot, int startline)
{
    SDL_Rect src = CreateSDLRect(0, top * m_line_height, m_rect.w, (bot - top) * m_line_height);
    SDL_Rect dst = CreateSDLRect(0, startline * m_line_height, m_rect.w, src.h);
    SDL_BlitSurface(m_bigsurf.get(), &src, m_surface.get(), &dst);
}


//============================================================================
// SDLItemPicker
//============================================================================
SDLItemPicker::SDLItemPicker(OptionH opt, ItemPileH ipile, Item::ItemType classes,
                             Selector::SelectType type, std::string const & title,
                             CreatureH cr) :
    SDLPopup(opt, ipile->numStacks() + 2 * Item::Num_Types, title),
    ItemPicker(ipile, classes, type, m_rect.h / m_line_height - 2, cr)
{
    for (int y = 0; y < static_cast<int>(m_lines.size()); ++y)
    {
        if (m_lines[y].alpha_shortcut)
        {
            static std::string alpha_shortcut("X");
            alpha_shortcut[0] = m_lines[y].alpha_shortcut;
            blitString(alpha_shortcut, y, LeftMargin, m_line_height, m_bigsurf);
        }
        blitString(m_lines[y].line, y, LeftMargin + m_line_height * 2 + Separator * 2, m_rect.w, m_bigsurf);
    }
}



void
SDLItemPicker::redraw()
{
    static std::string sep("X");
    clearRect(0, 2 * m_line_height, m_rect.w, m_rect.h);

    int top = getTopLine();
    int bot = getBotLine();

    blitLinesToSurface(top, bot, 2);
    for (int i = top; i < bot; ++i)
    {
        sep[0] = getSeparator(i);
        if (sep[0] != ' ')
            blitString(sep, 2 + i - top, LeftMargin + m_line_height * 2, m_line_height, m_surface);
    }
    if (m_seltype != Selector::DisplayOnly)
    {
        hilightLine(0, 2 + getHiliteLine(), Colour::Yellow, 128);
    }
}


//============================================================================
// SDLInfoDisplayer
//============================================================================
SDLInfoDisplayer::SDLInfoDisplayer(OptionH opt, std::string const & title, int cols,
                                   std::vector<std::string> const & lines,
                                   bool allow_help) :
    SDLPopup(opt, static_cast<int>(lines.size()), title),
    InfoDisplayer(static_cast<int>(lines.size()), 0, m_rect.h / m_line_height - 2, allow_help)
{
    typedef std::vector<std::string> StrLines;
    typedef std::vector<SurfaceH> Surfs;
    
    Surfs surfs(cols);
    std::vector<int> widths(cols, 0);
    int numlines = static_cast<int>(lines.size());

    for (int i = 0; i < cols; ++i)
        surfs[i] = CreateSurface(m_rect.w, numlines * m_line_height);

    for (int y = 0; y < numlines; ++y)
    {
        StrLines strlines = TextUtils::Tokenise(lines[y], "\t", "\\", "\"");
        blitString(strlines[0], y, 0, m_rect.w, surfs[0]);
        /*
        for (int x = 0; x < static_cast<int>(strlines.size()); ++x)
        {
            int w = blitString(strlines[x], y, 0, m_rect.w, surfs[x]);
            if (w > widths[x])
                widths[x] = w;
        }*/
    }
    SDL_BlitSurface(surfs[0].get(), 0, m_surface.get(), 0);
/*
    int xpos = LeftMargin;
    for (int x = 0; x < cols; ++x)
    {
        SDL_Rect src = CreateSDLRect(0, 0, xpos + widths[x], m_surface->h);
        SDL_Rect dst = CreateSDLRect(xpos, 0, widths[x], src.h);
        SDL_BlitSurface(surfs[x].get(), &src, m_surface.get(), &dst);
        xpos += widths[x] + Separator;
    }
  */
    scroll(0);
}




void
SDLInfoDisplayer::redraw()
{
    clearRect(0, 2 * m_line_height, m_rect.w, m_rect.h);
    blitLinesToSurface(getTopLine(), getBotLine(), 2);
}



//============================================================================
// SDLLinePicker
//============================================================================
SDLLinePicker::SDLLinePicker(OptionH opt, std::string const & title, 
                             std::vector<std::string> const & lines) :
    SDLPopup(opt, static_cast<int>(lines.size()), title),
    LinePicker(lines, true, 0, m_rect.h / m_line_height)
{
    typedef std::vector<std::string> StrLines;
    for (int y = 0; y < static_cast<int>(lines.size()); ++y)
    {
        StrLines strlines = TextUtils::Tokenise(m_lines[y].line, "\t", "\\", "\"");
        for (int x = 0; x < static_cast<int>(strlines.size()); ++x)
        {
            blitString(strlines[x], y, LeftMargin + x * 5 * m_line_height, m_rect.w, m_bigsurf);
        }
    }
    scroll(0);
}


void 
SDLLinePicker::redraw()
{
    clearRect(0, 2 * m_line_height, m_rect.w, m_rect.h);
    blitLinesToSurface(getTopLine(), getBotLine(), 2);    
    hilightLine(0, 2 + getHiliteLine(), Colour::Yellow, 128);
}
