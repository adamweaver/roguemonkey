// -*- Mode: C++ -*-
// RogueMonkey (c) 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include "dice.h"
#include "dmutils.h"


namespace
{
    int NumSurrounding(DMUtils::CharVec const & cv, int width, int height, int x, int y,
		       char terrain, bool filled_borders)
    {
	int num = 0;

	for (int yadj = -1; yadj < 2; ++yadj)
	    for (int xadj = -1; xadj < 2; ++xadj)
	    {
		if (x + xadj >= 0 && x + xadj < width - 1 &&  y + yadj >= 0 && y + yadj < height - 1)
		{
		    if (cv[(y + yadj) * width + x + xadj] == terrain)
			++num;
		}
		else if (filled_borders)
		    ++num;
	    }

	    return num;
    }

}



DMUtils::CharVec 
DMUtils::CellularAutomata(int width, int height, char terrain, int perc_fill,
                          int blank_if_less, int fill_if_more, int iterations,
                          bool borders_filled)
{
    CharVec cv(width * height, ' ');

    for (int i = 0; i < static_cast<int>(cv.size()); ++i)
    {
	if (Dice::Random0(100) + 1 < perc_fill)
	    cv[i] = terrain;
    }

    for ( ; iterations; --iterations)
    {
	CharVec tmp(width * height);
	for (int y = 0; y < height; ++y)
	    for (int x = 0; x < width; ++x)
	    {
		int num = NumSurrounding(cv, width, height, x, y, terrain, borders_filled);
		if (num < blank_if_less)
		    tmp[y * width + x] = ' ';
		else if (num > fill_if_more)
		    tmp[y * width + x] = terrain;
		else
		    tmp[y * width + x] = cv[y * width + x];
	    }
	    cv.swap(tmp);
    }
    
    return cv;
}   	

