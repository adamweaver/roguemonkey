#ifndef H_DICE_
#define H_DICE_ 1

// -*- Mode: C++ -*-
// RogueMonkey (c) 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt


#include <limits>

#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int.hpp"
#include "boost/random/variate_generator.hpp"
#include "boost/noncopyable.hpp"

/**
 * Random Number (PRNG) utilities
 */
class Dice : boost::noncopyable
{
public:
    /**
     * Constructs a specific PRNG to return numbers from a min to a max,
     * with an optional seed.
     *
     * @param mmin        minimum of PRNG range
     * @param mmax        maximum of PRNG range
     * @param seed        initial seed for PRNG 
     */
    Dice(int mmin, int mmax, int seed);

    /**
     * Constructs a specific PRNG to return numbers from a min to a max,
     * with an optional seed.
     *
     * @param mmin        minimum of PRNG range (defaults 0)
     * @param mmax        maximum of PRNG range (defaults to INT_MAX)
     */
    Dice(int mmin = 0, int mmax = std::numeric_limits<int>::max());

    /**
     * Returns a number from the global PRNG from 0 to 1 less than mmax.
     *
     * @param mmax        1 more than maximum return
     * @return            [0..mmax)
     */
    static int Random0(int mmax);

    /**
     * Returns a number from the global PRNG based on textual description.
     * ie "(1d6+3)/3=2:3" is roll 1d6, add 3, then divide result by 3,
     * with min of 2, and max of 3. Operator precedence is not respected,
     * brackets, parentheses, and braces must be used.
     *
     * @param desc        dice description
     * @return            result
     */
    static int Random(char const * desc);

    /**
     * Returns a number from the global PRNG based on textual description.
     * ie "(1d6+3)/3=2:3" is roll 1d6, add 3, then divide result by 3,
     * with min of 2, and max of 3. Operator precedence is not respected,
     * brackets, parentheses, and braces must be used.
     *
     * @param desc        dice description
     * @return            result
     */
    static int Random(std::string const & desc);

    /**
     * Returns a number from local PRNG as initially declared
     *
     * @return            result
     */
    int rnd();


private:
    boost::mt19937 m_rng;
    boost::uniform_int<> m_type;
    boost::variate_generator<boost::mt19937 &, boost::uniform_int<> > m_variate;
};




#endif

