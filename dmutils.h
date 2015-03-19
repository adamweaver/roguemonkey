#ifndef H_DMUTILS_
#define H_DMUTILS_ 1

// -*- Mode: C++ -*-
// RogueMonkey (c) 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <vector>



namespace DMUtils
{
    typedef std::vector<char> CharVec;

    /**
     * Create a Cellular Automata filled array
     *
     * @param width           width of array
     * @param height          height of array
     * @param terrain         terrain "character" to fill with
     * @param perc_fill       % chance that a given cell is filled
     * @param blank_if_less   cell cleared if number of filled neighbours less than this
     * @param fill_if_more    cell is filled if number of filled neighbours more than this
     * @param iterations      number of cycles to run through
     * @param borders_filled  are borders (<0 or >=width) treated as filled?
     * @return                Cellular Automata filled array
     */
    CharVec CellularAutomata(int width, int height, char terrain, int perc_fill,
                             int blank_if_less, int fill_if_more, int iterations,
                             bool borders_filled); 

}










#endif

