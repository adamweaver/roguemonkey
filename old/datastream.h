// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#ifndef H_DATASTREAM_
#define H_DATASTREAM_ 1

#include <vector>

#include "fov.h"
#include "handles.h"
#include "inputdef.h"
#include "netstring.h"


class DataStream
{
    int            m_guid;
    int            m_num_data;
    NetString      m_data;

public:
    DataStream(int hguid);
    ~DataStream();

    void transmit();

    void sendChat(std::string const & sender, std::string const & chat);
    
    void drawMap(int x, int y, FOV::PCells const & pcells);

    void updateStat(std::string const & statname, std::string const & val);

    void sendMessage(std::string const & msg);

    int getGUID() const { return m_guid; }

};








#endif

