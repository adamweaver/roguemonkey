#ifndef H_LOG_
#define H_LOG_ 1

// -*- Mode: C++ -*-
// RogueMonkey Copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

void DumpToStderr(std::string const & str);


class Log
{   
public:
    typedef void (*DumpLine)(std::string const & str);

    typedef std::vector<std::string> History;
    typedef std::vector<DumpLine>    DumpLines;

    History             m_history;
    DumpLines           m_dumplines;
    std::ostringstream  m_line;

    Log() :
        m_history(),
        m_dumplines(),
        m_line()
    {
        m_dumplines.push_back(&DumpToStderr);
    }
    
public:
    struct EndLog {};
    
    static Log & Instance()
    {
        static Log logger;
        return logger;
    }

    template<typename U> std::ostream & operator<<(U u)
    {
        return m_line << u;
    }
    
    void operator<<(EndLog)
    {
        m_history.push_back(m_line.str());
        for (DumpLines::iterator it = m_dumplines.begin(); it != m_dumplines.end(); ++it)
            (*it)(m_line.str());
        m_line.str("");

    }

    int numLines() const
    {
        return static_cast<int>(m_history.size());
    }

    std::string const & getLine(int line) const
    {
        return m_history.at(line);
    }

    void addDumper(DumpLine d)
    {
        m_dumplines.push_back(d);
    }

    void delDumper(DumpLine d)
    {
        m_dumplines.erase(std::find(m_dumplines.begin(), m_dumplines.end(), d), m_dumplines.end());
    }
};
  

#define LOG_TRACE(L) Log::Instance() << "TRACE: " __FILE__ << ' ' << __LINE__ << " : " << L << Log::EndLog
#define LOG_DEBUG(L) Log::Instance() << "DEBUG: " __FILE__ << ' ' << __LINE__ << " : " << L << Log::EndLog
#define LOG_WARN(L)  Log::Instance() << "WARN:"   __FILE__ << ' ' << __LINE__ << " : " << L << Log::EndLog
#define LOG_CRIT(L)  Log::Instance() << "CRITICAL:" __FILE__ << ' ' << __LINE__ << " : " << L << Log::EndLog

          
#endif
