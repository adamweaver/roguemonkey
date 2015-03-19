// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <utility>

#include "boost/bind.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/depth_first_search.hpp"
#include "boost/graph/visitors.hpp"
#include "boost/spirit/core.hpp"

#include "error.h"
#include "option.h"
#include "textutils.h"


OptionH OPTION;

//============================================================================
// Option
//============================================================================
Option::Option(std::string const & title, std::string const & super, BooksH books) :
    m_books(books),
    m_vocab(),
    m_title(title),
    m_super(super)
{
}


OptionH
Option::Create(std::string const & filename)
{
    Sources src(1, filename);
    return Create(src);
}

namespace
{
    struct CycleDetector : public boost::dfs_visitor<>
    {
        CycleDetector(bool & has_cycle) : m_has_cycle(has_cycle) { }
        template<class Edge, class Graph>
        void back_edge(Edge, Graph &) { m_has_cycle = true; }
        bool & m_has_cycle;
    };
    
}


struct Option::Helper
{
    typedef std::pair<int, int> Edge;
    typedef std::vector<Edge> EdgeList;
    typedef std::map<std::string, int> Vertices;
    
    std::string     ident;             // boost.spirit will fill in     
    std::string     value;             // boost.spirit will fill in
    OptionH         current;           // current option (book)
    BooksH          books;             // library of options (books)
    EdgeList        edges;             // boost.graph for cycle detection
    Vertices        vertices;          // boost.graph for cycle detection    
    int             next_edge;         				       

    Helper(OptionH c, BooksH b) :
        ident(),
        value(),
        current(c),
        books(b),
        edges(),
        vertices(),
        next_edge(0)
    {
    }
    
    int AssignVertexNo(std::string const & name)
    {
        Vertices::iterator it = vertices.find(name);
        if (it != vertices.end())
            return it->second;
        int num = next_edge++;
        vertices.insert(Vertices::value_type(name, num));
        return num;
    }
    
    void SetBook1(char const *b, char const *e) 
    {      
        if (b != e) 
            ident.assign(b, e);
    }
    void SetBook2(char const *b, char const *e)
    {
        if (b != e) 
            value.assign(b, e);
    }
    void InsertNewBook()
    {
        int edgvert = AssignVertexNo(ident);
        if (value.size() > 1 && value[0] =='[' && *(value.end() - 1) == ']')
        {
            value.assign(value.begin() + 1, value.end() - 1);
            int valvert = AssignVertexNo(value);
            edges.push_back(Edge(edgvert, valvert));
        }  
        
        OptionH newopt(new Option(ident, value, books));
        books->insert(Books::value_type(ident, newopt));
        current = newopt;
        ident = value = ""; 
    }
    void SetWord(char const *b, char const *e)
    {
        if (b != e) 
            ident.assign(b, e);
    }
    void SetDefinition(char const *b, char const *e)
    {
        if (b != e) 
            value.assign(b, e);
    }
    void InsertNewWord()
    {
	    if (value.size() > 1 && value[0] =='[' && *(value.end() - 1) == ']')
	    {
    	    value.assign(value.begin() + 1, value.end() - 1);
	        edges.push_back(Edge(AssignVertexNo(current->m_title), AssignVertexNo(value)));
	    }  
	    current->m_vocab.insert(Vocab::value_type(ident, value));
	    ident = value = "";
    }
};


OptionH
Option::Create(Option::Sources const & filenames)
{
    using namespace boost;
    using namespace boost::spirit;

    BooksH books(new Books);
    OptionH opt(new Option("root", "", books));
    Helper helper(opt, books);

    
    // set up the rules for boost.spirit
    rule<> identifier   = +(alnum_p | '_');
    
    rule<> evalue       =  +(alnum_p | punct_p | '_') >>  
	                       *anychar_p >>
	                       *(alnum_p | punct_p | '_');
    
    rule<> bracketed1   = ch_p('[') >> *blank_p >> 
                          identifier[bind(&Helper::SetBook1, ref(helper), _1, _2)] >>
                          *blank_p >> ch_p(']');

    rule<> bracketed2   = ch_p('[') >> *blank_p >> 
                          identifier[bind(&Helper::SetBook2, ref(helper), _1, _2)] >>
                          *blank_p >> ch_p(']');

    rule<> comment_line = (ch_p('#') | ';') >> *print_p;

    rule<> book_line    = (*blank_p >> bracketed1 >> *blank_p >> 
                           !( as_lower_d["inherits"] >> *blank_p >> bracketed2))
                          [bind(&Helper::InsertNewBook, ref(helper))];
    
    rule<> equals_line  = (*blank_p >> 
                            identifier[bind(&Helper::SetWord, ref(helper), _1, _2)] >>
                           *blank_p >> ch_p('=') >> *blank_p >>
                            evalue[bind(&Helper::SetDefinition, ref(helper), _1, _2)])
                          [bind(&Helper::InsertNewWord, ref(helper))];
        
    rule<> ini_parser    = book_line | equals_line | comment_line;

    
    // slurp all of the files in order
    for (Sources::const_iterator ci = filenames.begin(); ci != filenames.end(); ++ci)
    {
	    std::ifstream infile(ci->c_str());
	    int line_no = 0;
	    std::string line;
        while (std::getline(infile, line))
	    {  
            ++line_no;

            if (line.empty() || line.find_first_not_of(" \t\n") == std::string::npos)
                continue;
            
    	    parse_info<> parser = parse(line.c_str(), ini_parser);
	        if (!parser.hit)
	        {
		        std::cerr << "Option: Failed parse line " << line_no 
		                  << " of " << *ci << ": '" << line << "'\n";
	        }
	    }
    }

    bool some_fail = false;
    // now make sure we have all of the required books
    for (Helper::Vertices::iterator ci = helper.vertices.begin(); ci != helper.vertices.end(); ++ci)
    {
	    if (helper.books->find(ci->first) == helper.books->end())
	    {
    	    some_fail = true;
	        std::cerr << "Option [" << ci->first << "] has been referenced but not defined";
	    }
    }

    if (some_fail)
	    throw Error<FileE>("Some required Options have not been defined");
    

    // do we have any cycles?
    adjacency_list<> graph(helper.edges.begin(), helper.edges.end(), helper.edges.size());
    CycleDetector vis(some_fail);
    depth_first_search(graph, visitor(vis));
    if (some_fail)
	    throw Error<FileE>("Illegal circular reference in files");


    // now update each
    for (Books::iterator it = helper.books->begin(); it != helper.books->end(); ++it)
    {
	    std::stack<OptionH> ostack;
	    OptionH current = it->second;
	    std::string supername = current->m_super;
	    while (supername != "")
	    {
	        current->m_super = "";
            current = helper.books->find(supername)->second;
	        supername = current->m_super;
	        ostack.push(current);
	    }
	    current = it->second;
	    while (!ostack.empty())
	    {
    	    OptionH pop = ostack.top();
	        ostack.pop();
	        for (Vocab::iterator v = pop->m_vocab.begin(); v != pop->m_vocab.end(); ++v)
	        {
		        if (current->m_vocab.find(v->first) == current->m_vocab.end())
		            current->m_vocab.insert(*v);
	        }	    
	    }
    }   

    for (Books::iterator b = helper.books->begin(); b != helper.books->end(); ++b)
        opt->m_vocab.insert(b->first, b->first);
    
    return opt;
}


bool
Option::checkRequiredKeys(Option::Sources const & srcs, std::string const & prefix) const
{
    return true;
}


void
Option::dumpToStderr() const
{
    std::cerr << "Current node: " << m_title << "\n";
    for (Books::const_iterator b = m_books->begin(); b != m_books->end(); ++b)
    {
        std::cerr << "\n['" << b->second->m_title << "']\n";
        for (Vocab::const_iterator v = b->second->m_vocab.begin(); v != b->second->m_vocab.end(); ++v)
        {
            std::cerr << "'"<< v->first << "' = '" << v->second << "'\n";
        }
    }
    return;
}


bool
Option::hasSub(std::string const & name) const
{
    std::vector<std::string> tokens = TextUtils::Tokenise(name, "/", "\\", "");
    Option *self = const_cast<Option *>(this);

    for (std::vector<std::string>::const_iterator ci = tokens.begin(); ci != tokens.end(); ++ci)
    {
	    Vocab::const_iterator v = self->m_vocab.find(*ci);
	    if (v == self->m_vocab.end())
    	    return false;
    	Books::const_iterator b = self->m_books->find(v->second);
    	if (b == self->m_books->end())
	        return false;
	    self = b->second.get();
    }
    return true;
}


OptionH 
Option::getSub(std::string const & name) const
{
    std::vector<std::string> tokens = TextUtils::Tokenise(name, "/", "\\", "");
    OptionH self(const_cast<Option *>(this)->shared_from_this());

    for (std::vector<std::string>::const_iterator ci = tokens.begin(); ci != tokens.end(); ++ci)
    {
	    Vocab::const_iterator v = self->m_vocab.find(*ci);
	    if (v == self->m_vocab.end())
	    {
    	    Error<NotFoundE> err("getSub() Unable to locate sub-key part ");
	        err << *ci << " of " << name;
	        throw err;
	    }
	    Books::const_iterator b = self->m_books->find(v->second);
	    if (b == self->m_books->end())
	    {
    	    Error<NotFoundE> err("Requested unavailable sub-key part ");
	        err << b->first << "(" << *ci << ") of " << name;
	        throw err;
	    }
	    self = b->second;
    }
    return self;
}


std::string const &
Option::getLeaf(std::string const & name) const
{
    std::vector<std::string> tokens = TextUtils::Tokenise(name, "/", "\\", "");
    Option const *self = this;
    std::string leafname = tokens.back();
    tokens.pop_back();

    for (std::vector<std::string>::const_iterator ci = tokens.begin(); ci != tokens.end(); ++ci)
    {
	    Vocab::const_iterator v = self->m_vocab.find(*ci);
	    if (v == self->m_vocab.end())
	    {
    	    Error<NotFoundE> err("getLeaf() Unable to locate sub-key part ");
	        err << *ci << " of " << name;
	        throw err;
	    }
	    Books::const_iterator b = self->m_books->find(v->second);
	    if (b == self->m_books->end())
	    {
  	        Error<NotFoundE> err("Requested unavailable sub-key part ");
	        err << b->first << "(" << *ci << ") of " << name;
	        throw err;
	    }
	    self = b->second.get();
    }
    Vocab::const_iterator v = self->m_vocab.find(leafname);
    if (v == self->m_vocab.end())
    {
	    Error<NotFoundE> err("Unable to locate leaf key part ");
	    err << leafname << " of " << name;
	    throw err;
    }
    return v->second;
}


bool
Option::hasKey(std::string const & name) const
{
    std::vector<std::string> tokens = TextUtils::Tokenise(name, "/", "\\", "");
    Option const *self = this;
    std::string leafname = tokens.back();
    tokens.pop_back();

    for (std::vector<std::string>::const_iterator ci = tokens.begin(); ci != tokens.end(); ++ci)
    {
        Vocab::const_iterator v = self->m_vocab.find(*ci);
	    if (v == self->m_vocab.end())
	        return false;
	    Books::const_iterator b = self->m_books->find(v->second);
	    if (b == self->m_books->end())
    	    return false;
	    self = b->second.get();
    }
    Vocab::const_iterator v = self->m_vocab.find(leafname);

    return v != self->m_vocab.end();
}

