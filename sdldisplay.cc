// -*- Mode: C++ -*-
// RogueMonkey Copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file COPYING

#include <algorithm>
#include <cassert>
#include <sstream>

#include "creature.h"
#include "dictionary.h"
#include "error.h"
#include "handles.h"
#include "hero.h"
#include "item.h"
#include "map.h"
#include "option.h"
#include "sdldisplay.h"
#include "sdlpanels.h"



SDL_Color RGB[Colour::EndColour] =
{
    {  0,   0,   0, 0},        // Black
    { 64,  64,  64, 0},        // DGrey
    {128, 128, 128, 0},        // LGrey
    {192, 192, 192, 0},        // Silver
    {255, 255, 255, 0},        // White
    {255,   0,   0, 0},        // Red
    {255, 110, 180, 0},        // Pink
    {228,  96,   0, 0},        // Orange
    { 96,  64,  32, 0},        // Brown
    {184, 115,  51, 0},        // Gold
    {228, 228,   0, 0},        // Yellow
    {  0,  96,   0, 0},        // DGreen
    {  0, 192,   0, 0},        // LGreen
    {  0,   0, 128, 0},        // DBlue
    {  0,   0, 255, 0},        // LBlue
    {  0, 192, 192, 0},        // Aqua
    {128,   0, 128, 0},        // Purple
    {192,  64, 192, 0}         // Magenta
};

std::string DefaultFontName = "data/ttf/VeraMono.ttf";
int DefaultFontSize = 0;
long DefaultFontIndex = 0;


//============================================================================
//============================================================================
// SDLDisplay
//============================================================================
//============================================================================

namespace
{
    SDL_Surface *Main_Surface = 0;
    SurfaceH Scratch;
}


SurfaceH
SDLDisplay::InitSDL(int width, int height, int bpp)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0)
        throw Error<DisplayE>("SDL Init failed");
    SurfaceH s(SDL_SetVideoMode(width, height, bpp, SDL_SWSURFACE), &DeleteMainSurface);
    if (!s.get())
        throw Error<DisplayE>("SDL set video mode failed");
    if (TTF_Init() < 0)
        throw Error<DisplayE>("SDL_ttf init failed");

    SDL_FillRect(s.get(), 0, 0);
    Main_Surface = s.get();
    Scratch = CreateSurface(s->w, s->h);
    return s;
}




Display *
SDLDisplay::create(OptionH opt)
{
    return new SDLDisplay(opt);
}


SDLDisplay::~SDLDisplay()
{
}


SDLDisplay::SDLDisplay(OptionH opt) :
    Display(opt),
    m_options(opt),

    m_mainsurf(InitSDL(opt->get<int>("Width"), opt->get<int>("Height"), opt->get<int>("ColourDepth"))),
    m_background(LoadSurface(opt->get("Background", ""))),

    m_main_map(new SDLMapPanel(opt->getSub("Map"))),
    m_main_stats(new SDLStatPanel(opt->getSub("Stats"))),
    m_main_messages(new SDLMessagePanel(opt->getSub("Messages"), this)),

    m_events(),
    m_popups(),

    m_background_opacity(opt->get<int>("BackgroundOpacity", 255)),
    m_inactive_opacity(opt->get<int>("InactiveOpacity", 128)),
    m_meta(0),
    m_max_hp(0),
    m_max_mana(0)
{
    DefaultFontName = opt->get("DefaultFont", DefaultFont);
    DefaultFontSize = opt->get<int>("DefaultFontSize", DefaultFontSize);
    DefaultFontIndex = opt->get<long>("DefaultFontIndex", DefaultFontIndex);

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    SDL_EnableUNICODE(1);
    SDL_WM_SetCaption("RogueMonkey " VERSION, "RogueMonkey " VERSION);
    //SDL_SetCursor();
    SDL_ShowCursor(SDL_ENABLE);

}


void
SDLDisplay::drawCellsToMap(MapH map, int tlx, int tly, int cols, int rows)
{
    m_main_map->clearAll();
    Coords const mapsize = map->getSize();
    Coords const dispsize = m_main_map->numCells();
    int extrax = (dispsize.X() - cols) / 2;
    int extray = (dispsize.Y() - rows) / 2;

    int map_start_x = tlx - extrax;
    int map_start_y = tly - extray;
    int dsp_start_x = (map_start_x < 0) ? -map_start_x : 0;
    int dsp_start_y = (map_start_y < 0) ? -map_start_y : 0;
    int map_end_x = tlx + cols + extrax > mapsize.X() ? mapsize.X() : tlx + cols + extrax;
    int map_end_y = tly + rows + extray > mapsize.Y() ? mapsize.Y() : tly + rows + extray;
    
    if (map_start_x < 0)
        map_start_x = 0;
    if (map_start_y < 0)
        map_start_y = 0;

    for (int y = map_start_y; y < map_end_y; ++y)
    {
        for (int x = map_start_x; x < map_end_x; ++x)
        {
            Representation rep(0, Colour::DGrey);
            if (y >= tly && y <  tly + rows && x >= tlx &&x < tlx + cols && map->getSeenGrid(x, y) != Map::Dark)
                rep = map->getRepresentation(Coords(x, y), HERO);
            else
                rep.first = map->getHeroSeenChar(x, y);

            if (rep.first)
                m_main_map->plotCharacter(x - map_start_x + dsp_start_x, y - map_start_y + dsp_start_y, rep.first, rep.second);
        }
    }
}


Coords
SDLDisplay::getMainMapSize() const
{
    return m_main_map->numCells();
}


void
SDLDisplay::printMessage(std::string const & msg)
{
    m_main_messages->printMessage(msg);
}


void
SDLDisplay::updateStats(unsigned int types)
{
    m_main_stats->updateStats(types);
}


void
SDLDisplay::showPreviousMessages()
{
    std::vector<std::string> lines = m_main_messages->getPreviousMessages();
    SDLInfoDisplayerH msgs(new SDLInfoDisplayer(m_options->getSub("GenericListPopup"), COLOUR_LBLUE "Previous:", 1, lines, true));
    m_popups.push_back(msgs);
    msgs->scroll(lines.size());
    msgs->processEvents(this);
    m_popups.pop_back();
    render();
}


void
SDLDisplay::render()
{
    SDL_FillRect(m_mainsurf.get(), 0, 0);
    if (m_background.get())
    {
        SDL_BlitSurface(m_background.get(), 0, m_mainsurf.get(), 0);
        SDL_SetAlpha(m_mainsurf.get(), SDL_SRCALPHA, m_background_opacity);
    }
    m_main_map->blitToSurface(m_mainsurf);
    m_main_messages->blitToSurface(m_mainsurf);
    m_main_stats->blitToSurface(m_mainsurf);
    m_main_messages->m_num_so_far = 0;

    int numpops = static_cast<int>(m_popups.size());
    for (PopupPanels::iterator it = m_popups.begin(); it != m_popups.end(); ++it, --numpops)
    {
        if (numpops == 1 && m_inactive_opacity < 255)
        {
            SurfaceH black = CreateSurface(m_mainsurf->w, m_mainsurf->h);
            SDL_FillRect(black.get(), 0, 0);
            SDL_SetAlpha(black.get(), SDL_SRCALPHA, m_inactive_opacity);
            SDL_BlitSurface(black.get(), 0, m_mainsurf.get(), 0);
        }
        SDL_SetAlpha(m_mainsurf.get(), 0, 255);
        (*it)->blitToSurface(m_mainsurf);
    }

    SDL_UpdateRect(m_mainsurf.get(), 0, 0, 0, 0);
}



Input::KeyPress
SDLDisplay::getRawKeyPress()
{
    typedef Dictionary<int, int> KeyLookup;
    typedef std::pair<int, int> Kpair;
    static Kpair const kd[] =
    {
        Kpair(SDLK_KP0, Input::KP0), Kpair(SDLK_KP1, Input::KP1),
        Kpair(SDLK_KP2, Input::KP2), Kpair(SDLK_KP3, Input::KP3),
        Kpair(SDLK_KP4, Input::KP4), Kpair(SDLK_KP5, Input::KP5),
        Kpair(SDLK_KP6, Input::KP6), Kpair(SDLK_KP7, Input::KP7),
        Kpair(SDLK_KP8, Input::KP8), Kpair(SDLK_KP9, Input::KP9),
        Kpair(SDLK_UP, Input::Up), Kpair(SDLK_DOWN, Input::Down),
        Kpair(SDLK_LEFT, Input::Left), Kpair(SDLK_RIGHT, Input::Right),
        Kpair(SDLK_KP_PERIOD, '.'), Kpair(SDLK_KP_DIVIDE, '/'),
        Kpair(SDLK_KP_MULTIPLY, '*'), Kpair(SDLK_KP_MINUS, '-'),
        Kpair(SDLK_KP_PLUS, '+'), Kpair(SDLK_KP_ENTER, 10),
        Kpair(SDLK_UP, Input::KP8), Kpair(SDLK_DOWN, Input::KP2),
        Kpair(SDLK_RIGHT, Input::KP6), Kpair(SDLK_LEFT, Input::KP4),
        Kpair(SDLK_HOME, Input::Home), Kpair(SDLK_END, Input::End),
        Kpair(SDLK_PAGEUP, Input::PageUp), Kpair(SDLK_PAGEDOWN, Input::PageDown),
        Kpair(SDLK_ESCAPE, Input::Esc), Kpair(SDLK_RETURN, Input::Enter),
        Kpair(SDLK_KP_ENTER, Input::Enter), Kpair(SDLK_TAB, Input::Tab),
        Kpair(SDLK_F1, Input::F1), Kpair(SDLK_F2, Input::F2),
        Kpair(SDLK_F3, Input::F3), Kpair(SDLK_F4, Input::F4),
        Kpair(SDLK_F5, Input::F5), Kpair(SDLK_F6, Input::F6),
        Kpair(SDLK_F7, Input::F7), Kpair(SDLK_F8, Input::F8),
        Kpair(SDLK_F9, Input::F9), Kpair(SDLK_F10, Input::F10),
        Kpair(SDLK_F11, Input::F11), Kpair(SDLK_F12, Input::F12)
    };

    static unsigned const kdsz = sizeof(kd) / sizeof(kd[0]);
    static KeyLookup special_lookup(&kd[0], &kd[0] + kdsz);
    KeyLookup::iterator i;
    Input::KeyPress keyhit(0);
    bool data_received = false;

    while (data_received == false)
    {
        if (SDL_WaitEvent(&m_events))
        {
            switch (m_events.type)
            {
            case SDL_KEYDOWN:
                keyhit.type = Input::KeyPress::Key;
                if (m_events.key.keysym.unicode < 128 && m_events.key.keysym.unicode > 0)
                {
                    keyhit.data.key = m_events.key.keysym.unicode;
                    data_received = true;
                }
                else
                {
                    i = special_lookup.find(m_events.key.keysym.sym);
                    if (i != special_lookup.end())
                    {
                        keyhit.data.key = i->second;
                        data_received = true;
                    }
                }

                // Meta key information stored separately so it works properly
                // with mouse events, too (SDL_Event is a union of event types)
                if (m_events.key.keysym.mod & KMOD_CTRL)
                    m_meta |= Input::Control;
                if (m_events.key.keysym.mod & KMOD_ALT)
                    m_meta |= Input::Alt;
                if (m_events.key.keysym.mod & KMOD_META)
                    m_meta |= Input::Meta;
                break;

            case SDL_KEYUP:
                if (m_events.key.keysym.mod & KMOD_CTRL)
                    m_meta &= ~Input::Control;
                if (m_events.key.keysym.mod & KMOD_ALT)
                    m_meta &= ~Input::Alt;
                if (m_events.key.keysym.mod & KMOD_META)
                    m_meta &= ~Input::Meta;
                break;

            case SDL_MOUSEBUTTONDOWN:
                keyhit.type = Input::KeyPress::Mouse;
                keyhit.data.coord.x = m_events.button.x;
                keyhit.data.coord.y = m_events.button.y;
                keyhit.data.coord.b = m_events.button.button;
                data_received = true;
                break;

            case SDL_QUIT:
                // TODO: Fix this
                throw Error<DisplayE>("SDL_QUIT event received");

            default:
                break;
            }
        }
    }

    keyhit.meta = m_meta;
    return keyhit;
}


ItemPicker::Selected
SDLDisplay::selectItemsFromPile(ItemPileH source, Item::ItemType classes,
                                Selector::SelectType sel, CreatureH cr,
                                std::string const & title)
{
    if (!source || source->empty())
        return ItemPicker::Selected();

    SDLItemPickerH picker(new SDLItemPicker(m_options->getSub("InventoryPopup"),
                                            source, classes, sel, title, cr));
    m_popups.push_back(picker);
    picker->scroll(0);
    ItemPicker::Selected result = picker->getResult(this);
    m_popups.pop_back();
    render();
    return result;
}


Input::KeyPress
SDLDisplay::askSingleKeyQuestion(std::string const & prompt, std::string const & allowed)
{
    Input::KeyPress kp(0);
    printMessage(prompt);
    render();

    while (!kp)
    {
        kp = getRawKeyPress();

        // TODO: Support mouse
        if (kp.type == Input::KeyPress::Mouse || allowed.end() ==
            std::find(allowed.begin(), allowed.end(), char(kp.data.key)))
        {
            kp.reset();
        }
    }
    return kp;
}



void
SDLDisplay::displayKeyHelp(Actions::Mode type)
{
    using namespace Actions;
    std::string title(COLOUR_WHITE "Keyboard commands available in " COLOUR_MAGENTA);
    title += (type == Normal) ? "Normal" : (type == Menu) ? "Menu" : "Targetting";
    title += COLOUR_WHITE " mode:";
    std::vector<std::string> lines;

    int max_help =       (type == Normal) ? static_cast<int>(NormalMode::EndNormalMode) :
                         (type == Menu)   ? static_cast<int>(MenuMode::EndMenuMode) :
                                            static_cast<int>(TargetMode::EndTargetMode);
    char const **names = (type == Normal) ? NormalMode::HelpNames :
                         (type == Menu)   ? MenuMode::HelpNames :
                                            TargetMode::HelpNames;

     //  0 is "Invalid" for all modes
    for (int i = 1; i < max_help; ++i)
    {
        std::string line = getKeySequence(type, i, Colour::White, COLOUR_LGREY " or ");
        line += '\t';
        line += names[i];
        lines.push_back(line);
    }

    SDLInfoDisplayerH helper(new SDLInfoDisplayer(m_options->getSub("HelpPopup"),  
                                                  title, 2, lines, type != Menu));
    m_popups.push_back(helper);
    helper->scroll(0);
    helper->processEvents(this);
    m_popups.pop_back();
    render();
}



int 
SDLDisplay::selectFromList(std::vector<std::string> const & vec, std::string const & title)
{
    SDLLinePickerH picker(new SDLLinePicker(m_options->getSub("GenericListPopup"), title, vec));
    m_popups.push_back(picker);
    picker->scroll(0);
    int result = picker->getResult(this);
    m_popups.pop_back();
    render();
    return result;
}


int 
SDLDisplay::selectFromList(std::vector<std::string> const & sel, std::vector<std::string> const & desc,
                           std::string const & title)
{
    SDLLinePickerH picker(new SDLLinePicker(m_options->getSub("SkillPopup"), title, sel));
    m_popups.push_back(picker);
    picker->scroll(0);
    int result = picker->getResult(this);
    m_popups.pop_back();
    render();
    return result;
}


#ifdef WIZARD
void 
SDLDisplay::drawWizardMap()
{
    Coords mypos = HERO->getCoords();
    int myposx = mypos.X(); int myposy = mypos.Y();
    MapH mymap = mypos.M();
    Coords mapsize = mymap->getSize();

    SDLMapPanelH pan(new SDLMapPanel(m_options->getSub("WizMiniMap")));
    m_popups.push_back(pan);
    Coords pansize = pan->numCells();

    Coords panctr((pansize.X() + 1) / 2, (pansize.Y() + 1) / 2);
    bool done = false; 
    bool redraw = true;
    while (!done)
    {
        // FIRST: clamp displayed area
        int maptlx = myposx - panctr.X();
        int maptly = myposy - panctr.Y();
        int mapbrx = myposx + panctr.X();
        int mapbry = myposy + panctr.Y();
        
        int pantlx = 0; int pantly = 0;
        int panbrx = pansize.X(); int panbry = pansize.Y();
        
        if (maptlx < 0)
        {
            pantlx -= maptlx; maptlx = 0;
        }
        if (maptly < 0)
        {
            pantly -= maptly; maptly = 0;
        }
        if (mapbrx > mapsize.X())
        {
            panbrx -= (mapbrx - mapsize.X()); mapbrx = mapsize.X();
        }
        if (mapbry > mapsize.Y())
        {
            panbry -= (mapbry - mapsize.Y()); mapbry = mapsize.Y();
        }

        // SECOND: display area on screen
        if (redraw)
        {
            pan->clearAll();
            for (int y = maptly; y < mapbry; ++y)
            {
                for (int x = maptlx; x < mapbrx; ++x)
                {
                    Representation rep = mymap->getRepresentation(Coords(x, y), HERO);
                    pan->plotCharacter(x - maptlx + pantlx, y - maptly + pantly, rep.first, rep.second);
                }
            }
            render();
            redraw = false;
        }

        // THIRD: process input
        Actions::NormalMode::Type act = getNormalAction();
        if (Actions::NormalMode::IsMove(act))
        {
            redraw = true;
            myposx += Actions::NormalMode::GetXMoveOffset(act) * 5;
            myposy += Actions::NormalMode::GetYMoveOffset(act) * 5;
        }
        else if (act == Actions::NormalMode::Cancel)
            done = true;
    }


    m_popups.pop_back();
    render();
}
#endif


Option::Sources 
SDLDisplay::getRequiredOpts() const
{
    static std::string req_opts[] = 
    {
        "Width", "Height", "ColourDepth", 
        "Map", 
        "Map/Font",
        "Map/FontSize",
        "Map/CellWidth",
        "Map/CellHeight",
        "Map/Width",
        "Map/Height",
        "Map/TopLeftX",
        "Map/TopLeftY",  

        "Stats", 
        "Stats/Font",
        "Stats/FontSize",
        "Stats/Width",
        "Stats/Height",
        "Stats/TopLeftX",
        "Stats/TopLeftY",  

        "Messages", 
        "Messages/Font",
        "Messages/FontSize",
        "Messages/Width",
        "Messages/Height",
        "Messages/TopLeftX",
        "Messages/TopLeftY", 
        "Messages/Logging",
        
        "InventoryPopup",
        "InventoryPopup/Font",
        "InventoryPopup/FontSize",
        "InventoryPopup/Width",
        "InventoryPopup/Height",

        "HelpPopup",
        "HelpPopup/Font",
        "HelpPopup/FontSize",
        "HelpPopup/Width",
        "HelpPopup/Height",

        "GenericListPopup",
        "GenericListPopup/Font",
        "GenericListPopup/FontSize",
        "GenericListPopup/Width",
        "GenericListPopup/Height",

        "WizMiniMap",
        "WizMiniMap", 
        "WizMiniMap/Font",
        "WizMiniMap/FontSize",
        "WizMiniMap/CellWidth",
        "WizMiniMap/CellHeight",
        "WizMiniMap/Width",
        "WizMiniMap/Height",
        "WizMiniMap/TopLeftX",
        "WizMiniMap/TopLeftY"
    };

    int const num_req_opts = sizeof(req_opts) / sizeof(std::string);
    return Option::Sources(&req_opts[0], &req_opts[0] + num_req_opts);
}

//============================================================================
// Global Funcs
//============================================================================
void
DeleteFont(TTF_Font *f)
{
    if (f)
        TTF_CloseFont(f);
}

void
DeleteSurface(SDL_Surface *s)
{
    if (s)
        SDL_FreeSurface(s);
}

void
DeleteMainSurface(SDL_Surface *)
{
    SDL_Quit();
}

SurfaceH
CreateSurface(int width, int height)
{
    return SurfaceH(SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
                                         Main_Surface->format->BitsPerPixel,
                                         Main_Surface->format->Rmask,
                                         Main_Surface->format->Gmask,
                                         Main_Surface->format->Bmask,
                                         Main_Surface->format->Amask),
                    &DeleteSurface);
}


namespace
{
    typedef std::pair<std::string, std::pair<int, long> > FontTuple;      
    typedef Dictionary<std::string, SurfaceH> ImageRepos;
    typedef Dictionary<FontTuple, FontH> FontRepos;

    ImageRepos image_repos;
    FontRepos  font_repos;
}





SurfaceH
LoadSurface(std::string const & filename)
{
    if (filename == "")
        return SurfaceH();

    ImageRepos::iterator it = image_repos.find(filename);
    if (it == image_repos.end())
    {
        SurfaceH img(SDL_LoadBMP(filename.c_str()), &DeleteSurface);
        it = image_repos.insert(ImageRepos::value_type(filename, img)).first;
    }
    return it->second;
}



FontH
LoadFont(std::string const & fontname, int ptsize, long face)
{
    FontTuple tuple(std::make_pair(fontname, std::make_pair(ptsize, face)));
    FontRepos::iterator it = font_repos.find(tuple);
    if (it == font_repos.end())
    {
        FontH font(TTF_OpenFontIndex(fontname.c_str(), ptsize, face), &DeleteFont);
        if (!font.get())
        {
            if (font_repos.empty())
            {
                Error<DisplayE> err("Failed to open font: ");
                err << fontname << '-' << ptsize << '-' << face;
                throw err;
            }
            it = font_repos.begin();
        }
        else
            it = font_repos.insert(FontRepos::value_type(tuple, font)).first;
    }
    return it->second;
}



SDL_Rect
CreateSDLRect(int x, int y, int w, int h)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    return rect;
}


SurfaceH
CreateTextSurface(std::string const & text, FontH font, Colour::Type col)
{
    SurfaceH surf(TTF_RenderUTF8_Blended(font.get(), text.c_str(), RGB[col]), &DeleteSurface);
    return surf;
}


SurfaceH
CreateSingleText(std::string const & text, FontH font, int width)
{
    int line_height = TTF_FontLineSkip(font.get());    
    SurfaceH surf = CreateSurface(width, line_height);
    int xpos = 0;
    Colour::ColourStringVec csv = Colour::SplitStringByColour(text, Colour::LGrey);
    for (Colour::ColourStringVec::iterator it = csv.begin(); it != csv.end() && xpos < width; ++it)
    {
        SurfaceH surface = CreateTextSurface(it->second, font, it->first);
        SDL_Rect dst = CreateSDLRect(xpos, 0, surface->w, line_height);
        SDL_BlitSurface(surface.get(), 0, surf.get(), &dst);
        xpos += surface->w;
    }
    return surf;
}



SurfaceH 
CreateMultiText(std::string const & text, FontH font, int width, int & num_lines)
{
    // BUG: Not thread-safe
    SDL_FillRect(Scratch.get(), 0, 0);
    int lheight = TTF_FontLineSkip(font.get());
    num_lines = 0;
    int xpos = 0;
    Colour::ColourStringVec csv = Colour::SplitStringByColour(text, Colour::LGrey);

    while (!csv.empty())
    {
        Colour::ColourStringVec::iterator csvit = csv.begin();

        // erase leading blanks if we're at the left margin
        if (xpos == 0)
        {
            std::string::iterator it = csvit->second.begin();
            while (it != csvit->second.end() && *it == ' ')
                ++it;
            csvit->second.erase(csvit->second.begin(), it);

            // restart the loop if the token consisted of nothing but blanks
            if (csvit->second.empty())
            {
                csv.pop_front();
                continue;
            }
        }

        SurfaceH attempt(CreateTextSurface(csvit->second, font, csvit->first));

        if (xpos + attempt->w <= width)
        {
            // yay, it fits
            SDL_Rect dest = CreateSDLRect(xpos, num_lines * lheight, attempt->w, lheight);
            SDL_BlitSurface(attempt.get(), 0, Scratch.get(), &dest);
            xpos += attempt->w;
            csv.pop_front();
        }
        else
        {
            // doesn't fit
            int guesspos = csvit->second.size() * (width - xpos) / attempt->w;           
            // find the earliest space character to split on
            while (guesspos && csvit->second[guesspos] != ' ')
                --guesspos;  

            if (guesspos > 0)
            {
                // we may still have room. put the guessed new fit in front
                std::string word(csvit->second.begin(), csvit->second.begin() + guesspos + 1);
                csv.push_front(Colour::ColourString(csvit->first, word));
                csvit->second.erase(csvit->second.begin(), csvit->second.begin() + guesspos + 1);
            }
            else
            {
                // bail out, word won't fit at all on the line
                if (xpos == 0)
                    break;

                // won't fit, so go to the next line
                xpos = 0;
                ++num_lines;              
            }
        }
    }

    ++num_lines;
    SurfaceH ret = CreateSurface(width, num_lines * lheight);
    SDL_Rect rect = CreateSDLRect(0, 0, width, num_lines * lheight);
    SDL_BlitSurface(Scratch.get(), &rect, ret.get(), 0);
    return ret;
}
    
//============================================================================
// SDL display registrar
//============================================================================
namespace
{
    struct SDLRegistrar
    {
        SDLRegistrar()
        {
            Display::Fact & fact(Display::GetFactory());
            fact.registerCreator("SDL", SDLDisplay::Create);
            fact.registerCreator("sdl", SDLDisplay::Create);
        }
    } registrar;
}

