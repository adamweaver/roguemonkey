// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#ifndef H_NUMCONVERTER_
#define H_NUMCONVERTER_ 1

#include <climits>
#include <netinet/in.h>

namespace Display
{
    // base structure & fake specialisation to catch unsupported sizes
    template<typename T, unsigned int Size>
    struct DoConv;


    template<typename T>
    struct DoConv<T, 8U>
    {
        static T ToNetwork(T num)
        {
            return num;
        }

        static T ToHost(T num)
        {
            return num;
        }
    };


    template<typename T>
    struct DoConv<T, 16U>
    {
        static T ToNetwork(T num)
        {
            return htons(num);
        }

        static T ToHost(T num)
        {
            return ntohs(num);
        }
    };

    
    template<typename T>
    struct DoConv<T, 32U>
    {
        static T ToNetwork(T num)
        {
            return htonl(num);
        }

        static T ToHost(T num)
        {
            return ntohl(num);
        }
    };

    // assumes that 64bit is a long, so ints are 32bits. Very unportable,
    // but until we get a htonll() we're stuck. I really don't want to
    // have to start typedef-ing integer sizes. 
    template<typename T>
    struct DoConv<T, 64U>
    {
        union Hack
        {
            T val;
            unsigned raw[2];
        };

        static T ToNetwork(T num)
        {
            Hack dodgy;
            dodgy.val = num;
            dodgy.raw[0] = DoConv<unsigned, 32U>::ToNetwork(dodgy.raw[0]);
            dodgy.raw[1] = DoConv<unsigned, 32U>::ToNetwork(dodgy.raw[1]);
            return dodgy.val;
        }

        static T ToHost(T num)
        {
            Hack dodgy;
            dodgy.val = num;
            dodgy.raw[0] = DoConv<unsigned, 32U>::ToHost(dodgy.raw[0]);
            dodgy.raw[1] = DoConv<unsigned, 32U>::ToHost(dodgy.raw[1]);
            return dodgy.val;
        }
    };


    /**
     * Coverts to/from Network Byte Order (big-endian). DoConv<> actually
     * does the work
     */
    template<typename T>
    struct Conv
    {
        /**
         * Covert a host integer to network byte order
         *
         * @param num       integer
         * @return          integer in correct format
         */
        static T ToNetwork(T num)
        {
            return DoConv<T, sizeof(T) * CHAR_BIT>::ToNetwork(num);
        }

        /**
         * Convert a network integer to host order
         *
         * @param num      integer
         * @return         integer in correct format
         */
        static T ToHost(T num)
        {
            return DoConv<T, sizeof(T) * CHAR_BIT>::ToHost(num);
        }
    };

    // fake specialisation to give compile-time error for illegal types
    template<double>
    struct Conv
    {
    };
    
    // fake specialisation to give compile-time error for illegal types
    template<long double>
    struct Conv
    {
    };

    // fake specialisation to give compile-time error for illegal types  
    template<float>
    struct Conv
    {
    };
}



#endif

