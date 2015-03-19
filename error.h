#ifndef H_ERROR_
#define H_ERROR_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt


#include <sstream>
#include <stdexcept>

#include "boost/shared_ptr.hpp"



template<class T>
class Error : public std::exception
{
    boost::shared_ptr<std::ostringstream> m_err;

public:
    Error(char const *msg = "")
    :   std::exception(),
        m_err(new std::ostringstream(msg, std::ios_base::app |
                                     std::ios_base::ate | std::ios_base::out))
    {
    }

    virtual ~Error() throw()
    {
    }

    virtual char const * what() const throw()
    {
        return m_err->str().c_str();
    }

    template<typename U>
    std::ostream & operator<<(U u)
    {
        return (*m_err) << u;
    }
};



struct DisplayE
{
};

struct NotFoundE
{
};

struct FileE
{
};

struct FormatE
{
};

struct CommsE
{
};

#endif
