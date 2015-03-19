#ifndef H_SKIPLIST_
#define H_SKIPLIST_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <utility>
#include <iterator>
#include <vector>

#include <cassert>

#include "dice.h"

template
<
    typename Key,
    typename Value,
    class Comparator = std::less<Key>,
    class Allocator = std::allocator<std::pair<Key, Value> >
>
class SkipList
{
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::pair<Key, Value> value_type;
    typedef Comparator Cmp;
    typedef Allocator allocator_type;

    typedef typename Allocator::size_type size_type;
    typedef typename Allocator::difference_type difference_type;
    typedef typename Allocator::reference reference;
    typedef typename Allocator::const_reference const_reference;

    //=======================================================================
    // Node
    //=======================================================================
    struct Node
    {
	Node(size_type sz, value_type const & vt)
	    : forward(sz), value(vt)
	{
	}

	std::vector<Node *> forward;
	value_type value;
    };

    class const_iterator;

    //=======================================================================
    // iterator
    //=======================================================================
    class iterator : public std::iterator<std::forward_iterator_tag, value_type>
    {
    public:
	iterator()
	    : m_node(0)
	{
	}

	value_type & operator*() const
	{
	    return m_node->value;
	}

	value_type * operator->() const
	{
	    return &m_node->value;
	}

	iterator & operator++()
	{
	    m_node = m_node->forward[0];
	    return *this;
	}

	iterator operator++(int)
	{
	    iterator tmp(*this);
	    m_node = m_node->forward[0];
	    return tmp;
	}

	bool operator==(iterator const & rhs)
	{
	    return m_node == rhs.m_node;
	}

	bool operator!=(iterator const & rhs)
	{
	    return m_node != rhs.m_node;
	}

    private:
	friend class const_iterator;
	friend class SkipList;
	iterator(Node *n)
	    : m_node(n)
	{
	}

	Node *m_node;
    };

    //=======================================================================
    // const_iterator
    //=======================================================================
    class const_iterator : public std::iterator<std::forward_iterator_tag, value_type const>
    {
    public:
	const_iterator()
	    : m_node(0)
	{
	}

	const_iterator(iterator const & it)
	    : m_node(it.m_node)
	{
	}

	value_type const & operator*() const
	{
	    return m_node->value;
	}

	value_type const * operator->() const
	{
	    return &m_node->value;
	}

	const_iterator & operator++()
	{
	    m_node = m_node->forward[0];
	    return *this;
	}

	const_iterator operator++(int)
	{
	    const_iterator tmp(*this);
	    m_node = m_node->forward[0];
	    return tmp;
	}

	bool operator==(const_iterator const & rhs)
	{
	    return m_node == rhs.m_node;
	}

	bool operator!=(const_iterator const & rhs)
	{
	    return m_node != rhs.m_node;
	}

    private:
	friend class SkipList;
	const_iterator(Node *n)
	    : m_node(n)
	{
	}

	Node *m_node;
    };

    //=======================================================================
    // SkipList
    //=======================================================================
    explicit SkipList(size_type maxsize = 5)
    	: m_maxsize(maxsize),
	  m_cursize(1),
	  m_header(maxsize, value_type()),
	  m_dice(1, maxsize)
    {
    }

    ~SkipList()
    {
	Node *ptr = m_header.forward[0];
	while (ptr != 0)
	{
	    Node *q = ptr->forward[0];
	    delete ptr;
	    ptr = q;
	}
    }

    iterator find(key_type const & k)
    {
	Node *x = &m_header;
	for (int i = m_cursize - 1; i >= 0;  --i)
	{
	    while (x->forward[i] != 0 && m_cmp(x->forward[i]->value.first, k))
	    {
		x = x->forward[i];
	    }
	}
	x = x->forward[0];
	return iterator((x != 0 && x->value.first == k) ? x : 0);
    }

    const_iterator find(key_type const & k) const
    {
	Node *x = &m_header;
	for (int i = m_maxsize - 1; i >= 0; --i)
	{
	    while (x->forward[i] != 0 && m_cmp(x->forward[i]->value.first, k))
	    {
		x = x->forward[i];
	    }
	}
	x = x->forward[0];
	return const_iterator((x != 0 && x->value.first == k) ? x : 0);
    }

    mapped_type & operator[](key_type const & k)
    {
	std::vector<Node *> update(m_maxsize);
	Node *x = &m_header;
	for (int i = m_cursize - 1; i >= 0; --i)
	{
	    while (x->forward[i] != 0 && m_cmp(x->forward[i]->value.first, k))
	    {
		x = x->forward[i];
	    }
	    update[i] = x;
	}
	x = x->forward[0];
	if (x == 0 || x->value.first != k)
	{
	    size_type level = m_dice();
	    if (level > m_cursize)
	    {
		update[m_cursize] = &m_header;
		level = ++m_cursize;
	    }
	    x = new Node(level, value_type(k, mapped_type()));
	    for (size_type i = 0; i < level; ++i)
	    {
		x->forward[i] = update[i]->forward[i];
		update[i]->forward[i] = x;
	    }
	}
	return x->value.second;
    }

    iterator insert(key_type const & k, mapped_type const & m)
    {
	std::vector<Node *> update(m_maxsize);
	Node *x = &m_header;
	for (int i = m_cursize - 1; i >= 0; --i)
	{
	    while (x->forward[i] != 0 && m_cmp(x->forward[i]->value.first, k))
	    {
		x = x->forward[i];
	    }
	    update[i] = x;
	}

	size_type level = m_dice();
	if (level > m_cursize)
	{
	    update[m_cursize] = &m_header;
	    level = ++m_cursize;
	}

	x = new Node(level, value_type(k, m));

	for (size_type i = 0; i < level; ++i)
	{
	    x->forward[i] = update[i]->forward[i];
	    update[i]->forward[i] = x;
	}
	return iterator(x);
    }


    iterator modifyKey(key_type const &k, key_type const & newkey)
    {
	Node *x = find(k).m_node;
	assert (x != 0);
	if (x->forward[0] == 0 || !m_cmp(x->forward[0]->value.first, newkey))
	{
	    x->value.first = newkey;
	    return iterator(x);
	}

	mapped_type value = x->value.second;
	erase(k);
	return insert(newkey, value);
    }

    void erase(key_type const & k)
    {
	std::vector<Node *> update(m_maxsize);
	Node *x = &m_header;
	for (int i = m_cursize - 1; i >= 0; --i)
	{
	    while (x->forward[i] != 0 && m_cmp(x->forward[i]->value.first, k))
	    {
		x = x->forward[i];
	    }
	    update[i] = x;
	}

	x = x->forward[0];
	if (x != 0 && x->value.first == k)
	{
	    for (size_type i = 0; i < m_cursize; ++i)
	    {
		if (update[i]->forward[i] != x)
		    break;
		update[i]->forward[i] = x->forward[i];
	    }
	    delete x;
	    while (m_cursize > 1 && m_header.forward[m_cursize - 1] == 0)
		--m_cursize;
	}
    }

    void erase(iterator it)
    {
	erase(it->first);
    }

    void clear()
    {
	Node *ptr = m_header.forward[0];
	while (ptr != 0)
	{
	    Node *q = ptr->forward[0];
	    delete ptr;
	    ptr = q;
	}
	m_cursize = 1;
    }

    size_type size() const
    {
	size_type n = 0;
	Node *x = m_header.forward[0];
	while (x != 0)
	{
	    ++n;
	    x = x->forward[0];
	}
	return n;
    }

    bool empty() const
    {
	return m_header.forward[0] == 0;
    }

    iterator begin()
    {
	return iterator(m_header.forward[0]);
    }

    const_iterator begin() const
    {
	return const_iterator(m_header.forward[0]);
    }

    iterator end()
    {
	return iterator(0);
    }

    const_iterator end() const
    {
	return const_iterator(0);
    }

private:
    friend class Node;

    size_type m_maxsize;
    size_type m_cursize;
    Node m_header;
    FixedDice m_dice;
    Comparator m_cmp;

};

#endif

