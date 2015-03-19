// -*- Mode: C++ -*-
// RogueMonkey Copyringt 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

//#include <iostream>
//#include <ostream>

#include "boost/test/minimal.hpp"
#include "boost/thread/thread.hpp"

#include "tcpip.h"


void StartServer();
void StartClient();


int test_main(int, char **)
{
    std::srand(std::time(0));

    boost::thread_group tgroup;
    tgroup.create_thread(StartServer);

    tgroup.create_thread(StartClient);
    tgroup.create_thread(StartClient);
    tgroup.create_thread(StartClient);
    tgroup.create_thread(StartClient);
    tgroup.create_thread(StartClient);
    tgroup.create_thread(StartClient);

    tgroup.join_all();

    return 0;
}


void StartServer()
{
    ServerSocket server(5555);
    typedef std::queue<int> InputQ;
    std::vector<int> clientnums;

    InputQ iq;

    for (int loop = 0; loop < 1000; ++loop)
    {
        int preferred = -1;
        if (!clientnums.empty())
            preferred = rand() % clientnums.size();
        iq = server.waitForInput(preferred == -1 ? 0 : clientnums[preferred], 50);
//        std::cout << "srv: num conns pending: " << iq.size() << ", preferring " 
//                  << (preferred == -1 ? -1 : clientnums[preferred]) << "\n";

        while (!iq.empty())
        {
            int which = iq.front();
            iq.pop();

            if (which == 0)
            {
                int clnum = server.acceptConnection();
                BOOST_CHECK(clnum >= 0);
//                std::cout << "server accepted client # " << clnum << '\n';
                clientnums.push_back(clnum);
            }
            else
            {
                NetString nsdata(server.readString(which));
                server.writeString(which, nsdata);
                std::vector<char> & ns = nsdata.getRawBuffer();
                ns.push_back('\0');
//                std::cout << "Client #" << which << " sent " << &ns[0] << "\n";
            }
        }
        if (loop % 100 == 0)
        {
//            std::cout << "server " << loop << '\n';
        }
    }
}



void StartClient()
{

    ClientSocket cl("localhost", 5555);
//    std::cout << "Client connected\n";

    int numsent = 0;

    for (int i = 0; i < 100; ++i)
    {
        NetString ns;
        int rnd = std::rand();
        ns << rnd;
        cl.writeString(ns);
        NetString ret(cl.readString());
        int rnd1;
        ret >> rnd1;
        BOOST_CHECK(rnd == rnd1);
        ++numsent;
    }
}

