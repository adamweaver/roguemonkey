// -*- Mode: C++ -*-
// RogueMonkey (c) 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt


#ifndef H_MATRIX2D_
#define H_MATRIX2D_ 1

#include <deque>
#include <cassert>
#include <iterator>


template<typename T>
class Matrix2D
{
public:
    typedef std::deque<T> Line1D;
    typedef std::deque<Line1D> Line2D;
    typedef T                                       value_type;
    typedef value_type *                            pointer;
    typedef value_type const *                      const_pointer;
    typedef unsigned int                            size_type;
    typedef value_type &                            reference;
    typedef value_type const &                      const_reference;
    typedef ptrdiff_t                               difference_type;

    class const_iterator;

    class iterator : public std::iterator<std::random_access_iterator_tag, T>
    {
    public:
        iterator();
	iterator(typename Matrix2D<T>::Line2D * mat,
                 typename Matrix2D<T>::size_type x,
                 typename Matrix2D<T>::size_type y);

        T & operator*() const;
        T * operator->() const;

        iterator const & operator++();
        iterator operator++(int);
        iterator const & operator--();
        iterator operator--(int);
        iterator & operator+=(int plus);
        iterator operator+(int plus) const;
        iterator & operator-=(int minus);
        iterator operator-(int minus) const;
        typename Matrix2D<T>::difference_type operator-(iterator const & rhs) const;


        bool operator< (iterator const & rhs) const;
        bool operator> (iterator const & rhs) const;
        bool operator<= (iterator const & rhs) const;
        bool operator>= (iterator const & rhs) const;
        bool operator== (iterator const & rhs) const;
        bool operator!= (iterator const & rhs) const;

    private:
        friend class Matrix2D<T>::const_iterator;
        typename Matrix2D<T>::Line2D *m_matrix;
        typename Matrix2D<T>::size_type m_xpos;
        typename Matrix2D<T>::size_type m_ypos;
    };

    class const_iterator : public std::iterator<std::random_access_iterator_tag, T>
    {
    public:
        const_iterator();
        const_iterator(typename Matrix2D<T>::iterator const & rhs);
        const_iterator(typename Matrix2D<T>::Line2D *mat,
                       typename Matrix2D<T>::size_type x,
                       typename Matrix2D<T>::size_type y);

        T const & operator*() const;
        T const * operator->() const;

        const_iterator const & operator++();
        const_iterator operator++(int);
        const_iterator const & operator--();
        const_iterator operator--(int);
        const_iterator & operator+=(int plus);
        const_iterator & operator-=(int minus);
        const_iterator operator+(int plus) const;
        const_iterator operator-(int minus) const;
        typename Matrix2D<T>::difference_type operator-(const_iterator const & rhs) const;

        bool operator< (const_iterator const & rhs) const;
        bool operator> (const_iterator const & rhs) const;
        bool operator<= (const_iterator const & rhs) const;
        bool operator>= (const_iterator const & rhs) const;
        bool operator== (const_iterator const & rhs) const;
        bool operator!= (const_iterator const & rhs) const;

    private:
        typename Matrix2D<T>::Line2D *m_matrix;
        typename Matrix2D<T>::size_type m_xpos;
        typename Matrix2D<T>::size_type m_ypos;
    };


    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


    Matrix2D();
	Matrix2D(size_type cols, size_type rows, T const & tmp = T());
    template<typename It>
        Matrix2D(size_type cols, size_type rows, It first, It last);

    size_type size_x() const;
    size_type size_y() const;

    T & operator() (size_type x, size_type y);
    T const & operator() (size_type x, size_type y) const;
    T & at(size_type x, size_type y);
    T const & at(size_type x, size_type y) const;

    void push_front_y(T const & tmp = T());
    void push_front_x(T const & tmp = T());
    void push_back_y(T const & tmp = T());
    void push_back_x(T const & tmp = T());
    template<typename It> void push_front_y(It first, It last);
    template<typename It> void push_front_x(It first, It last);
    template<typename It> void push_back_y(It first, It last);
    template<typename It> void push_back_x(It first, It last);


    void pop_front_y();
    void pop_front_x();
    void pop_back_y();
    void pop_back_x();

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;

private:
    Line2D m_matrix;

};


template<typename T>
Matrix2D<T>::Matrix2D()
    : m_matrix()
{
}


template<typename T>
Matrix2D<T>::Matrix2D(size_type cols, size_type rows, T const & tmp)
    : m_matrix(rows, Line1D(cols, tmp))
{
}


template<typename T>
template<typename It>
Matrix2D<T>::Matrix2D(size_type cols, size_type rows, It first, It last)
    : m_matrix()
{
    assert(std::distance(first, last) >= rows * cols);
    for (size_type i = 0; i < rows; ++i)
    {
        m_matrix.push_back(Line1D(first, first + cols));
        std::advance(first, cols);
    }
}


template<typename T>
typename Matrix2D<T>::size_type
Matrix2D<T>::size_x() const
{
    return m_matrix.size() ? m_matrix.front().size() : 0;
}


template<typename T>
typename Matrix2D<T>::size_type
Matrix2D<T>::size_y() const
{
    return m_matrix.size();
}


template<typename T>
T &
Matrix2D<T>::operator() (typename Matrix2D<T>::size_type x,
                         typename Matrix2D<T>::size_type y)
{
    return m_matrix[y][x];
}


template<typename T>
T &
Matrix2D<T>::at(typename Matrix2D<T>::size_type x,
                typename Matrix2D<T>::size_type y)
{
    return m_matrix.at(y).at(x);
}


template<typename T>
T const &
Matrix2D<T>::operator() (typename Matrix2D<T>::size_type x,
                         typename Matrix2D<T>::size_type y) const
{
    return m_matrix[y][x];
}


template<typename T>
T const &
Matrix2D<T>::at(typename Matrix2D<T>::size_type x,
                typename Matrix2D<T>::size_type y) const
{
    return m_matrix.at(y).at(x);
}


template<typename T>
void
Matrix2D<T>::push_front_y(T const & tmp)
{
    m_matrix.push_front(Line1D(size_y(), tmp));
}


template<typename T>
void
Matrix2D<T>::push_back_y(T const & tmp)
{
    m_matrix.push_back(Line1D(size_y(), tmp));
}


template<typename T>
template<typename It>
void
Matrix2D<T>::push_front_y(It first, It last)
{
    assert(std::distance(first, last) >= size_y());
    m_matrix.push_front(Line1D(first, last));
}


template<typename T>
template<typename It>
void
Matrix2D<T>::push_back_y(It first, It last)
{
    assert(std::distance(first, last) >= size_y());
    m_matrix.push_back(Line1D(first, last));
}


template<typename T>
void
Matrix2D<T>::push_front_x(T const & tmp)
{
    for (typename Line2D::iterator i = m_matrix.begin(); i != m_matrix.end(); ++i)
        i->push_front(tmp);
}


template<typename T>
void
Matrix2D<T>::push_back_x(T const & tmp)
{
    for (typename Line2D::iterator i = m_matrix.begin(); i != m_matrix.end(); ++i)
        i->push_back(tmp);
}


template<typename T>
template<typename It>
void
Matrix2D<T>::push_front_x(It first, It last)
{
    assert(std::distance(first, last) >= size_x());
    for (typename Line2D::iterator i = m_matrix.begin(); i != m_matrix.end(); ++i)
        i->push_front(*first++);
}


template<typename T>
template<typename It>
void
Matrix2D<T>::push_back_x(It first, It last)
{
    assert(std::distance(first, last) >= size_x());
    for (typename Line2D::iterator i = m_matrix.begin(); i != m_matrix.end(); ++i)
        i->push_back(*first++);
}


template<typename T>
void
Matrix2D<T>::pop_front_y()
{
    m_matrix.pop_front();
}


template<typename T>
void
Matrix2D<T>::pop_back_y()
{
    m_matrix.pop_back();
}


template<typename T>
void
Matrix2D<T>::pop_front_x()
{
    for (typename Line2D::iterator i = m_matrix.begin(); i != m_matrix.end(); ++i)
        i->pop_front();
}


template<typename T>
void
Matrix2D<T>::pop_back_x()
{
    for (typename Line2D::iterator i = m_matrix.begin(); i != m_matrix.end(); ++i)
        i->pop_back();
}


template<typename T>
typename Matrix2D<T>::iterator
Matrix2D<T>::begin()
{
    return iterator(&m_matrix, 0, 0);
}


template<typename T>
typename Matrix2D<T>::const_iterator
Matrix2D<T>::begin() const
{
    return const_iterator(&m_matrix, 0, 0);
}


template<typename T>
typename Matrix2D<T>::iterator
Matrix2D<T>::end()
{
    return iterator(&m_matrix, size_x(), size_y());
}


template<typename T>
typename Matrix2D<T>::const_iterator
Matrix2D<T>::end() const
{
    return const_iterator(&m_matrix, size_x(), size_y());
}


template<typename T>
typename Matrix2D<T>::reverse_iterator
Matrix2D<T>::rbegin()
{
    return reverse_iterator(&m_matrix, size_x(), size_y());
}


template<typename T>
typename Matrix2D<T>::const_reverse_iterator
Matrix2D<T>::rbegin() const
{
    return const_reverse_iterator(&m_matrix, size_x(), size_y());
}


template<typename T>
typename Matrix2D<T>::reverse_iterator
Matrix2D<T>::rend()
{
    return reverse_iterator(&m_matrix, 0, 0);
}


template<typename T>
typename Matrix2D<T>::const_reverse_iterator
Matrix2D<T>::rend() const
{
    return const_reverse_iterator(&m_matrix, 0, 0);
}

//-------------------------------------------------------------------------//

template<typename T>
Matrix2D<T>::iterator::iterator()
    : m_matrix(0), m_xpos(0), m_ypos(0)
{
}


template<typename T>
Matrix2D<T>::iterator::iterator(typename Matrix2D<T>::Line2D * mat,
                                typename Matrix2D<T>::size_type x,
                                typename Matrix2D<T>::size_type y)
    : m_matrix(mat), m_xpos(x), m_ypos(y)
{
}


template<typename T>
T &
Matrix2D<T>::iterator::operator*() const
{
    return m_matrix->operator()(m_xpos, m_ypos);
}


template<typename T>
T *
Matrix2D<T>::iterator::operator->() const
{
    return & operator*();
}


template<typename T>
typename Matrix2D<T>::iterator const &
Matrix2D<T>::iterator::operator++()
{
    if (++m_xpos > m_matrix->size_x())
    {
        ++m_ypos;
        m_xpos = 0;
    }
    return *this;
}


template<typename T>
typename Matrix2D<T>::iterator
Matrix2D<T>::iterator::operator++(int)
{
    iterator tmp(*this);
    this->operator++();
    return tmp;
}


template<typename T>
typename Matrix2D<T>::iterator const &
Matrix2D<T>::iterator::operator--()
{
    if (m_xpos-- == 0)
    {
        m_xpos = m_matrix->size_x();
        --m_ypos;
    }
    return *this;
}


template<typename T>
typename Matrix2D<T>::iterator
Matrix2D<T>::iterator::operator--(int)
{
    iterator tmp(*this);
    this->operator--();
    return tmp;
}


template<typename T>
typename Matrix2D<T>::iterator &
Matrix2D<T>::iterator::operator+= (int plus)
{
    typename Matrix2D<T>::size_type xsize = m_matrix->size_x();
    typename Matrix2D<T>::size_type ysize = m_matrix->size_y();
    m_xpos += plus;
    while (m_xpos > xsize)
    {
        m_xpos -= xsize;
        ++m_ypos;
    }
    if (m_ypos > ysize)
    {
        m_xpos = xsize;
        m_ypos = ysize;
    }
    return *this;
}


template<typename T>
typename Matrix2D<T>::iterator
Matrix2D<T>::iterator::operator+ (int plus) const
{
    iterator tmp(*this);
    tmp += plus;
    return tmp;
}


template<typename T>
typename Matrix2D<T>::iterator &
Matrix2D<T>::iterator::operator-= (int minus)
{
    typename Matrix2D<T>::size_type xsize = m_matrix->size_x();
    signed int xpos = m_xpos;
    signed int ypos = m_ypos;
    xpos -= minus;
    while (xpos < 0)
    {
        xpos += xsize;
        --ypos;
    }
    if (ypos < 0)
    {
        xpos = 0;
        ypos = 0;
    }
    m_xpos = xpos;
    m_ypos = ypos;
    return *this;
}


template<typename T>
typename Matrix2D<T>::iterator
Matrix2D<T>::iterator::operator- (int minus) const
{
    iterator tmp(*this);
    tmp -= minus;
    return tmp;
}



template<typename T>
bool
Matrix2D<T>::iterator::operator< (typename Matrix2D<T>::iterator const & rhs) const
{
    return (m_ypos < rhs.m_ypos || m_ypos == rhs.m_ypos && m_xpos < rhs.m_xpos);
}


template<typename T>
bool
Matrix2D<T>::iterator::operator> (typename Matrix2D<T>::iterator const & rhs) const
{
    return (m_ypos > rhs.m_ypos || m_ypos == rhs.m_ypos && m_xpos > rhs.m_xpos);
}


template<typename T>
bool
Matrix2D<T>::iterator::operator<= (typename Matrix2D<T>::iterator const & rhs) const
{
    return !operator>(rhs);
}


template<typename T>
bool
Matrix2D<T>::iterator::operator>= (typename Matrix2D<T>::iterator const & rhs) const
{
    return !operator<(rhs);
}


template<typename T>
bool
Matrix2D<T>::iterator::operator== (typename Matrix2D<T>::iterator const & rhs) const
{
    return (m_xpos == rhs.m_xpos && m_ypos == rhs.m_ypos);
}


template<typename T>
bool
Matrix2D<T>::iterator::operator!= (typename Matrix2D<T>::iterator const & rhs) const
{
    return !operator!(rhs);
}


template<typename T>
typename Matrix2D<T>::difference_type
Matrix2D<T>::iterator::operator-(typename Matrix2D<T>::iterator const & rhs) const
{
    typename Matrix2D<T>::size_type xsize = m_matrix->size_x();
    typename Matrix2D<T>::difference_type delta = xsize - m_xpos + rhs.m_xpos;
    if (rhs > *this)
        delta += (rhs.m_ypos - m_ypos) * xsize;
    else
        delta = -delta - (m_ypos - rhs.m_xpos) * xsize;
    return delta;
}

//-------------------------------------------------------------------------//

template<typename T>
Matrix2D<T>::const_iterator::const_iterator()
    : m_matrix(0), m_xpos(0), m_ypos(0)
{
}


template<typename T>
Matrix2D<T>::const_iterator::const_iterator(typename Matrix2D<T>::Line2D * mat,
                                            typename Matrix2D<T>::size_type x,
                                            typename Matrix2D<T>::size_type y)
    : m_matrix(mat), m_xpos(x), m_ypos(y)
{
}


template<typename T>
Matrix2D<T>::const_iterator::const_iterator(typename Matrix2D<T>::iterator const & rhs)
    : m_matrix(rhs.m_matrix), m_xpos(rhs.m_xpos), m_ypos(rhs.m_ypos)
{
}


template<typename T>
T const &
Matrix2D<T>::const_iterator::operator*() const
{
    return const_cast<typename Matrix2D<T>::Line2D const>(*m_matrix).operator()(m_xpos, m_ypos);
}


template<typename T>
T const *
Matrix2D<T>::const_iterator::operator->() const
{
    return & operator*();
}


template<typename T>
typename Matrix2D<T>::const_iterator const &
Matrix2D<T>::const_iterator::operator++()
{
    if (++m_xpos > m_matrix->size_x())
    {
        ++m_ypos;
        m_xpos = 0;
    }
    return *this;
}


template<typename T>
typename Matrix2D<T>::const_iterator
Matrix2D<T>::const_iterator::operator++(int)
{
    const_iterator tmp(*this);
    this->operator++();
    return tmp;
}


template<typename T>
typename Matrix2D<T>::const_iterator const &
Matrix2D<T>::const_iterator::operator--()
{
    if (m_xpos-- == 0)
    {
        m_xpos = m_matrix->size_x();
        --m_ypos;
    }
    return *this;
}


template<typename T>
typename Matrix2D<T>::const_iterator
Matrix2D<T>::const_iterator::operator--(int)
{
    const_iterator tmp(*this);
    this->operator--();
    return tmp;
}


template<typename T>
typename Matrix2D<T>::const_iterator &
Matrix2D<T>::const_iterator::operator+= (int plus)
{
    typename Matrix2D<T>::size_type xsize = m_matrix->size_x();
    typename Matrix2D<T>::size_type ysize = m_matrix->size_y();
    m_xpos += plus;
    while (m_xpos > xsize)
    {
        m_xpos -= xsize;
        ++m_ypos;
    }
    if (m_ypos > ysize)
    {
        m_xpos = xsize;
        m_ypos = ysize;
    }
    return *this;
}


template<typename T>
typename Matrix2D<T>::const_iterator
Matrix2D<T>::const_iterator::operator+ (int plus) const
{
    const_iterator tmp(*this);
    tmp += plus;
    return tmp;
}


template<typename T>
typename Matrix2D<T>::const_iterator &
Matrix2D<T>::const_iterator::operator-= (int minus)
{
    typename Matrix2D<T>::size_type xsize = m_matrix->size_x();
    signed int xpos = m_xpos;
    signed int ypos = m_ypos;
    xpos -= minus;
    while (xpos < 0)
    {
        xpos += xsize;
        --ypos;
    }
    if (ypos < 0)
    {
        xpos = 0;
        ypos = 0;
    }
    m_xpos = xpos;
    m_ypos = ypos;
    return *this;
}


template<typename T>
typename Matrix2D<T>::const_iterator
Matrix2D<T>::const_iterator::operator- (int minus) const
{
    const_iterator tmp(*this);
    tmp -= minus;
    return tmp;
}



template<typename T>
bool
Matrix2D<T>::const_iterator::operator<
    (typename Matrix2D<T>::const_iterator const & rhs) const
{
    return (m_ypos < rhs.m_ypos || m_ypos == rhs.m_ypos && m_xpos < rhs.m_xpos);
}


template<typename T>
bool
Matrix2D<T>::const_iterator::operator>
    (typename Matrix2D<T>::const_iterator const & rhs) const
{
    return (m_ypos > rhs.m_ypos || m_ypos == rhs.m_ypos && m_xpos > rhs.m_xpos);
}


template<typename T>
bool
Matrix2D<T>::const_iterator::operator<=
    (typename Matrix2D<T>::const_iterator const & rhs) const
{
    return !operator>(rhs);
}


template<typename T>
bool
Matrix2D<T>::const_iterator::operator>=
    (typename Matrix2D<T>::const_iterator const & rhs) const
{
    return !operator<(rhs);
}


template<typename T>
bool
Matrix2D<T>::const_iterator::operator==
    (typename Matrix2D<T>::const_iterator const & rhs) const
{
    return (m_xpos == rhs.m_xpos && m_ypos == rhs.m_ypos);
}


template<typename T>
bool
Matrix2D<T>::const_iterator::operator!=
    (typename Matrix2D<T>::const_iterator const & rhs) const
{
    return !operator!(rhs);
}


template<typename T>
typename Matrix2D<T>::difference_type
Matrix2D<T>::const_iterator::operator-
    (typename Matrix2D<T>::const_iterator const & rhs) const
{
    typename Matrix2D<T>::size_type xsize = m_matrix->size_x();
    typename Matrix2D<T>::difference_type delta = xsize - m_xpos + rhs.m_xpos;
    if (rhs > *this)
        delta += (rhs.m_ypos - m_ypos) * xsize;
    else
        delta = -delta - (m_ypos - rhs.m_xpos) * xsize;
    return delta;
}


#endif
