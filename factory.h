#ifndef H_FACTORY_
#define H_FACTORY_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <typeinfo>
#include <utility>
#include <functional>

#include "dictionary.h"
#include "error.h"



/**
 * On Factory<> error, throw exception
 */
template<typename ReturnedType, typename IDKey>
struct ThrowP
{
    ReturnedType doNotFound(IDKey const & id) const
    {
        ReturnedType rt;
        Error<NotFoundE> err("Factory creation function for ");
        err << typeid(rt).name() << " not found. ID requested was " << id;
        throw err;
    }
};  


/**
 * On error, create a default object
 */
template<typename ReturnedType, typename IDKey>
struct CreateDefaultP
{
    ReturnedType doNotFound(IDKey const & /*id*/) const
    {
        return ReturnedType();
    }
};


/**
 * Object factory. Creation functions must be registered with
 * the factory, and then can be later called by index with
 * optionally up to 5 parameters
 */
template
<
    typename CreatedType,
    typename IDKey,
    typename CreatorFunc,
    typename ReturnedType = CreatedType,
    typename OnNotFoundP = ThrowP<ReturnedType, IDKey>,
    typename ComparatorFunc = std::less<IDKey>
>
class Factory : private OnNotFoundP
{
public:

    /**
     * Creates empty Factory<>
     */
    Factory()
    	: m_lookup()
    {
    }


    /**
     * Creates an empty Factory<> with an ordering functor
     *
     * @param c      Ordering Functor
     */
    explicit Factory(ComparatorFunc const & c)
    	: m_lookup(c)
    {
    }


    /**
     * Register a creator function
     *
     * @param key    lookup key
     * @param func   creation function
     */
    void registerCreator(IDKey const &key, CreatorFunc const &func)
    {
	    m_lookup.insert(key, func);
    }


    /**
     * Remove a creation function
     *
     * @param key    Key index of removed function
     */
    void deregisterCreator(IDKey const &key)
    {
        m_lookup.erase(key);
    }


    /**
     * Create an object from factory with no arguments
     *
     * @param key    key index
     *
     * @return Object
     */
    ReturnedType create(IDKey const &key)
    {
        typename Lookup::const_iterator ci = m_lookup.find(key);
        return (ci == m_lookup.end()) ? this->doNotFound(key) 
                                      : ReturnedType((ci->second)());
    }


    /**
     * Create an object from factory with 1 argument
     *
     * @param P1     Argument type
     * @param key    key index of desired factory function
     * @param p1     Argument 1
     *
     * @return Object
     */
    template<typename P1>
    ReturnedType create(IDKey const &key, P1 const &p1)
    {
    typename Lookup::const_iterator ci = m_lookup.find(key);
    return (ci == m_lookup.end()) ? this->doNotFound(key) 
                                  : ReturnedType((ci->second)(p1));
    }


    /**
     * Create an object from factory with 2 arguments
     *
     * @param P1     Argument 1 type
     * @param P2     Argument 2 type
     * @param key    Key index of deisred factory function
     * @param p1     Argument 1
     * @param p2     Argument 2
     *
     * @return Object
     */
    template<typename P1, typename P2>
    ReturnedType create(IDKey const &key, P1 const &p1, P2 const &p2)
    {
        typename Lookup::const_iterator ci = m_lookup.find(key);
        return (ci == m_lookup.end()) ? this->doNotFound(key) 
                                      : ReturnedType((ci->second)(p1, p2));
    }


    /**
     * Create an object from factory with 3 arguments
     *
     * @param P1     Argument 1 type
     * @param P2     Argument 2 type
     * @param P3     Argument 3 type
     * @param key    Key index of deisred factory function
     * @param p1     Argument 1
     * @param p2     Argument 2
     * @param p3     Argument 3
     *
     * @return Object
     */
    template<typename P1, typename P2, typename P3>
    ReturnedType create(IDKey const &key, P1 const &p1, P2 const &p2,
			P3 const &p3)
    {
        typename Lookup::const_iterator ci = m_lookup.find(key);
        return (ci == m_lookup.end()) ? this->doNotFound(key) :
	            ReturnedType((ci->second)(p1, p2, p3));
    }


    /**
     * Create an object from factory with 4 arguments
     *
     * @param P1     Argument 1 type
     * @param P2     Argument 2 type
     * @param P3     Argument 3 type
     * @param P4     Argument 4 type
     * @param key    Key index of deisred factory function
     * @param p1     Argument 1
     * @param p2     Argument 2
     * @param p3     Argument 3
     * @param p4     Argument 4
     *
     * @return Object
     */
    template<typename P1, typename P2, typename P3, typename P4>
    ReturnedType create(IDKey const &key, P1 const &p1, P2 const &p2,
			P3 const &p3, P4 const &p4)
    {
        typename Lookup::const_iterator ci = m_lookup.find(key);
        return (ci == m_lookup.end()) ? this->doNotFound(key) :
	        ReturnedType((ci->second)(p1, p2, p3, p4));
    }


    /**
     * Create an object from factory with 2 arguments
     *
     * @param P1     Argument 1 type
     * @param P2     Argument 2 type
     * @param P3     Argument 3 type
     * @param P4     Argument 4 type
     * @param P5     Argument 5 type
     * @param key    Key index of deisred factory function
     * @param p1     Argument 1
     * @param p2     Argument 2
     * @param p3     Argument 3
     * @param p4     Argument 4
     * @param p5     Argument 5
     *
     * @return Object
     */
    template<typename P1, typename P2, typename P3, typename P4, typename P5>
    ReturnedType create(IDKey const &key, P1 const &p1, P2 const &p2,
			P3 const &p3, P4 const &p4, P5 const &p5)
    {
        typename Lookup::const_iterator ci = m_lookup.find(key);
        return (ci == m_lookup.end()) ? this->doNotFound(key) :
	        ReturnedType((ci->second)(p1, p2, p3, p4, p5));
    }

private:
    typedef Dictionary<IDKey, CreatorFunc, ComparatorFunc> Lookup;
    Lookup m_lookup;
};




#endif

