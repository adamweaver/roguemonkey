#ifndef H_STLLIKE_
#define H_STLLIKE_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2007 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <functional>
#include <string>
#include <utility>


// copy_if is from the BOOST Wiki -
// http://www.crystalclearsoftware.com/cgi-bin/boost_wiki/wiki.pl?STLAlgorithmExtensions/CopyIf

/**
 * copy_if - copies from input to destination if *input satisfies a predicate
 *
 * @param InputIterator
 *                  iterator type which satisfies input iterator requirements
 * @param OutputIterator
 *                  iterator type which satisfies output iterator requirements
 * @param Predicate predicate type (unary predicate)
 * @param first     beginning of input sequence
 * @param last      one-past-the-end of input sequence
 * @param dest      beginning of output sequence
 * @param p         predicate to be used
 *
 * @return one past the end of the copied elements
 */
template <typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator
copy_if(InputIterator first, InputIterator last, OutputIterator dest,
                       Predicate p)
{
    for (;first != last; ++first)
    {
        if (p(*first))
        {
            *dest = *first;
            ++dest;
        }
    }
    return dest;
}


// for_each_if is from the BOOST Wiki -
// http://www.crystalclearsoftware.com/cgi-bin/boost_wiki/wiki.pl?STLAlgorithmExtensions/ForEachIf

/**
 * for_each_if Performs f() on any element elem in [first, last) where pred(elem) is true
 *
 * @param InputIterator  Input Iterator type
 * @param Predicate      Predicate type to satisfy
 * @param UnaryFunction  Function type
 * @param first          beginning of input sequence
 * @param last           one-past-the-end of input sequence
 * @param pred           predicate to be satisfied
 * @param f              function to be performed where pred() is true
 * @return               copy of f
 */
template<typename InputIterator, typename Predicate, typename UnaryFunction>
UnaryFunction
for_each_if(InputIterator first, InputIterator last,
                           Predicate pred, UnaryFunction f)
{
    for(;first != last; ++first)
    {
        if (pred(*first))
        {
            f(*first);
        }
    }
    return f;
}

/**
 * true if the common subset of [b1..e1) equals [b2..e2). 
 * warning - empty ranges will compare true (as the subset "nil" is equal)
 *
 * @param InputIter1        Input Iterator type for first range
 * @param InputIter2        Input Iterator type for second range
 * @param b1                beginning of first range
 * @param e1                end of first range
 * @param b2                beginning of second range
 * @param e2                end of second range
 * @return                  true if common subset remains equal
 */
template<class InputIter1, class InputIter2>
bool
equal_so_far(InputIter1 b1, InputIter2 e1, InputIter2 b2, InputIter2 e2)
{
    while (b1 != e1 && b2 != e2)
    {
        if (*b1 != *b2)
            return false;
        ++b1;
        ++b2;
    }
    return true;
}


/**
 * generic functor which does binary-and of arguments
 *
 * @param T           type of arguments
 */
template<typename T>
struct binary_and : public std::binary_function<T, T, bool>
{
    bool operator()(T t, T u) const
    {
        return t & u;
    }
};


/**
 * Functor if pair.first member equals a value
 *
 * @param pair_type   type of pair to be used
 * @param T           pair_type::first_argument_type
 */
template<typename pair_type>
struct FirstEquals : public std::unary_function<pair_type const &, bool>
{
    typename pair_type::first_type type;

    FirstEquals(typename pair_type::first_type data) :
        type(data)
    {
    }

    bool operator() (pair_type const & rhs)
    {
        return rhs.first == type;
    }

};


template<typename pair_type>
struct SecondEquals : public std::unary_function<pair_type const &, bool>
{
    typename pair_type::second_type type;

    SecondEquals(typename pair_type::second_type data) :
        type(data)
    {
    }

    bool operator() (pair_type const & rhs)
    {
        return rhs.second == type;
    }

};


/**
 * Converts a string representation to it's enum
 *
 * @param E        type of enum
 * @param str      string to check
 * @param maxnums  number of enums in array
 * @param names    array of enum names
 * @param def      default value to return if string not found
 * @return         enum corresponding to name, or def if not found
 */
template<typename E>
E
StringToEnum(std::string const & str, E maxnums, char const *names[], E def = static_cast<E>(0))
{
    for (int i = 0; i < maxnums; ++i)
    {
        if (str.compare(names[i]) == 0)
        {
            def = static_cast<E>(i);
            break;
        }
    }
    return def;
}


/**
 * Sort ASCII characters [a-zA-Z] in order (lowercase before uppercase)
 *
 * @param l     lhs character to compare
 * @param r     rhs character to compare
 * @return      true if lhs comes before rhs (lowercase before uppercase)
 */
struct AlphaComparator : public std::binary_function<char, char, bool>
{
    bool operator() (char l, char r) const
    {
        return (l ^ 32) < (r ^ 32);
    }
};


template<typename PAIR>
struct AlphaPairComparator : public std::binary_function<PAIR, PAIR, bool>
{
    bool operator() (PAIR l, PAIR r) const
    {
        return (l.first ^ 32) < (r.first ^ 32);
    }
};


template<typename PAIR>
struct AlphaPairFinder : public std::binary_function<PAIR, PAIR, bool>
{
    bool operator() (PAIR l, PAIR r) const
    {
        return l.first < r.first;
    }
};


// using bare std::for_each is borked, due to the whole mem_fun and mem_fun_ref fiasco.
// boost::lambda is a very solid alternative
// my_for_each is from C/C++ Users Journal (September 2003), by Vladimir Bartov
template<class Iter, class Func>
inline Func
For_Each(Iter first, Iter last, Func func) 
{
    return std::for_each(first, last, func);
}

template<class Iter, class Ret, class Class>
inline std::mem_fun_t<Ret, Class>
For_Each(Iter first, Iter last, Ret (Class::*func)())
{
    return std::for_each(first, last, std::mem_fun_t<Ret, Class>(func));
}

template<class Iter, class Ret, class Class>
inline std::const_mem_fun_t<Ret, Class>
For_Each(Iter first, Iter last, Ret (Class::*func)() const)
{
    return std::for_each(first, last, std::const_mem_fun_t<Ret, Class>(func));
}


template<class Iter, class Obj, class Ret, class Class, class Type>
inline void
Apply(Iter first, Iter last, Obj & obj, Ret (Class::*func)(Type))
{
    for ( ; first != last; ++first)
    {
        ((obj).*(func))(*first);
    }
}





#endif

