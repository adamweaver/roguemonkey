// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

//#include "boost/program_options.hpp"

#include "armour.h"
#include "display.h"
#include "error.h"
#include "handles.h"
#include "hero.h"
#include "inputdef.h"
#include "option.h"
#include "skills.h"
#include "weapon.h"
#include "world.h"

void DumpToStderr();
void Set_Up();


namespace
{
    std::string Req_Opts[] = 
    {
        "RogueMonkey",
        "RogueMonkey/Display",
        "RogueMonkey/Display/Keys",
        "RogueMonkey/Display/Keys/Normal",
        "RogueMonkey/Display/Keys/Menu",
        "RogueMonkey/Display/Keys/Targetting",
        "RogueMonkey/Display/Name"
    };
    int const Num_Req_Opts = sizeof(Req_Opts) / sizeof(std::string);
}


int
main(int /*argc*/, char /* **argv*/)
try
{
/*   
_GLIBCXX_DEBUG
 namespace PO = boost::program_options;
    std::string conf;
    
    PO::options_description opt_desc("Allowed options");
    opt_desc.add_options()
        ("help", "produce help message")
        ("version", "version information")
        ("config,c", PO::value<std::string>(&conf)->default_value("init.ini"),
         "path to configuration file to be used");

    PO::variables_map var_map;
    PO::store(PO::parse_command_line(argc, argv, opt_desc), var_map);
    PO::notify(var_map);

    if (var_map.count("help"))
    {
        std::cout << opt_desc << std::endl;
        return 0;
    }

    if (var_map.count("version"))
    {
        std::cout << VERSION << std::endl;
        return 0;
    }
*/
    std::vector<std::string> optnames;
    optnames.push_back("init.ini");

#ifdef _WINDOWS_
    #define HOME "%HOMEPATH%/RogueMonkey"
#else
    #define HOME "~/.RogueMonkey"
#endif    
    optnames.push_back(HOME "/init.ini");
#undef HOME

    OPTION = Option::Create(optnames);
    Option::Sources required_keys(&Req_Opts[0], &Req_Opts[0] + Num_Req_Opts);
    OPTION->checkRequiredKeys(required_keys, "");
    OptionH displayopts = OPTION->getSub("RogueMonkey/Display");

    std::string displaytype = displayopts->get("Name", "SDL");
    DISPLAY = Display::GetFactory().create(displaytype, displayopts);
    displayopts->checkRequiredKeys(DISPLAY->getRequiredOpts(), "");

    Set_Up();
    World::Init();

    HERO = Hero::CreateHero();
    World::TheWorld().mainLoop(HERO);

    return 0;
}
catch(std::exception & e)
{
    std::cerr << "Error caught in main loop: " << e.what() << std::endl;
}
catch(...)
{
    std::cerr << "Unknown exception caught in main loop." << std::endl;
}


void
Set_Up()
{
    static struct SUI_Init
    {
        void (*Initter)();
        char const *Name;
    } initters[] = 
    {
        {Weapon::Init, "Weapon"},
        {Armour::Init, "Armour"},

        {Classes::Init, "Classes"}
    };
    static int const num_initters = sizeof(initters) / sizeof(SUI_Init);
    int i = 0;
    try
    {
        for ( ; i < num_initters; ++i)
            initters[i].Initter();
    }
    catch(...)
    {
        std::cerr << "Error in Option file for " << initters[i].Name << "\n";
        throw;
    }
}


void
DumpToStderr(std::string const & str)
{
    std::cerr << str << '\n';
}

