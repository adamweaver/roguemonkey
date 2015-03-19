// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#ifndef H_TCPIP_
#define H_TCPIP_ 1

#include <queue>
#include <string>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "dictionary.h"
#include "netstring.h"

class ServerSocket;
typedef boost::shared_ptr<ServerSocket> ServerSocketH;

class ClientSocket;
typedef boost::shared_ptr<ClientSocket> ClientSocketH;

class SocketData;
typedef boost::shared_ptr<SocketData> SocketDataH;


/**
 * Server class owning clients and multiplexing for input from clients
 */
class ServerSocket
{
    typedef Dictionary<int, SocketDataH> Clients;

    SocketDataH                    server;
    Clients                        clients;

public:
    struct MultiBuf
    {
        char const *buf;
        unsigned len;

        MultiBuf(char const *b, unsigned l) :
            buf(b),
            len(l)
        {
        }
    };
    typedef std::vector<MultiBuf>  MultiBuffers;

    /**
     * Create a ServerSocket ready to listen() for new connections
     *
     * @param  port      port to listen() on
     */
    ServerSocket(unsigned short port);

    /**
     * Checks to see which sockets have pending input, and update list of who
     * received input when.
     * 
     * @param  turn      client whose turn is next
     * @param  tm        microseconds to wait
     * @return           queue of client numbers. 0 is new connection waiting
     */
    std::queue<int> waitForInput(int turn, long tm);

    /**
     * Accept a new connection, return client number
     *
     * @return           client number accepted, or 0 for failure
     */
    int acceptConnection();

    /**
     * Read some data from a client
     *
     * @param  pos       client number to read from
     * @return           NetString of data returned
     */
    NetString readString(int pos);

    /**
     * Write a NetString to a client
     *
     * @param  pos       client number to write to
     * @param  data      NetString to write
     * @return           Number of bytes written
     */
    int writeString(int pos, NetString & data);

    /**
     * Write MultiBuffers to a client
     *
     * @param  pos      client number to write to
     * @param  b        MultiBuffers to write
     * @return          Total number of bytes written
     */
    int writeMultiBuffer(int pos, MultiBuffers & b);
};




/**
 * Client connection to server
 */
class ClientSocket
{
    SocketDataH server;

public:
    /**
     * Creates a connection to a server
     * 
     * @param host   host ipo address (dotted quad or name)
     * @param port   port to connect
     */
    ClientSocket(std::string const & host, unsigned short port);

    /**
     * Has the server sent any unread messages?
     *
     * @return      true if a message sent
     */
    bool outstandingMessage() const;

    /**
     * Read a NetString from the server
     * 
     * @return NetString written by server
     */
    NetString readString();

    /**
     * Write a NetString to the server
     * 
     * @param ns     NetString to write
     * @return number of characters written
     */
    int writeString(NetString & ns);
};



#endif

