// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

/****************************************************************************
 * Some code adapted from Effective TCP/IP Programming, by Jon C. Snader
 * Copyright 2000 by Addison Wesley
 *
 * Addison Wesley have graciously given permission for reuse -
 * http://home.netcom.com/~jsnader/etcpabout.html
 ****************************************************************************/

#include <iostream>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstring>
#include <set>
#include <utility>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "boost/thread/xtime.hpp"

#include "error.h"
#include "handles.h"
#include "tcpip.h"


//============================================================================
// SocketData
//============================================================================
class SocketData
{
    friend class ServerSocket;
    friend class ClientSocket;

    sockaddr_in      address;
    int              sock;
    boost::xtime     xtime_last_accessed;
    static int       on;

public:
    SocketData() :
        address(),
        sock(),
        xtime_last_accessed()
    {
        std::memset(&address, 0, sizeof(sockaddr_in));
        boost::xtime_get(&xtime_last_accessed, boost::TIME_UTC);
    }

    ~SocketData()
    {
        close(sock);
    }

    void serverSetAddress(unsigned short port)
    {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
    }

    void clientSetAddress(char const *sname, unsigned short port)
    {
        address.sin_family = AF_INET;
        if (!inet_aton(sname, &address.sin_addr))
        {
            hostent *hent = gethostbyname(sname);
            if (hent == 0)
                throw Error<CommsE>("Unknown server name specified");
            address.sin_addr = *(in_addr *)hent->h_addr;
        }
        address.sin_port = htons(port);
    }

    bool isValidSocket()
    {
        return sock >= 0;
    }

    int inetAton(char const *cp, in_addr *pin)
    {
        int rc = inet_addr(cp);
        if (rc < 0 && strcmp(cp, "255.255.255.255"))
            return 0;
        pin->s_addr = rc;
        return 1;
    }


    int readNumBytes(int bytes, char *arr)
    {
        int readcount = 0;
        int numret = 0;

        while (bytes > 0)
        {
            readcount = recv(sock, arr, bytes, 0);
            if (readcount < 0)
            {
                if (errno == EINTR)
                    continue;
                Error<CommsE> err("Unknown errno in readNumBytes(): ");
                err << errno;
                throw err;
            }
            numret += readcount;
            arr += readcount;
            bytes -= readcount;
        }
        return numret;
    }


    NetString readString()
    {
        int num_msg = 0;
        char char_read = 0;
        do
        {
            readNumBytes(1, &char_read);
            if (std::isdigit(char_read))
            {
                num_msg *= 10;
                num_msg += char_read - '0';
            }
        } while (char_read != ':');

        NetString ns(num_msg + 2);
        std::vector<char> & data = ns.getRawBuffer();
        char *out = &data[0];
        readNumBytes(num_msg + 1, out);
        data.erase(data.end() - 1, data.end());
        return ns;
    }

    int doSend(char *data, int bytes)
    {
        int sentcount = 0;
        while (bytes > 0)
        {
            sentcount = send(sock, data, bytes, 0);
            if (sentcount < 0)
            {
                if (errno == EINTR)
                    continue;
                Error<CommsE> err("Unknown errno in sendNumBytes(): ");
                err << errno;
                throw err;
            }
            if (sentcount == 0)
                break;
            data += sentcount;
            bytes -= sentcount;
        }
        return sentcount;
    }

    int writeString(NetString & data)
    {
        char out[50] = {'\0'};
        std::vector<char> & dt = data.getRawBuffer();
        int nump = std::sprintf(&out[0], "%u:", dt.size());
        dt.push_back(',');
        assert(nump < 50 && "Too long number string written");
        int numsent = doSend(&out[0], nump);
        return numsent + doSend(&dt[0], dt.size());
    }

    int writeMultiBuffer(ServerSocket::MultiBuffers & b)
    {
        std::vector<iovec> iov;
        for (ServerSocket::MultiBuffers::iterator i = b.begin(); i != b.end(); ++i)
        {
            iovec v;
            v.iov_base = (void *)(i->buf);
            v.iov_len = i->len;
            iov.push_back(v);
        }
        return writev(sock, &iov[0], iov.size());
    }
};

int SocketData::on = 1;

//============================================================================
// ServerSocket
//============================================================================
ServerSocket::ServerSocket(unsigned short port) :
    server(new SocketData),
    clients()
{
    server->sock = socket(AF_INET, SOCK_STREAM, 0);
    server->serverSetAddress(port);

    if (!server->isValidSocket())
        throw Error<CommsE>("server: socket call failed");

    if (setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR,
                   &SocketData::on, sizeof(SocketData::on)) < 0)
        throw Error<CommsE>("server: setsockopt failed");

    int opts = fcntl(server->sock, F_GETFL);
    if (opts < 0)
        throw Error<CommsE>("server: failed to get standard socket opts");
    if (fcntl(server->sock, F_SETFL, opts | O_NONBLOCK) < 0)
        throw Error<CommsE>("server: failed to set nonblocking socket opts");

    if (bind(server->sock, (sockaddr *)&server->address, sizeof(server->address)) < 0)
        throw Error<CommsE>("server: bind failed");

    if (listen(server->sock, 8) < 0)
        throw Error<CommsE>("server: listen failed");
}



int
ServerSocket::acceptConnection()
{
    SocketDataH cl = SocketDataH(new SocketData);
    socklen_t len = sizeof(cl->address);
    cl->sock = accept(server->sock, (sockaddr *)&cl->address, &len);

    if (!cl->isValidSocket())
        return -1;

    clients.insert(cl->sock, cl);
    boost::xtime_get(&cl->xtime_last_accessed, boost::TIME_UTC);
    return cl->sock;
}


NetString
ServerSocket::readString(int pos)
{
    assert (clients.find(pos) != clients.end() && "Invalid Client# requested");
    return clients[pos]->readString();
}


int
ServerSocket::writeString(int pos, NetString & data)
{
    assert (clients.find(pos) != clients.end() && "Invalid Client# requested");
    return clients[pos]->writeString(data);
}



int
ServerSocket::writeMultiBuffer(int pos, MultiBuffers & b)
{
    assert (clients.find(pos) != clients.end() && "Invalid Client# requested");
    assert (!b.empty() && "Empty Multibuffers passed");
    return clients[pos]->writeMultiBuffer(b);
}


namespace
{
    typedef std::pair<boost::xtime, int> PendingPair;
    struct PairXTimeCmp : public std::binary_function<PendingPair, PendingPair, bool>
    {
        bool operator() (PendingPair l, PendingPair r)
        {
            return boost::xtime_cmp(l.first, r.first) == -1;
        }
    };
}


std::queue<int>
ServerSocket::waitForInput(int clientturn, long tm)
{
    fd_set sock_set;
    FD_ZERO(&sock_set);
    FD_SET(server->sock, &sock_set);
    int highsock = server->sock;

    for (Clients::iterator i = clients.begin(); i != clients.end(); ++i)
    {
        FD_SET(i->second->sock, &sock_set);
        if (i->second->sock > highsock)
            highsock = i->second->sock;
    }

    timeval timeout;
    timeout.tv_sec = tm / 1000;
    timeout.tv_usec = tm % 1000;
    int readysocks = select(highsock + 1, &sock_set, 0, 0, &timeout);

    if (!readysocks)
        return std::queue<int>();

    typedef std::vector<PendingPair> Pending;
    Pending p;
    for (Clients::iterator i = clients.begin(); i != clients.end(); ++i)
    {
        if (FD_ISSET(i->second->sock, &sock_set))
        {
            p.push_back(i->first == clientturn ?
                        PendingPair(boost::xtime(), i->first) :
                        PendingPair(i->second->xtime_last_accessed, i->first));
            boost::xtime_get(&i->second->xtime_last_accessed, boost::TIME_UTC);
        }
    }

    std::sort(p.begin(), p.end(), PairXTimeCmp());
    std::queue<int> q;
    for (Pending::iterator i = p.begin(); i != p.end(); ++i)
    {
        q.push(i->second);
    }

    // new connections are handled last
    if (FD_ISSET(server->sock, &sock_set))
    {
        q.push(0);
    }

    return q;
}

//============================================================================
// ClientSocket
//============================================================================
ClientSocket::ClientSocket(std::string const & host, unsigned short port) :
    server(new SocketData)
{
    server->sock = socket(AF_INET, SOCK_STREAM, 0);
    server->clientSetAddress(host.c_str(), port);

    if (!server->isValidSocket())
        throw Error<CommsE>("client: socket call failed");

    if (setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR,
                   &SocketData::on, sizeof(SocketData::on)) < 0)
        throw Error<CommsE>("client: setsockopt failed");

    if (connect(server->sock, (sockaddr *)&server->address, sizeof(sockaddr_in)) < 0)
        throw Error<CommsE>("client: connect failed");
}

NetString
ClientSocket::readString()
{
    return server->readString();
}


int
ClientSocket::writeString(NetString & ns)
{
    return server->writeString(ns);
}


bool
ClientSocket::outstandingMessage() const
{
    fd_set sock_set;
    FD_ZERO(&sock_set);
    FD_SET(server->sock, &sock_set);
    int highsock = server->sock;

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    int readysocks = select(highsock + 1, &sock_set, 0, 0, &timeout);
    return readysocks;
}


