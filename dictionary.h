#ifndef H_DICTIONARY_
#define H_DICTIONARY_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <vector>
#include <utility>
#include <algorithm>
#include <functional>

/**
 * Dictionary<>: associative array higher with speed lookup than std::map<>.
 * @author Adam White
 * @version 1
 * @see std::map<>
 */
template
<
    typename Key,
    typename Value,
    class ComparatorFunc = std::less<Key>
>
class Dictionary
{
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::pair<Key, Value> value_type;
    typedef std::vector<value_type> Dict;
    typedef typename Dict::allocator_type allocator_type;
    typedef ComparatorFunc key_compare;


    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::difference_type difference_type;
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference const_reference;

    typedef typename Dict::iterator iterator;
    typedef typename Dict::const_iterator const_iterator;
    typedef typename Dict::reverse_iterator reverse_iterator;
    typedef typename Dict::const_reverse_iterator const_reverse_iterator;


    /**
     * Creates a Dictionary<> with an optional custom ordering func
     * @param comp   optional ordering functor
     */
    explicit Dictionary(ComparatorFunc const comp = ComparatorFunc())
    :   m_dict(),
        m_comparator(comp)
    {
    }


    /**
     * Creates a Dictionary<> from iterator range
     * @param In     Forward iterator class
     * @param first  Beginning of iterator range
     * @param last   One-past-end of iterator range
     * @param comp   Optional ordering functor
     */
    template<typename In>
        Dictionary(In first, In last, ComparatorFunc comp = ComparatorFunc())
    :   m_dict(first, last),
        m_comparator(comp)
    {
        std::sort(begin(), end(), m_comparator);
    }


    /**
     * Locate iterator to key
     * @param k      key to locate
     * @return iterator or end() if unsuccessful
     */
    iterator find(key_type const & k)
    {
        iterator i = std::lower_bound(begin(), end(), k, m_comparator);
        return i != end() && i->first == k ? i : end();
    }


    /**
     * Locate iterator by key
     * @param k      key to locate
     * @return iterator or end() const if unsuccessful
     */
    const_iterator find(key_type const & k) const
    {
        const_iterator i = std::lower_bound(begin(), end(), k, m_comparator);
        return i != end() && i->first == k ? i : end();
    }


    /**
     * Indexed access by key
     * @param k      key index
     * @return value or default-constructed Value() if non-existent
     */
    mapped_type & operator[] (key_type const & k)
    {
        iterator i = std::lower_bound(begin(), end(), k, m_comparator);
        if (i == end() || i->first != k)
        {
            i = m_dict.insert(i, value_type(k, mapped_type()));
        }
        return i->second;
    }



    /**
     * Update or insert value. Not a standard part of std::map
     * @param k      key index
     * @param m      value to insert/update
     * @return iterator to inserted item
     */
    iterator insert(key_type const & k, mapped_type const & m)
    {
        iterator i = std::lower_bound(begin(), end(), k, m_comparator);
        if (i == end() || i->first != k)
        {
            i = m_dict.insert(i, std::make_pair(k, m));
        }
        else
            i->second = m;

        return i;
    }


    /**
     * Update or insert value_type
     * @param val    value_type to insert
     * @return pair of iterator to item and bool (true if newly inserted, false if updated)
     */
    std::pair<iterator, bool> insert(value_type const & val)
    {
        iterator i = std::lower_bound(begin(), end(), val.first, m_comparator);
        bool found = true;
        if (i == end() || i->first != val.first)
        {
            i = m_dict.insert(i, val);
            found = false;
        }
        else
            i->second = val.second;

        return std::make_pair(i, !found);
    }


    /**
     * Insert a range denoted by iterators
     * @param In     Forward iterator type
     * @param first  beginning of range
     * @param last   one-past end of range
     */
    template<typename In>
    void insert(In first, In last)
    {
        for ( ; first != last; ++first)
        {
            insert(*first);
        }
    }


    /**
     * Delete an item by iterator
     * @param pos    Iterator to item to be deleted
     */
    void erase(iterator pos)
    {
        m_dict.erase(pos);
    }


    /**
     * Delete an item by key index
     * @param k      key index of item to delete
     * @return 0 if not deleted (didn't exist), 1 otherwise
     */
    size_type erase(key_type const & k)
    {
        iterator i = find(k);
        if (i == m_dict.end())
            return 0;
        m_dict.erase(i);
        return 1;
    }


    /**
     * Delete a range
     * @param first  Beginning of range
     * @param last   One-past-end of range
     */
    void erase(iterator first, iterator last)
    {
        m_dict.erase(first, last);
    }


    /**
     * Delete entire contents
     */
    void clear()
    {
        m_dict.clear();
    }


    /**
     * Count key-index items
     * @param k      key index to count
     * @return 1 if exists, 0 otherwise
     */
    size_type count(key_type const & k)
    {
        return find(k) != end();
    }

    /**
     * Return number of items in Dictionary
     * @return     number of key-value pairs
     */
    size_type size() const
    {
        return m_dict.size();
    }

    /**
     * Does the dictionary contain any elements?
     * @return       true if dictionary does not contain elements
     */
    bool empty() const 
    {
        return m_dict.empty();
    }

    /**
     * Find position in which item could be inserted safely
     * @param k      key item to check for insert position
     * @return iterator to appropriate position
     */
    iterator lower_bound(key_type const & k)
    {
        return std::lower_bound(begin(), end(), k, m_comparator);
    }


    /**
     * Find first element whose key is not less than key index
     * @param k      key item to check for insert position
     * @return iterator to appropriate position
     */
    const_iterator lower_bound(key_type const & k) const
    {
        return std::lower_bound(begin(), end(), k, m_comparator);
    }


    /**
     * Find first element whose key is greater than key index
     * @param k      key index to check
     * @return iterator to element
     */
    iterator upper_bound(key_type const & k)
    {
        return std::upper_bound(begin(), end(), k, m_comparator);
    }


    /**
     * Find first element whose key is greater than key index
     * @param k      key index to check
     * @return iterator to element
     */
    const_iterator upper_bound(key_type const & k) const
    {
        return std::upper_bound(begin(), end(), k, m_comparator);
    }


    /**
     * Finds a range of all elements whose key matches
     * @param k      key index to check
     * @return pair of iterator ranges
     */
    std::pair<iterator, iterator> equal_range(key_type const & k)
    {
        return std::equal_range(begin(), end(), k, m_comparator);
    }


    /**
     *  Finds a range of all elements whose key matches
     * @param k      key index to check
     * @return pair of iterator ranges
     */
    std::pair<const_iterator, const_iterator> equal_range(key_type const & k) const
    {
        return std::equal_range(begin(), end(), k, m_comparator);
    }


    /**
     * Beginning of range
     * @return first iterator
     */
    iterator begin()
    {
        return m_dict.begin();
    }

    /**
     * Beginning of range
     * @return first iterator
     */
    const_iterator begin() const
    {
        return m_dict.begin();
    }


    /**
     * Gets end of range
     * @return One-past-end of range
     */
    iterator end()
    {
        return m_dict.end();
    }


    /**
     * Gets end of range
     * @return One-past-end of range
     */
    const_iterator end() const
    {
        return m_dict.end();
    }


    /**
     * Gets reverse iterator to reversed beginning
     * @return First reversed iterator
     */
    reverse_iterator rbegin()
    {
        return m_dict.rbegin();
    }


    /**
     * Gets reverse iterator to reversed beginning
     * @return First reversed iterator
     */
    const_reverse_iterator rbegin() const
    {
        return m_dict.rbegin();
    }


    /**
     * Gets end iterator for reversed sequence
     * @return One-past-end reversed iterator
     */
    reverse_iterator rend()
    {
        return m_dict.rend();
    }


    /**
     * Gets end iterator for reversed sequence
     * @return One-past-end reversed iterator
     */
    const_reverse_iterator rend() const
    {
        return m_dict.rend();
    }


    class value_compare :
        public std::binary_function<value_type, value_type, bool>
    {
    private:
        friend class Dictionary;
        ComparatorFunc cmp;

        value_compare(ComparatorFunc c)
        : cmp(c)
        {
        }

    public:
        bool operator() (key_type const & x, key_type const & y) const
        {
            return cmp(x, y);
        }

        bool operator() (value_type const & x, key_type const & y) const
        {
            return cmp(x.first, y);
        }

        bool operator() (key_type const & x, value_type const & y) const
        {
            return cmp(x, y.first);
        }

        bool operator() (value_type const & x, value_type const & y) const
        {
            return cmp(x.first, y.first);
        }
    };


private:
    std::vector<value_type> m_dict;
    value_compare m_comparator;

};


/**
 * Compares two Dictionary<> for equality
 * @param K      Key type
 * @param V      Value type
 * @param C      Comparator functor
 * @param lhs    Dictionary<> one
 * @param rhs    Dictionary<> two
 *
 * @return true if equal
 */
template
<
    typename K,
    typename V,
    class C
>
bool
operator==(Dictionary<K,V,C> const &lhs, Dictionary<K,V,C> const &rhs)
{
    return lhs.m_dict == rhs.m_dict;
}


/**
 * Compares two Dictionary<> for less-than
 * @param K      Key type
 * @param V      Value type
 * @param C      Comparator functor
 * @param lhs    Dictionary<> one
 * @param rhs    Dictionary<> two
 *
 * @return true if lhs is less than rhs
 */
template
<
    typename K,
    typename V,
    class C
>
bool
operator<(Dictionary<K,V,C> const &lhs, Dictionary<K,V,C> const &rhs)
{
    return lhs.m_dict < rhs.m_dict;
}


#endif

