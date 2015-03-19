#ifndef H_TEXTUTILS_
#define H_TEXTUTILS_ 1

// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <vector>
#include <string>

#include "handles.h"

/**
 * English text manipluation utilities
 */
namespace TextUtils
{
    /**
     * Test if character is a vowel [a|e|i|o|u]
     * @param v      character to be tested
     * @return true or false
     */
    bool IsVowel(char v);

    /**
     * splits string into tokens
     * @param str      string to be tokenised
     * @param delims   list of characters to use as delimiters
     * @param escapers list of characters to escape a-la '\' in shell
     * @param quote    quote characters like ""
     * @return         vector of tokens
     */
    std::vector<std::string> Tokenise(std::string const & str,  
                                      std::string const & delims,
                                      std::string const & escapers, 
                                      std::string const & quote);

    /**
     * returns the next token, updates position for next search.
     * @param str      string to tokenise
     * @param out      position to begin, updated with one past end
     * @param delims   list of characters to use as delimiters
     * @param escapers list of characters to escape a-la '\' in shell
     * @param quote    quote characters like ""
     * @return         next token
     */
    std::string GetToken(std::string const & str, std::string::const_iterator & out, 
                         std::string const & delims, std::string const & escapers, 
                         std::string const & quote);

    /**
     * splits a character string into delimited fields
     * @param str      input string
     * @param delims   list of characters to be used as delimiters
     * @param escapers list of characters used to escape a-la '\' in shell
     * @param quote    list of characters used to quote entire fields, such as "blah"
     * @param outp     Fwd iterator pointing to space to store the tokens. It is a user error to not
     *                 have enough space in the collection Fwd iterates through. If in doubt, use a
     *                 std::*inserter<> as the iterator
     */
    template<typename Fwd>
        void Tokenise(std::string const & str, std::string const & delims,
                      std::string const & escapers, std::string const & quote,
                      Fwd outp)
    {
        std::vector<std::string> tokens = Tokenise(str, delims, escapers, quote);
        std::copy(tokens.begin(), tokens.end(), outp);
    }

    /**
     * Trim delimiters from front and back of string
     * @param orig    string to be trimmed
     * @param rubbish delimiters to trim from @param orig
     * @return the modified string
     */
    std::string & TrimFrontAndBack(std::string & orig, std::string const & rubbish);

    /**
     * Converts an entire (ASCII only) string to lowercase.
     * If the string contains any characters above 127 (international characters of
     * UTF-8), result is extremely undefined. 
     * @param orig   string to be lowercased
     * @return the modified string
     */
    std::string & LowercaseString(std::string & orig);


    /**
     * Converts an entire (ASCII only) string to uppercase.
     * If the string contains any characters above 127 (international characters of
     * UTF-8), result is extremely undefined.
     * @param orig   string to be uppercased
     * @return the modified string
     */
    std::string & UppercaseString(std::string & orig);

    /**
     * Capitalise the first letter of the string
     * @param orig     string to capitalise first
     * @return         modified string
     */
    std::string Cap1st(std::string const & orig);
    
    /**
     * Case Insensitive Comparison (ASCII only)
     * @param  lhs      1st string to compare
     * @param  rhs      2nd string to compare
     * @return          bool
     */
    bool CaseInsensitiveCompare(std::string const & lhs, std::string const & rhs);

    /**
     * Pluralises a string. Words to be pluralised are prepended with '#'
     * sword => swords, ox => oxen, child => children
     * @param orig   string to be pluralised
     * @param number number of items (ie, 1 == pluralise, !1 == do not pluralise)
     * @return pluralised string
     */
    std::string PluraliseString(std::string const & orig, int number);

    /**
     * Pluralises a word according to English rules
     * @param orig   word to be pluralised
     * @return pluralised word
     */
    std::string PluraliseWord(std::string const & orig);

    /**
     * Indefinite Article - 'A' or 'An'
     * @param orig   word to check
     * @return "a" or "an"
     */
    std::string IndefiniteArticle(std::string const & orig);

    /**
     * Does the string represent true? (This is not a WTF - it could be
     * "True", "true", "Yes", "T", "t", "1", "Y", or any combination)
     * @param str    string to examine
     * @return       bool
     */
    bool IsTrue(std::string const & str);

    std::string FormatMessage(std::string const & message, CreatureH viewer, DescribableH d1, DescribableH d2 = DescribableH(), 
                              DescribableH d3 = DescribableH(), DescribableH d4 = DescribableH(),
                              DescribableH d5 = DescribableH(), DescribableH d6 = DescribableH());

    std::string FormatMessage(std::string const & message, CreatureH viewer, std::vector<DescribableH> const & desc);

    /**
     * Convert a number into roman numerals (I, II, III, IV, V, VI, VII, VIII, IX, X, ...)
     * @param num      number to convert
     * @return         string representation
     */
    std::string RomanNumerals(int num);
}








#endif

