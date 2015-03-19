#ifndef H_EVENTS_
#define H_EVENTS_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt


struct Events
{
    enum Type
    {
        ActorMoved,

        EndEvents
    };

    struct Category
    {
        enum Type
        {
            HostileOnly                  =  0x0001,
            EquipmentManipulation        =  0x0002,

            AllEvents                    =  0xFFFF
        };
    };

    static bool fitsCategory(Type event, Category::Type category);
};





#endif

