// -*- Mode: C++ -*-
// RogueMonkey Copyringt 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#ifndef H_NETSTRING_
#define H_NETSTRING_ 1

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "error.h"

/**
 * NetString is a portable ASCII-based method of transmitting strings.
 * Format is ASCII representation: [len]":"[string]","
 * 0-length strings are "0:,"
 * "Hello World" would be "11:Hello World,"
 */
class NetString
{
    std::vector<char> m_data;
    int               m_idx;

    template<typename T>
    int printToBuf(T t, char *format, char *buf, int bufsize)
    {
        return std::snprintf(buf, bufsize - 1, format, t);
    }

    NetString & addToData(char *buf, int len)
    {
        char lenbuf[50] = {0};
        int lenlen = std::snprintf(lenbuf, sizeof(lenbuf) - 1, "%d:", len);
        m_data.insert(m_data.end(), &lenbuf[0], &lenbuf[lenlen]);
        m_data.insert(m_data.end(), buf, buf + len);
        m_data.push_back(',');
        return *this;
    }

    template<typename T>
    NetString & getNumber(T & t, char *format)
    {
        int numread = 0;
        std::sscanf(&m_data.front() + m_idx, format, &t, &numread);
        m_idx += numread;
        return *this;
    }

public:
    /**
     * Create a NetString already initialised with a string
     *
     * @param   s       string to initialise
     */
    explicit NetString(std::string const & s) :
        m_data(),
        m_idx(0)
    {
        *this << s;
    }

    /**
     * Create a NetString with an indication of potential size.
     * Allow a little extra for formatting data
     *
     * @param  size      size of expected data
     */
    explicit NetString(int size) :
        m_data(size, '\0'),
        m_idx(0)
    {
    }

    /**
     * Create an empty NetString
     */
    NetString() :
        m_data(),
        m_idx(0)
    {
    }

    /**
     * Clear the NetString and reset the read pointer to the beginning
     */
    void clear()
    {
        m_data.clear();
        m_idx = 0;
    }


    /**
     * Return the raw buffer being used. Dangerous. Should only be used
     * to fill buffer directly from another source i.e. socket read()
     *
     * @return        vector<char> buffer being used to store data
     */
    std::vector<char> & getRawBuffer()
    {
        return m_data;
    }

    char const *
    getRaw() const
    {
        return &m_data[0];
    }

    unsigned
    rawSize() const
    {
        return m_data.size();
    }

    /**
     * Compares two netstrings for equality
     * 
     * @param ns       netstring to compare
     * @return         true if equal
     */
    bool operator== (NetString const & ns)
    {
        return m_data == ns.m_data;
    }


    /**
     * Clears NetString and assigns data
     * 
     * @param T      Type of data to insert
     * @param t      data to insert
     * @return *this
     */
    template<typename T>
    NetString & operator=(T const & t)
    {
        clear();
        return *this << t;
    }

    //======================================================================
    // Input data
    //======================================================================
    NetString & operator<<(int t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%d", buf, sizeof(buf)));
    }

    NetString & operator<<(unsigned int t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%u", buf, sizeof(buf)));
    }

    NetString & operator<<(short t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%hd", buf, sizeof(buf)));
    }

    NetString & operator<<(unsigned short t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%hu", buf, sizeof(buf)));
    }

    NetString & operator<<(long t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%ld", buf, sizeof(buf)));
    }

    NetString & operator<<(unsigned long t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%lu", buf, sizeof(buf)));
    }

    NetString & operator<<(double t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%f", buf, sizeof(buf)));
    }

    NetString & operator<<(long double t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%lf", buf, sizeof(buf)));
    }

    NetString & operator<<(char t)
    {
        char single[5]="1:X,";
        single[2] = t;
        m_data.insert(m_data.end(), &single[0], &single[4]);
        return *this;
    }

    NetString & operator<<(char *t)
    {
        return addToData(t, std::strlen(t));
    }

    NetString & operator<<(std::string const &t)
    {
        return addToData(const_cast<char *>(t.c_str()), t.length());
    }

    NetString & operator<<(void *t)
    {
        char buf[50] = {0};
        return addToData(buf, printToBuf(t, "%p", buf, sizeof(buf)));
    }

    //======================================================================
    // Output Data
    //======================================================================
    NetString & operator>>(int & t)
    {
        return getNumber(t, "%*d:%d,%n");
    }

    NetString & operator>>(unsigned int & t)
    {
        return getNumber(t, "%*d:%u,%n");
    }

    NetString & operator>>(short & t)
    {
        return getNumber(t, "%*d:%hi,%n");
    }

    NetString & operator>>(unsigned short & t)
    {
        return getNumber(t, "%*d:%hu,%n");
    }

    NetString & operator>>(long & t)
    {
        return getNumber(t, "%*d:%li,%n");
    }

    NetString & operator>>(unsigned long & t)
    {
        return getNumber(t, "%*d:%lu,%n");
    }

    NetString & operator>>(double & t)
    {
        return getNumber(t, "%*d:%le,%n");
    }

    NetString & operator>>(long double & t)
    {
        return getNumber(t, "%*d:%Le,%n");
    }

    NetString & operator>>(char & t)
    {
        return getNumber(t, "%*d:%c,%n");
    }

    NetString & operator>>(std::string & t)
    {
        int string_len = 0;
        int num = 0;
        std::sscanf(&m_data.front() + m_idx, "%d:%n", &string_len, &num);
        m_idx += num;
        t.assign(&m_data.front() + m_idx, &m_data.front() + m_idx + string_len);
        t.append("\0");
        m_idx += string_len + 1;
        return *this;
    }

    template<typename T>
    T to()
    {
        T val;
        *this >> val;
        return val;
    }

    template<typename T>
    T toEnum(T maxenums, char const * names[])
    {
        std::string word = to<std::string>();
        for (int i = 0; i < maxenums; ++i)
            if (word == names[i])
                return static_cast<T>(i);
        throw Error<NotFoundE>("Enum not found");            
    }
};


#endif
