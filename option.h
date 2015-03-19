#ifndef H_OPTION_
#define H_OPTION_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <cassert>
#include <string>
#include <vector>


#include "boost/enable_shared_from_this.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/shared_ptr.hpp"


#include "dictionary.h"
#include "error.h"
#include "handles.h"



class Option : public boost::enable_shared_from_this<Option>
{ 
private:
    struct Helper;
    friend struct Helper;
    
    typedef Dictionary<std::string, std::string> Vocab;
    typedef Dictionary<std::string, OptionH> Books;
    typedef boost::shared_ptr<Books> BooksH;
    
    mutable BooksH          m_books;
    mutable Vocab           m_vocab;
    std::string             m_title;
    std::string             m_super;

    Option(std::string const & title, std::string const & super, BooksH books);

    std::string const & getLeaf(std::string const & name) const;
    
public:
    typedef std::vector<std::string> Sources;

    static OptionH Create(Sources const & filenames);
    static OptionH Create(std::string const & filename);

    bool checkRequiredKeys(Sources const & req, std::string const &prefix) const;    
    bool hasSub(std::string const & name) const;
    bool hasKey(std::string const & name) const;
    OptionH getSub(std::string const & name) const;
    void dumpToStderr() const;

    template<typename T>
    T get(std::string const & name) const
    {
	    return boost::lexical_cast<T>(getLeaf(name));
    }
 

    std::string get(std::string const & name) const
    {
	    return getLeaf(name);
    }

    template<typename T>
    T get(std::string const & name, T def) const 
    {
        try
        {
            return boost::lexical_cast<T>(getLeaf(name));
        }
        catch (...)
        {
	        return def;
	    }
    }
    

    std::string get(std::string const & name, char const * def) const
    {
        try
	    {
            return getLeaf(name);
        } 
        catch (...)
        {
	        return def;
	    }
    }
    
    
    std::string get(std::string const & name, std::string const & def) const
    {
    	try
    	{
	        return getLeaf(name);
	    }
	    catch (...)
	    {
    	    return def;
    	}      
    }
};

#if 0
template<class StructType>
class InitStruct
{
    enum DataType
    {
        DTInt, DTString, DTChar, DTDouble, DTIndex, DTEnum
    };

    union DataPtr
    {
        int StructType::*         intptr;
        std::string StructType::* stringptr;
        char StructType::*        charptr;
        double StructType::*      doubleptr;
    };

    struct DataArea
    {
        std::string  fieldname;
        DataType     datatype;
        DataPtr      dataptr;
        char const **enums;
        int          maxenum;
    };

    typedef std::vector<DataArea> VDA;
    char const ** m_mainnames;
    StructType *  m_structs;
    int           m_num_mainnames;
    VDA           m_data_area;
    OptionH       m_option;


public:
    InitStruct(char const ** mainnames, StructType * structs, int num_mains,
               OptionH opt)
    :   m_mainnames(mainnames),
    m_structs(structs),
    m_num_mainnames(num_mains),
    m_data_area(),
    m_option(opt)
    {
    }

    void process()
    {
        for (int i = 0; i < m_num_mainnames; ++i)
        {
            OptionH opt = m_option->getSubOption(m_mainnames[i]);

            for (typename VDA::iterator b = m_data_area.begin();
                b != m_data_area.end(); ++b)
            {
                std::string vname;
                bool edone = false;

                switch (b->datatype)
                {
                case DTInt:
                    (m_structs + i)->*(b->dataptr.intptr) =
                    opt->getAsInt(b->fieldname);
                    break;

                case DTString:
                    (m_structs + i)->*(b->dataptr.stringptr) =
                    opt->getAsString(b->fieldname);
                    break;

                case DTChar:
                    (m_structs + i)->*(b->dataptr.charptr) =
                    opt->getAsString(b->fieldname)[0];
                    break;

                case DTDouble:
                    (m_structs + i)->*(b->dataptr.doubleptr) =
                    opt->getAsDouble(b->fieldname);
                    break;

                case DTIndex:
                    (m_structs + i)->*(b->dataptr.intptr) = i;
                    break;

                case DTEnum:
                    vname = opt->getAsString(b->fieldname);
                    for (int e = 0; !edone && e < b->maxenum; ++e)
                    {
                        if (vname.compare((b->enums)[e]) == 0)
                        {
                            (m_structs + i)->*(b->dataptr.intptr) = e;
                            edone = true;
                        }
                    }
                    if (!edone)
                    {
                        assert("Unknown enum");
                    }
                    break;
                default:
                    assert(!"Unhandled DataType");
                }
            }

        }
    }


    void addField(char const *fieldname, int StructType::* field)
    {
        if (fieldname && fieldname[0] != '\0')
        {
            DataArea area;
            area.fieldname = fieldname;
            area.datatype = DTInt;
            area.dataptr.intptr = field;
            m_data_area.push_back(area);
        }
    }

    void addEnum(char const *fieldname, int StructType::* field,
                 char const **enumnames, int mx)
    {
        if (fieldname && fieldname[0] != '\0')
        {
            DataArea area;
            area.fieldname = fieldname;
            area.datatype = DTEnum;
            area.dataptr.intptr = field;
            area.maxenum = mx;
            area.enums = enumnames;
            m_data_area.push_back(area);
        }
    }

    void addField(char const *fieldname, std::string StructType::* field)
    {
        if (fieldname && fieldname[0] != '\0')
        {
            DataArea area;
            area.fieldname = fieldname;
            area.datatype = DTString;
            area.dataptr.stringptr = field;
            m_data_area.push_back(area);
        }
    }

    void addField(char const *fieldname, char StructType::* field)
    {
        if (fieldname && fieldname[0] != '\0')
        {
            DataArea area;
            area.fieldname = fieldname;
            area.datatype = DTChar;
            area.dataptr.charptr = field;
            m_data_area.push_back(area);
        }
    }

    void addField(char const *fieldname, double StructType::* field)
    {
        if (fieldname && fieldname[0] != '\0')
        {
            DataArea area;
            area.fieldname = fieldname;
            area.datatype = DTDouble;
            area.dataptr.doubleptr = field;
            m_data_area.push_back(area);
        }
    }

    void addIndex(char const *fieldname, int StructType::* field)
    {
        if (fieldname && fieldname[0] != '\0')
        {
            DataArea area;
            area.fieldname = fieldname;
            area.datatype = DTIndex;
            area.dataptr.intptr = field;
            m_data_area.push_back(area);
        }
    }


    template<typename E>
    void processEnum(char const *fieldname, E StructType::* field,
                     char const **enums, E num_enums)
    {
        for (int i = 0; i < m_num_mainnames; ++i, ++m_mainnames, ++m_structs)
        {
            if (fieldname[0] == '\0')
                continue;

            OptionH opt = m_option->getSubOption(*m_mainnames);
            std::string val = opt->getAsString(fieldname);

            bool found = false;

            for (int j = 0; !found && j < num_enums; ++j)
            {
                if (val.compare(enums[j]) == 0)
                {
                    (m_structs + i)->*field = static_cast<E>(j);
                    found = true;
                    break;
                }
            }
            if (found == false)
            {
                assert("Enum not found");
            }
        }
    }

};
#endif



#endif

