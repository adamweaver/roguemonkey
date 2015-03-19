// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include "events.h"



namespace
{
    Events::Category::Type category[Events::EndEvents] =
    {
        Events::Category::AllEvents                    // ActorMoved
    };
}


bool
Events::fitsCategory(Events::Type event, Events::Category::Type categ)
{
    // Yes, this is stupid, but it avoids "warning C4800" on VC2005
    return category[event] & categ ? true : false;
}

