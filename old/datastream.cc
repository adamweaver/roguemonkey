// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <sstream>


#include "datastream.h"
#include "world.h"

//============================================================================
// DataStream
//============================================================================
DataStream::DataStream(int hguid) :
    m_guid(hguid),
    m_num_data(0),
    m_data()
{

}


DataStream::~DataStream()
{
    m_data.clear();
    m_data << IPCMessage::IPCNames[IPCMessage::ShutDown];
    m_num_data = 1;
    transmit();
}

void
DataStream::sendChat(std::string const & sender, std::string const & chat)
{
    m_data << IPCMessage::IPCNames[IPCMessage::SendChat] << sender << chat;
    ++m_num_data;
}


void
DataStream::drawMap(int x, int y, FOV::PCells const & pcells)
{
    m_data << IPCMessage::IPCNames[IPCMessage::DrawMap] << x << y;
    int max = x * y;
    std::ostringstream out;
    for (int i = 0; i < max; ++i)
    {
        out << char(pcells[i].representation.first);
        out << char(pcells[i].representation.second);
        out << char(pcells[i].terrain);
    }
    m_data << out.str();
    ++m_num_data;
}
    

void
DataStream::updateStat(std::string const & statname, std::string const & val)
{
    m_data << IPCMessage::IPCNames[IPCMessage::UpdateStat] << statname << val;
    ++m_num_data;
}


void 
DataStream::sendMessage(std::string const & msg)
{
    m_data << IPCMessage::IPCNames[IPCMessage::SendMessage] << msg;
    ++m_num_data;
}


void
DataStream::transmit()
{
    NetString num_messages;
    num_messages << m_num_data;

    char datasize[50] = {0};
    unsigned sz = m_data.rawSize() + num_messages.rawSize();
    int numprinted = std::snprintf(&datasize[0], 49, "%d:", sz);
    char comma = ',';

    ServerSocket::MultiBuffers mb;
    mb.push_back(ServerSocket::MultiBuf(&datasize[0], numprinted));
    mb.push_back(ServerSocket::MultiBuf(num_messages.getRaw(), m_data.rawSize()));
    mb.push_back(ServerSocket::MultiBuf(m_data.getRaw(), m_data.rawSize()));
    mb.push_back(ServerSocket::MultiBuf(&comma, 1));
    World::TheWorld().writeMultiBuffers(m_guid, mb);

    m_data.clear();
    m_num_data = 0;
}

